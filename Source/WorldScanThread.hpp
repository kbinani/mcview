#pragma once

namespace mcview {

class WorldScanThread : public juce::Thread {
public:
  explicit WorldScanThread(juce::String name) : juce::Thread(name) {}
  virtual ~WorldScanThread() override {}
  virtual void abandon() = 0;
};

} // namespace mcview
