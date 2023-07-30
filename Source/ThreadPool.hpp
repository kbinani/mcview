#pragma once

namespace mcview {

class ThreadPool;

class ThreadPoolJob {
public:
  //==============================================================================
  /** Creates a thread pool job object.
      After creating your job, add it to a thread pool with ThreadPool::addJob().
  */
  explicit ThreadPoolJob(const juce::String &name) : jobName(name) {
  }

  /** Destructor. */
  virtual ~ThreadPoolJob() {
    // you mustn't delete a job while it's still in a pool! Use ThreadPool::removeJob()
    // to remove it first!
    //jassert(pool == nullptr || !pool->contains(this));
  }

  //==============================================================================
  /** Returns the name of this job.
      @see setJobName
  */
  juce::String getJobName() const {
    return jobName;
  }

  /** Changes the job's name.
      @see getJobName
  */
  void setJobName(const juce::String &newName) {
    jobName = newName;
  }

  //==============================================================================
  /** These are the values that can be returned by the runJob() method.
   */
  enum JobStatus {
    jobHasFinished = 0, /**< indicates that the job has finished and can be
                             removed from the pool. */

    jobNeedsRunningAgain /**< indicates that the job would like to be called
                              again when a thread is free. */
  };

  /** Performs the actual work that this job needs to do.

      Your subclass must implement this method, in which is does its work.

      If the code in this method takes a significant time to run, it must repeatedly check
      the shouldExit() method to see if something is trying to interrupt the job.
      If shouldExit() ever returns true, the runJob() method must return immediately.

      If this method returns jobHasFinished, then the job will be removed from the pool
      immediately. If it returns jobNeedsRunningAgain, then the job will be left in the
      pool and will get a chance to run again as soon as a thread is free.

      @see shouldExit()
  */
  virtual JobStatus runJob() = 0;

  //==============================================================================
  /** Returns true if this job is currently running its runJob() method. */
  bool isRunning() const noexcept { return isActive; }

  /** Returns true if something is trying to interrupt this job and make it stop.

      Your runJob() method must call this whenever it gets a chance, and if it ever
      returns true, the runJob() method must return immediately.

      @see signalJobShouldExit()
  */
  bool shouldExit() const noexcept { return shouldStop; }

  /** Calling this will cause the shouldExit() method to return true, and the job
      should (if it's been implemented correctly) stop as soon as possible.

      @see shouldExit()
  */
  void signalJobShouldExit() {
    shouldStop = true;
    listeners.call([](juce::Thread::Listener &l) { l.exitSignalSent(); });
  }

  /** Add a listener to this thread job which will receive a callback when
      signalJobShouldExit was called on this thread job.

      @see signalJobShouldExit, removeListener
  */
  void addListener(juce::Thread::Listener *listener) {
    listeners.add(listener);
  }

  /** Removes a listener added with addListener. */
  void removeListener(juce::Thread::Listener *listener) {
    listeners.remove(listener);
  }

  //==============================================================================
private:
  friend class ThreadPool;
  juce::String jobName;
  ThreadPool *pool = nullptr;
  std::atomic<bool> shouldStop{false}, isActive{false}, shouldBeDeleted{false};
  juce::ListenerList<juce::Thread::Listener, juce::Array<juce::Thread::Listener *, juce::CriticalSection>> listeners;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreadPoolJob)
};

class ThreadPool {
  struct ThreadPoolThread : public juce::Thread {
    ThreadPoolThread(ThreadPool &p, size_t stackSize)
        : juce::Thread("Pool", stackSize), pool(p) {
    }

    void run() override {
      while (!threadShouldExit()) {
        if (!pool.runNextJob(*this))
          wait(500);
      }
    }

    std::atomic<ThreadPoolJob *> currentJob{nullptr};

    ThreadPool &pool;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreadPoolThread)
  };

