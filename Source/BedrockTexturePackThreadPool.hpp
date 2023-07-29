#pragma once

namespace mcview {

class BedrockTexturePackThreadPool : public TexturePackThreadPool {
public:
  BedrockTexturePackThreadPool(juce::File dir, Dimension dim, std::shared_ptr<leveldb::DB> db, std::shared_ptr<ProxyEnv> env, Delegate *delegate) : TexturePackThreadPool(delegate), fDb(db), fEnv(env), fWorldDirectory(dir), fDimension(dim) {
  }

  ~BedrockTexturePackThreadPool() override {
    fDb.reset();
    fEnv.reset();
  }

  void addTexturePackJob(Region region, bool useCache) override {
    if (!fDb || !fEnv) {
      return;
    }
    addJob(new BedrockTexturePackJob(fDb.get(), fWorldDirectory, region, fDimension, useCache, this), true);
  }

public:
  std::shared_ptr<leveldb::DB> fDb;
  std::shared_ptr<ProxyEnv> fEnv;
  juce::File const fWorldDirectory;
  Dimension const fDimension;
};

} // namespace mcview
