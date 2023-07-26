#pragma once

namespace mcview {

class TexturePackThreadPool : public ThreadPool {
public:
  TexturePackThreadPool() : ThreadPool((std::max)(1, (int)std::thread::hardware_concurrency() - 1)) {}
  virtual ~TexturePackThreadPool() {}

  virtual void addTexturePackJob(Region region, Dimension dim, bool useCache, TexturePackJob::Delegate *delegate) {}

  int compareJobs(ThreadPoolJob *a, ThreadPoolJob *b) override {
    TexturePackJob *jobA = dynamic_cast<TexturePackJob *>(a);
    TexturePackJob *jobB = dynamic_cast<TexturePackJob *>(b);
    if (!jobA || !jobB) {
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
    } else if (distanceB > distanceA) {
      return -1;
    } else {
      return 0;
    }
  }

  std::atomic<LookAt> fLookAt;
};

} // namespace mcview
