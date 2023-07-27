#pragma once

namespace mcview {

class JavaTexturePackThreadPool : public TexturePackThreadPool {
public:
  JavaTexturePackThreadPool(juce::File directory, Dimension dim, Delegate *delegate) : TexturePackThreadPool(delegate), fWorldDirectory(directory), fDimension(dim) {
  }

  ~JavaTexturePackThreadPool() override {}

  void addTexturePackJob(Region region, bool useCache) override {
    juce::File dir = DimensionDirectory(fWorldDirectory, fDimension);
    juce::File mca = dir.getChildFile(mcfile::je::Region::GetDefaultRegionFileName(region.first, region.second));
    addJob(new JavaTexturePackJob(fWorldDirectory, mca, region, fDimension, useCache, this), true);
  }

private:
  juce::File const fWorldDirectory;
  Dimension const fDimension;
};

} // namespace mcview
