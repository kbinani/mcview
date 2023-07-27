#pragma once

namespace mcview {

class JavaTexturePackThreadPool : public TexturePackThreadPool {
public:
  JavaTexturePackThreadPool(juce::File directory, Delegate *delegate) : TexturePackThreadPool(delegate), fWorldDirectory(directory) {
  }

  ~JavaTexturePackThreadPool() override {}

  void addTexturePackJob(Region region, Dimension dim, bool useCache) override {
    juce::File mca;
    switch (dim) {
    case Dimension::TheNether:
      mca = fWorldDirectory.getChildFile("DIM-1").getChildFile("region").getChildFile(mcfile::je::Region::GetDefaultRegionFileName(region.first, region.second));
      break;
    case Dimension::TheEnd:
      mca = fWorldDirectory.getChildFile("DIM1").getChildFile("region").getChildFile(mcfile::je::Region::GetDefaultRegionFileName(region.first, region.second));
      break;
    case Dimension::Overworld:
    default:
      mca = fWorldDirectory.getChildFile("region").getChildFile(mcfile::je::Region::GetDefaultRegionFileName(region.first, region.second));
      break;
    }
    addJob(new JavaTexturePackJob(fWorldDirectory, mca, region, dim, useCache, this), true);
  }

private:
  juce::File const fWorldDirectory;
};

} // namespace mcview
