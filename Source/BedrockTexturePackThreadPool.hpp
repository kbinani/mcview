#pragma once

namespace mcview {

class BedrockTexturePackThreadPool : public TexturePackThreadPool {
public:
  BedrockTexturePackThreadPool(juce::File dir, Delegate *delegate) : TexturePackThreadPool(delegate), fWorldDirectory(dir) {
    leveldb::DB *db = nullptr;
    if (auto st = leveldb::DB::Open({}, PathFromFile(dir) / "db", &db); st.ok()) {
      fDb.reset(db);
    }
  }

  ~BedrockTexturePackThreadPool() override {}

  void addTexturePackJob(Region region, Dimension dim, bool useCache) override {
    if (!fDb) {
      return;
    }
    addJob(new BedrockTexturePackJob(fDb.get(), fWorldDirectory, region, dim, useCache, this), true);
  }

private:
  juce::File const fWorldDirectory;
  std::unique_ptr<leveldb::DB> fDb;
};

} // namespace mcview