public:
  ThreadPool(int numThreads, size_t threadStackSize = 0, juce::Thread::Priority priority = juce::Thread::Priority::normal) {
    jassert(numThreads > 0); // not much point having a pool without any threads!

    for (int i = juce::jmax(1, numThreads); --i >= 0;)
      threads.add(new ThreadPoolThread(*this, threadStackSize));

    for (auto *t : threads)
      t->startThread(priority);
  }

  /** Creates a thread pool with one thread per CPU core.
      Once you've created a pool, you can give it some jobs by calling addJob().
      If you want to specify the number of threads, use the other constructor; this
      one creates a pool which has one thread for each CPU core.
      @see SystemStats::getNumCpus()
  */
  ThreadPool() : ThreadPool(juce::SystemStats::getNumCpus(), 0, juce::Thread::Priority::normal) {
  }

  /** Destructor.

      This will attempt to remove all the jobs before deleting, but if you want to
      specify a timeout, you should call removeAllJobs() explicitly before deleting
      the pool.
  */
  ~ThreadPool() {
    removeAllJobs(true, 5000);
    stopThreads();
  }

  //==============================================================================
  /** A callback class used when you need to select which ThreadPoolJob objects are suitable
      for some kind of operation.
      @see ThreadPool::removeAllJobs
  */
  class JobSelector {
  public:
    virtual ~JobSelector() = default;

    /** Should return true if the specified thread matches your criteria for whatever
        operation that this object is being used for.

        Any implementation of this method must be extremely fast and thread-safe!
    */
    virtual bool isJobSuitable(ThreadPoolJob *job) = 0;
  };

  //==============================================================================
  /** Adds a job to the queue.

      Once a job has been added, then the next time a thread is free, it will run
      the job's ThreadPoolJob::runJob() method. Depending on the return value of the
      runJob() method, the pool will either remove the job from the pool or add it to
      the back of the queue to be run again.

      If deleteJobWhenFinished is true, then the job object will be owned and deleted by
      the pool when not needed - if you do this, make sure that your object's destructor
      is thread-safe.

      If deleteJobWhenFinished is false, the pointer will be used but not deleted, and
      the caller is responsible for making sure the object is not deleted before it has
      been removed from the pool.
  */
  void addJob(ThreadPoolJob *job, bool deleteJobWhenFinished) {
    jassert(job != nullptr);
    jassert(job->pool == nullptr);

    if (job->pool == nullptr) {
      job->pool = this;
      job->shouldStop = false;
      job->isActive = false;
      job->shouldBeDeleted = deleteJobWhenFinished;

      {
        const juce::ScopedLock sl(lock);
        jobs.add(job);
      }

      for (auto *t : threads)
        t->notify();
    }
  }

  /** Adds a lambda function to be called as a job.
      This will create an internal ThreadPoolJob object to encapsulate and call the lambda.
  */
  void addJob(std::function<ThreadPoolJob::JobStatus()> jobToRun) {
    struct LambdaJobWrapper : public ThreadPoolJob {
      LambdaJobWrapper(std::function<ThreadPoolJob::JobStatus()> j) : ThreadPoolJob("lambda"), job(j) {}
      JobStatus runJob() override { return job(); }

      std::function<ThreadPoolJob::JobStatus()> job;
    };

    addJob(new LambdaJobWrapper(jobToRun), true);
  }

  /** Adds a lambda function to be called as a job.
      This will create an internal ThreadPoolJob object to encapsulate and call the lambda.
  */
  void addJob(std::function<void()> jobToRun) {
    struct LambdaJobWrapper : public ThreadPoolJob {
      LambdaJobWrapper(std::function<void()> j) : ThreadPoolJob("lambda"), job(j) {}
      JobStatus runJob() override {
        job();
        return ThreadPoolJob::jobHasFinished;
      }

      std::function<void()> job;
    };

    addJob(new LambdaJobWrapper(jobToRun), true);
  }

  /** Tries to remove a job from the pool.

      If the job isn't yet running, this will simply remove it. If it is running, it
      will wait for it to finish.

      If the timeout period expires before the job finishes running, then the job will be
      left in the pool and this will return false. It returns true if the job is successfully
      stopped and removed.

      @param job                  the job to remove
      @param interruptIfRunning   if true, then if the job is currently busy, its
                                  ThreadPoolJob::signalJobShouldExit() method will be called to try
                                  to interrupt it. If false, then if the job will be allowed to run
                                  until it stops normally (or the timeout expires)
      @param timeOutMilliseconds  the length of time this method should wait for the job to finish
                                  before giving up and returning false
  */
  bool removeJob(ThreadPoolJob *job, bool interruptIfRunning, int timeOutMs) {
    bool dontWait = true;
    juce::OwnedArray<ThreadPoolJob> deletionList;

    if (job != nullptr) {
      const juce::ScopedLock sl(lock);

      if (jobs.contains(job)) {
        if (job->isActive) {
          if (interruptIfRunning)
            job->signalJobShouldExit();

          dontWait = false;
        } else {
          jobs.removeFirstMatchingValue(job);
          addToDeleteList(deletionList, job);
        }
      }
    }

    return dontWait || waitForJobToFinish(job, timeOutMs);
  }

  /** Tries to remove all jobs from the pool.

      @param interruptRunningJobs if true, then all running jobs will have their ThreadPoolJob::signalJobShouldExit()
                                  methods called to try to interrupt them
      @param timeOutMilliseconds  the length of time this method should wait for all the jobs to finish
                                  before giving up and returning false
      @param selectedJobsToRemove if this is not a nullptr, the JobSelector object is asked to decide
                                  which jobs should be removed. If it is a nullptr, all jobs are removed
      @returns    true if all jobs are successfully stopped and removed; false if the timeout period
                  expires while waiting for one or more jobs to stop
  */
  bool removeAllJobs(bool interruptRunningJobs, int timeOutMs,
                     JobSelector *selectedJobsToRemove = nullptr) {
    juce::Array<ThreadPoolJob *> jobsToWaitFor;

    {
      juce::OwnedArray<ThreadPoolJob> deletionList;

      {
        const juce::ScopedLock sl(lock);

        for (int i = jobs.size(); --i >= 0;) {
          auto *job = jobs.getUnchecked(i);

          if (selectedJobsToRemove == nullptr || selectedJobsToRemove->isJobSuitable(job)) {
            if (job->isActive) {
              jobsToWaitFor.add(job);

              if (interruptRunningJobs)
                job->signalJobShouldExit();
            } else {
              jobs.remove(i);
              addToDeleteList(deletionList, job);
            }
          }
        }
      }
    }

    auto start = juce::Time::getMillisecondCounter();

    for (;;) {
      for (int i = jobsToWaitFor.size(); --i >= 0;) {
        auto *job = jobsToWaitFor.getUnchecked(i);

        if (!isJobRunning(job))
          jobsToWaitFor.remove(i);
      }

      if (jobsToWaitFor.size() == 0)
        break;

      if (timeOutMs >= 0 && juce::Time::getMillisecondCounter() >= start + (juce::uint32)timeOutMs)
        return false;

      jobFinishedSignal.wait(20);
    }

    return true;
  }

  /** Returns the number of jobs currently running or queued. */
  int getNumJobs() const noexcept {
    const juce::ScopedLock sl(lock);
    return jobs.size();
  }

  /** Returns the number of threads assigned to this thread pool. */
  int getNumThreads() const noexcept {
    return threads.size();
  }

  /** Returns one of the jobs in the queue.

      Note that this can be a very volatile list as jobs might be continuously getting shifted
      around in the list, and this method may return nullptr if the index is currently out-of-range.
  */
  ThreadPoolJob *getJob(int index) const noexcept {
    const juce::ScopedLock sl(lock);
    return jobs[index];
  }

  /** Returns true if the given job is currently queued or running.

      @see isJobRunning()
  */
  bool contains(const ThreadPoolJob *job) const noexcept {
    const juce::ScopedLock sl(lock);
    return jobs.contains(const_cast<ThreadPoolJob *>(job));
  }

  /** Returns true if the given job is currently being run by a thread. */
  bool isJobRunning(const ThreadPoolJob *job) const noexcept {
    const juce::ScopedLock sl(lock);
    return jobs.contains(const_cast<ThreadPoolJob *>(job)) && job->isActive;
  }

  /** Waits until a job has finished running and has been removed from the pool.

      This will wait until the job is no longer in the pool - i.e. until its
      runJob() method returns ThreadPoolJob::jobHasFinished.

      If the timeout period expires before the job finishes, this will return false;
      it returns true if the job has finished successfully.
  */
  bool waitForJobToFinish(const ThreadPoolJob *job, int timeOutMs) const {
    if (job != nullptr) {
      auto start = juce::Time::getMillisecondCounter();

      while (contains(job)) {
        if (timeOutMs >= 0 && juce::Time::getMillisecondCounter() >= start + (juce::uint32)timeOutMs)
          return false;

        jobFinishedSignal.wait(2);
      }
    }

    return true;
  }

  /** If the given job is in the queue, this will move it to the front so that it
      is the next one to be executed.
  */
  void moveJobToFront(const ThreadPoolJob *job) noexcept {
    const juce::ScopedLock sl(lock);

    auto index = jobs.indexOf(const_cast<ThreadPoolJob *>(job));

    if (index > 0 && !job->isActive)
      jobs.move(index, 0);
  }

  /** Returns a list of the names of all the jobs currently running or queued.
      If onlyReturnActiveJobs is true, only the ones currently running are returned.
  */
  juce::StringArray getNamesOfAllJobs(bool onlyReturnActiveJobs) const {
    juce::StringArray s;
    const juce::ScopedLock sl(lock);

    for (auto *job : jobs)
      if (job->isActive || !onlyReturnActiveJobs)
        s.add(job->getJobName());

    return s;
  }

  virtual int compareJobs(ThreadPoolJob *a, ThreadPoolJob *b) {
    return 0;
  }

