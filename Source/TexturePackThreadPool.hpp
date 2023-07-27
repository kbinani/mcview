#pragma once

namespace mcview {

class TexturePackThreadPool : public ThreadPool, public TexturePackJob::Delegate {
public:
  struct Delegate {
    virtual ~Delegate() = default;
    virtual void texturePackThreadPoolDidFinishJob(TexturePackThreadPool *pool, std::shared_ptr<TexturePackJob::Result> result) = 0;
  };

  explicit TexturePackThreadPool(Delegate *delegate) : ThreadPool((std::max)(1, (int)std::thread::hardware_concurrency() - 1)), fDelegate(delegate) {}
  virtual ~TexturePackThreadPool() {}

  virtual void addTexturePackJob(Region region, Dimension dim, bool useCache) {}

  void texturePackJobDidFinish(std::shared_ptr<TexturePackJob::Result> result) override {
    std::lock_guard<std::mutex> lock(fMut);
    if (fDelegate) {
      fDelegate->texturePackThreadPoolDidFinishJob(this, result);
    }
  }

  int compareJobs(ThreadPoolJob *a, ThreadPoolJob *b) override {
    TexturePackJob *jobA = dynamic_cast<TexturePackJob *>(a);
    TexturePackJob *jobB = dynamic_cast<TexturePackJob *>(b);
    if (jobA && !jobB) {
      return 1;
    } else if (!jobA && jobB) {
      return -1;
    } else if (!jobA || !jobB) {
      return 0;
    }
    LookAt la = fLookAt.load();
    juce::Point<float> center(la.fX, la.fZ);
    juce::Point<float> posA(jobA->fRegion.first * 512 + 256, jobA->fRegion.second * 512 + 256);
    juce::Point<float> posB(jobB->fRegion.first * 512 + 256, jobB->fRegion.second * 512 + 256);
    float distanceA = posA.getDistanceSquaredFrom(center);
    float distanceB = posB.getDistanceSquaredFrom(center);
    if (distanceA > distanceB) {
      return 1;
    } else if (distanceA < distanceB) {
      return -1;
    } else {
      return 0;
    }
  }

  void abandon(bool waitUntilTerminate) {
    {
      std::lock_guard<std::mutex> lock(fMut);
      fDelegate = nullptr;
    }
    removeAllJobs(true, waitUntilTerminate ? -1 : 0);
  }

public:
  std::atomic<LookAt> fLookAt;

private:
  std::mutex fMut;
  Delegate *fDelegate;
};

} // namespace mcview
