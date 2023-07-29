#pragma once

namespace mcview {

class FirewallEnv : public leveldb::Env {
public:
  explicit FirewallEnv(std::filesystem::path const &allowedDirectory) : fE(leveldb::Env::Default()) {
    namespace fs = std::filesystem;
    auto canonical = fs::canonical(allowedDirectory).native();
    if (!canonical.ends_with(fs::path::preferred_separator)) {
      canonical.push_back(fs::path::preferred_separator);
    }
    fAllowed = canonical;
  }

  leveldb::Status NewSequentialFile(std::filesystem::path const &fname, leveldb::SequentialFile **result) override {
    return fE->NewSequentialFile(fname, result);
  }

  leveldb::Status NewRandomAccessFile(std::filesystem::path const &fname, leveldb::RandomAccessFile **result) override {
    return fE->NewRandomAccessFile(fname, result);
  }

  leveldb::Status NewWritableFile(std::filesystem::path const &fname, leveldb::WritableFile **result) override {
    if (isAllowed(fname)) {
      return fE->NewWritableFile(fname, result);
    } else {
      return leveldb::Status::IOError({});
    }
  }

  bool FileExists(std::filesystem::path const &fname) override {
    return fE->FileExists(fname);
  }

  leveldb::Status GetChildren(std::filesystem::path const &dir, std::vector<std::filesystem::path> *result) override {
    return fE->GetChildren(dir, result);
  }

  leveldb::Status RemoveFile(std::filesystem::path const &fname) override {
    if (isAllowed(fname)) {
      return fE->RemoveFile(fname);
    } else {
      return leveldb::Status::IOError({});
    }
  }

  leveldb::Status CreateDir(std::filesystem::path const &dirname) override {
    if (isAllowed(dirname)) {
      return fE->CreateDir(dirname);
    } else {
      return leveldb::Status::IOError({});
    }
  }

  leveldb::Status RemoveDir(std::filesystem::path const &dirname) override {
    if (isAllowed(dirname)) {
      return fE->RemoveDir(dirname);
    } else {
      return leveldb::Status::IOError({});
    }
  }

  leveldb::Status GetFileSize(std::filesystem::path const &fname, uint64_t *file_size) override {
    return fE->GetFileSize(fname, file_size);
  }

  leveldb::Status RenameFile(std::filesystem::path const &src, std::filesystem::path const &target) override {
    if (isAllowed(src) && isAllowed(target)) {
      return fE->RenameFile(src, target);
    } else {
      return leveldb::Status::IOError({});
    }
  }

  leveldb::Status LockFile(std::filesystem::path const &fname, leveldb::FileLock **lock) override {
    if (isAllowed(fname)) {
      return fE->LockFile(fname, lock);
    } else {
      return leveldb::Status::IOError({});
    }
  }

  leveldb::Status UnlockFile(leveldb::FileLock *lock) override {
    return fE->UnlockFile(lock);
  }

  void Schedule(void (*function)(void *arg), void *arg) override {
    fE->Schedule(function, arg);
  }

  void StartThread(void (*function)(void *arg), void *arg) override {
    fE->StartThread(function, arg);
  }

  leveldb::Status GetTestDirectory(std::filesystem::path *path) override {
    return fE->GetTestDirectory(path);
  }

  leveldb::Status NewLogger(std::filesystem::path const &fname, leveldb::Logger **result) override {
    if (isAllowed(fname)) {
      return fE->NewLogger(fname, result);
    } else {
      return leveldb::Status::IOError({});
    }
  }

  uint64_t NowMicros() override {
    return fE->NowMicros();
  }

  void SleepForMicroseconds(int micros) override {
    fE->SleepForMicroseconds(micros);
  }

private:
  bool isAllowed(std::filesystem::path const &p) {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto path = fs::weakly_canonical(p, ec);
    if (ec) {
      return false;
    }
    return path.lexically_normal().native().starts_with(fAllowed);
  }

private:
  leveldb::Env *const fE;
  std::filesystem::path::string_type fAllowed;
};

} // namespace mcview
