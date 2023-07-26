#pragma once

namespace mcview {

class TexturePackThreadPool : public juce::ThreadPool {
public:
  TexturePackThreadPool() : juce::ThreadPool((std::max)(1, (int)std::thread::hardware_concurrency() - 1)) {}
  virtual ~TexturePackThreadPool() {}

  virtual void addTexturePackJob(Region region, Dimension dim, bool useCache, TexturePackJob::Delegate *delegate) {}
};

} // namespace mcview
