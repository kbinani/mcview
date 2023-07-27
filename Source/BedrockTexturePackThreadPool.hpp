#pragma once

namespace mcview {

class BedrockTexturePackThreadPool : public TexturePackThreadPool {
public:
  BedrockTexturePackThreadPool(juce::File dir, Dimension dim, std::shared_ptr<leveldb::DB> db, Delegate *delegate) : TexturePackThreadPool(delegate), fDb(db), fWorldDirectory(dir), fDimension(dim) {
  }

  ~BedrockTexturePackThreadPool() override {}

  void addTexturePackJob(Region region, bool useCache) override {
    if (!fDb) {
      return;
    }
    addJob(new BedrockTexturePackJob(fDb.get(), fWorldDirectory, region, fDimension, useCache, this), true);
  }

public:
  std::shared_ptr<leveldb::DB> fDb;
  juce::File const fWorldDirectory;
  Dimension const fDimension;
};

} // namespace mcview
