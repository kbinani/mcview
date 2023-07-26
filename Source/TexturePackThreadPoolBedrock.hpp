#pragma once

namespace mcview {

class TexturePackThreadPoolBedrock : public TexturePackThreadPool {
public:
  explicit TexturePackThreadPoolBedrock(juce::File dir) : fWorldDirectory(dir) {
    leveldb::DB *db = nullptr;
    if (auto st = leveldb::DB::Open({}, PathFromFile(dir) / "db", &db); st.ok()) {
      fDb.reset(db);
    }
  }

  ~TexturePackThreadPoolBedrock() override {}

  void addTexturePackJob(Region region, Dimension dim, bool useCache, TexturePackJob::Delegate *delegate) override {
    if (!fDb) {
      return;
    }
    addJob(new TexturePackJobBedrock(fDb.get(), fWorldDirectory, region, dim, useCache, delegate), true);
  }

private:
  juce::File const fWorldDirectory;
  std::unique_ptr<leveldb::DB> fDb;
};

} // namespace mcview