private:
  //==============================================================================
  juce::Array<ThreadPoolJob *> jobs;

  friend class ThreadPoolJob;
  juce::OwnedArray<ThreadPoolThread> threads;

  juce::CriticalSection lock;
  juce::WaitableEvent jobFinishedSignal;

  bool runNextJob(ThreadPoolThread &thread) {
    if (auto *job = pickNextJobToRun()) {
      auto result = ThreadPoolJob::jobHasFinished;
      thread.currentJob = job;

      try {
        result = job->runJob();
      } catch (...) {
        jassertfalse; // Your runJob() method mustn't throw any exceptions!
      }

      thread.currentJob = nullptr;

      juce::OwnedArray<ThreadPoolJob> deletionList;

      {
        const juce::ScopedLock sl(lock);

        if (jobs.contains(job)) {
          job->isActive = false;

          if (result != ThreadPoolJob::jobNeedsRunningAgain || job->shouldStop) {
            jobs.removeFirstMatchingValue(job);
            addToDeleteList(deletionList, job);

            jobFinishedSignal.signal();
          } else {
            // move the job to the end of the queue if it wants another go
            jobs.move(jobs.indexOf(job), -1);
          }
        }
      }

      return true;
    }

    return false;
  }
  ThreadPoolJob *pickNextJobToRun() {
    juce::OwnedArray<ThreadPoolJob> deletionList;

    {
      const juce::ScopedLock sl(lock);

      struct Comparator {
        explicit Comparator(ThreadPool *self) : fSelf(self) {}

        ThreadPool *const fSelf;
        int compareElements(ThreadPoolJob *a, ThreadPoolJob *b) {
          return fSelf->compareJobs(a, b);
        }
      } comparator(this);
      jobs.sort(comparator, true);

      for (int i = 0; i < jobs.size(); ++i) {
        if (auto *job = jobs[i]) {
          if (!job->isActive) {
            if (job->shouldStop) {
              jobs.remove(i);
              addToDeleteList(deletionList, job);
              --i;
              continue;
            }

            job->isActive = true;
            return job;
          }
        }
      }
    }

    return nullptr;
  }
  void addToDeleteList(juce::OwnedArray<ThreadPoolJob> &deletionList, ThreadPoolJob *job) const {
    job->shouldStop = true;
    job->pool = nullptr;

    if (job->shouldBeDeleted)
      deletionList.add(job);
  }
  void stopThreads() {
    for (auto *t : threads)
      t->signalThreadShouldExit();

    for (auto *t : threads)
      t->stopThread(500);
  }

  // Note that this method has changed, and no longer has a parameter to indicate
  // whether the jobs should be deleted - see the new method for details.
  void removeAllJobs(bool, int, bool);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreadPool)
};

} // namespace mcview
