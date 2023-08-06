#pragma once

namespace mcview {

class BedrockTexturePackThreadPool : public TexturePackThreadPool {
public:
  BedrockTexturePackThreadPool(juce::File dir,
                               Dimension dim,
                               std::optional<int64_t> lastPlayed,
                               std::shared_ptr<leveldb::DB> db,
                               std::shared_ptr<je2be::ReadonlyDb::Closer> dbAttachment,
                               Delegate *delegate)
      : TexturePackThreadPool(delegate),
        fDb(db),
        fDbAttachment(dbAttachment),
        fWorldDirectory(dir),
        fDimension(dim),
        fLastPlayed(lastPlayed) {
  }

  ~BedrockTexturePackThreadPool() override {
    fDb.reset();
    fDbAttachment.reset();
  }

  void addTexturePackJob(Region region, bool useCache) override {
    if (!fDb || !fDbAttachment) {
      return;
    }
    addJob(new BedrockTexturePackJob(fDb.get(), fWorldDirectory, region, fDimension, fLastPlayed, useCache, this), true);
  }

public:
  std::shared_ptr<leveldb::DB> fDb;
  std::shared_ptr<je2be::ReadonlyDb::Closer> fDbAttachment;
  juce::File const fWorldDirectory;
  Dimension const fDimension;
  std::optional<int64_t> const fLastPlayed;
};

} // namespace mcview
