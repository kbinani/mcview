#pragma once

namespace mcview {

class ProxyEnv : public leveldb::Env {
  struct File {
    std::optional<std::filesystem::path> fActual;
  };

  using Str = std::filesystem::path::string_type;

public:
  ProxyEnv(leveldb::Env *underlyingEnv, std::filesystem::path const &protectDir, std::filesystem::path const &workingDir)
      : fProtect(std::filesystem::canonical(protectDir)),
        fCanonicalProtect(std::filesystem::canonical(protectDir).native()),
        fWork(std::filesystem::canonical(workingDir)),
        fNextFileId(1) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::recursive_directory_iterator iterator(fProtect, ec);
    if (ec) {
      return;
    }
    fE = underlyingEnv;
    for (auto item : iterator) {
      auto path = item.path();
      if (fs::is_regular_file(path)) {
        if (auto key = FileKey(path); key) {
          fFiles[*key] = {};
        }
      }
    }
  }

  ~ProxyEnv() {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::remove_all(fWork, ec);
  }

  leveldb::Status NewSequentialFile(std::filesystem::path const &fname, leveldb::SequentialFile **result) override {
    if (!fE) {
      return IOError();
    }
    if (auto path = prepareForRead(fname); path) {
      return fE->NewSequentialFile(*path, result);
    } else {
      return IOError();
    }
  }

  leveldb::Status NewRandomAccessFile(std::filesystem::path const &fname, leveldb::RandomAccessFile **result) override {
    if (!fE) {
      return IOError();
    }
    if (auto path = prepareForRead(fname); path) {
      return fE->NewRandomAccessFile(*path, result);
    } else {
      return IOError();
    }
  }

  leveldb::Status NewWritableFile(std::filesystem::path const &fname, leveldb::WritableFile **result) override {
    if (!fE) {
      return IOError();
    }
    if (auto path = prepareForWrite(fname); path) {
      return fE->NewWritableFile(*path, result);
    } else {
      return IOError();
    }
  }

  bool FileExists(std::filesystem::path const &fname) override {
    if (!fE) {
      return false;
    }
    if (auto protect = shouldProtect(fname); protect) {
      if (*protect) {
        if (auto key = FileKey(fname); key) {
          std::lock_guard<std::mutex> lock(fMut);
          return fFiles.count(*key) > 0;
        } else {
          return false;
        }
      } else {
        return fE->FileExists(fname);
      }
    } else {
      return false;
    }
  }

  leveldb::Status GetChildren(std::filesystem::path const &dir, std::vector<std::filesystem::path> *result) override {
    namespace fs = std::filesystem;
    if (!fE) {
      return IOError();
    }
    auto protect = shouldProtect(dir);
    if (!protect) {
      return IOError();
    }
    if (!*protect) {
      return fE->GetChildren(dir, result);
    }
    auto key = FileKey(dir);
    if (!key) {
      return IOError();
    }
    Str prefix = *key;
    if (!prefix.ends_with(fs::path::preferred_separator)) {
      prefix.push_back(fs::path::preferred_separator);
    }
    std::lock_guard<std::mutex> lock(fMut);
    result->clear();
    for (auto const &it : fFiles) {
      if (!it.first.starts_with(prefix)) {
        continue;
      }
      Str trailing = it.first.substr(prefix.size());
      if (trailing.find(fs::path::preferred_separator) == Str::npos) {
        result->push_back(fs::path(trailing));
      }
    }
    return leveldb::Status::OK();
  }

  leveldb::Status RemoveFile(std::filesystem::path const &fname) override {
    if (!fE) {
      return IOError();
    }
    auto protect = shouldProtect(fname);
    if (!protect) {
      return IOError();
    }
    if (!*protect) {
      return fE->RemoveFile(fname);
    }
    auto key = FileKey(fname);
    if (!key) {
      return IOError();
    }
    std::lock_guard<std::mutex> lock(fMut);
    auto found = fFiles.find(*key);
    if (found == fFiles.end()) {
      return leveldb::Status::NotFound({});
    }
    if (found->second.fActual) {
      if (auto st = fE->RemoveFile(*found->second.fActual); !st.ok()) {
        return st;
      }
    }
    fFiles.erase(found);
    return leveldb::Status::OK();
  }

  leveldb::Status CreateDir(std::filesystem::path const &dirname) override {
    if (!fE) {
      return IOError();
    }
    auto protect = shouldProtect(dirname);
    if (!protect) {
      return IOError();
    }
    if (!*protect) {
      return fE->CreateDir(dirname);
    }
    return leveldb::Status::OK();
  }

  leveldb::Status RemoveDir(std::filesystem::path const &dirname) override {
    namespace fs = std::filesystem;
    if (!fE) {
      return IOError();
    }
    auto protect = shouldProtect(dirname);
    if (!protect) {
      return IOError();
    }
    if (!*protect) {
      return fE->RemoveDir(dirname);
    }
    auto key = FileKey(dirname);
    if (!key) {
      return IOError();
    }
    Str prefix = *key;
    if (!prefix.ends_with(fs::path::preferred_separator)) {
      prefix.push_back(fs::path::preferred_separator);
    }
    std::lock_guard<std::mutex> lock(fMut);
    for (auto const &it : fFiles) {
      if (it.first.starts_with(prefix)) {
        return IOError();
      }
    }
    return leveldb::Status::OK();
  }

  leveldb::Status GetFileSize(std::filesystem::path const &fname, uint64_t *file_size) override {
    if (!fE) {
      return IOError();
    }
    if (auto p = prepareForRead(fname); p) {
      return fE->GetFileSize(*p, file_size);
    } else {
      return IOError();
    }
  }

  leveldb::Status RenameFile(std::filesystem::path const &src, std::filesystem::path const &target) override {
    if (!fE) {
      return IOError();
    }
    auto protectSrc = shouldProtect(src);
    if (!protectSrc) {
      return IOError();
    }
    auto protectDest = shouldProtect(target);
    if (!protectDest) {
      return IOError();
    }
    if (!*protectSrc && !*protectDest) {
      return fE->RenameFile(src, target);
    }
    auto keySrc = FileKey(src);
    if (!keySrc) {
      return IOError();
    }
    auto keyDest = FileKey(target);
    if (!keyDest) {
      return IOError();
    }
    auto actualSrc = prepareForRead(src);
    if (!actualSrc) {
      return IOError();
    }
    auto actualDest = prepareForWrite(target);
    if (!actualDest) {
      return IOError();
    }
    mcfile::ScopedFile fileSrc(mcfile::File::Open(*actualSrc, mcfile::File::Mode::Read));
    if (!fileSrc) {
      return IOError();
    }
    mcfile::ScopedFile fileDest(mcfile::File::Open(*actualDest, mcfile::File::Mode::Write));
    if (!fileDest) {
      return IOError();
    }
    if (!FileCopy(fileSrc.get(), fileDest.get())) {
      fileDest.close();
      RemoveSilent(*actualDest);
      return IOError();
    }
    fileSrc.close();
    if (*protectSrc) {
      std::lock_guard<std::mutex> lock(fMut);
      fFiles.erase(*keySrc);
    } else {
      if (auto st = fE->RemoveFile(src); !st.ok()) {
        return st;
      }
    }
    return leveldb::Status::OK();
  }

  leveldb::Status LockFile(std::filesystem::path const &fname, leveldb::FileLock **lock) override {
    if (!fE) {
      return IOError();
    }
    auto actual = prepareForWrite(fname);
    if (!actual) {
      return IOError();
    }
    return fE->LockFile(*actual, lock);
  }

  leveldb::Status UnlockFile(leveldb::FileLock *lock) override {
    if (!fE) {
      return IOError();
    }
    return fE->UnlockFile(lock);
  }

  void Schedule(void (*function)(void *arg), void *arg) override {
    if (!fE) {
      return;
    }
    fE->Schedule(function, arg);
  }

  void StartThread(void (*function)(void *arg), void *arg) override {
    if (!fE) {
      return;
    }
    fE->StartThread(function, arg);
  }

  leveldb::Status GetTestDirectory(std::filesystem::path *path) override {
    return IOError();
  }

  leveldb::Status NewLogger(std::filesystem::path const &fname, leveldb::Logger **result) override {
    if (!fE) {
      return IOError();
    }
    auto actual = prepareForWrite(fname);
    if (!actual) {
      return IOError();
    }
    return fE->NewLogger(*actual, result);
  }

  uint64_t NowMicros() override {
    if (!fE) {
      return 0;
    }
    return fE->NowMicros();
  }

  void SleepForMicroseconds(int micros) override {
    if (!fE) {
      return;
    }
    fE->SleepForMicroseconds(micros);
  }

private:
  static leveldb::Status IOError() {
    return leveldb::Status::IOError({});
  }

  static std::optional<std::filesystem::path> Canonical(std::filesystem::path const &p) {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto ret = fs::weakly_canonical(p, ec);
    if (ec) {
      return std::nullopt;
    }
    return ret;
  }

  static std::optional<Str> FileKey(std::filesystem::path const &p) {
    auto path = p;
    path.make_preferred();
    if (auto canonical = Canonical(path); canonical) {
      return canonical->native();
    } else {
      return std::nullopt;
    }
  }

  std::optional<std::filesystem::path> prepareForRead(std::filesystem::path const &p) {
    std::lock_guard<std::mutex> lock(fMut);
    return unsafePrepareForRead(p);
  }

  std::optional<std::filesystem::path> unsafePrepareForRead(std::filesystem::path const &p) {
    auto key = FileKey(p);
    if (!key) {
      return std::nullopt;
    }
    auto protect = shouldProtect(p);
    if (!protect) {
      return std::nullopt;
    }
    if (*protect) {
      if (auto found = fFiles.find(*key); found != fFiles.end()) {
        if (found->second.fActual) {
          return found->second.fActual;
        } else {
          return p;
        }
      } else {
        return std::nullopt;
      }
    } else {
      return p;
    }
  }

  std::optional<std::filesystem::path> prepareForWrite(std::filesystem::path const &p) {
    std::lock_guard<std::mutex> lock(fMut);
    return unsafePrepareForWrite(p);
  }

  std::optional<std::filesystem::path> unsafePrepareForWrite(std::filesystem::path const &p) {
    namespace fs = std::filesystem;
    auto key = FileKey(p);
    if (!key) {
      return std::nullopt;
    }
    auto protect = shouldProtect(p);
    if (!protect) {
      return std::nullopt;
    }
    if (!*protect) {
      return p;
    }
    auto found = fFiles.find(*key);
    if (found == fFiles.end()) {
      auto path = nextProxyPath(p);
      File f;
      f.fActual = path;
      fFiles[*key] = f;
      return path;
    }
    File f = found->second;
    if (f.fActual) {
      return found->second.fActual;
    } else {
      auto path = nextProxyPath(p);
      mcfile::ScopedFile src(mcfile::File::Open(p, mcfile::File::Mode::Read));
      if (!src) {
        return std::nullopt;
      }
      mcfile::ScopedFile dest(mcfile::File::Open(path, mcfile::File::Mode::Write));
      if (!dest) {
        return std::nullopt;
      }
      std::vector<uint8_t> buffer(4096);
      while (!feof(src.get())) {
        auto read = fread(buffer.data(), 1, buffer.size(), src.get());
        if (fwrite(buffer.data(), 1, read, dest.get()) != read) {
          dest.close();
          RemoveSilent(path);
          return std::nullopt;
        }
      }
      f.fActual = path;
      fFiles[*key] = f;
      return path;
    }
  }

  std::optional<bool> shouldProtect(std::filesystem::path const &p) const {
    auto key = FileKey(p);
    if (!key) {
      return std::nullopt;
    }
    return key->starts_with(fCanonicalProtect);
  }

  static void RemoveSilent(std::filesystem::path const &p) {
    std::error_code ec;
    std::filesystem::remove(p, ec);
  }

  static bool FileCopy(FILE *src, FILE *dest) {
    std::vector<uint8_t> buffer(4096);
    while (!feof(src)) {
      auto read = fread(buffer.data(), 1, buffer.size(), src);
      if (fwrite(buffer.data(), 1, read, dest) != read) {
        return false;
      }
    }
    return true;
  }

  std::filesystem::path nextProxyPath(std::filesystem::path const &p) {
    namespace fs = std::filesystem;
    auto id = fNextFileId.fetch_add(1);
    return fWork / (fs::path(std::to_string(id) + "-").native() + p.filename().native());
  }

private:
  std::map<Str, File> fFiles;
  leveldb::Env *fE = nullptr;
  std::filesystem::path const fProtect;
  Str const fCanonicalProtect;
  std::filesystem::path const fWork;
  std::atomic_uint64_t fNextFileId;
  std::mutex fMut;
};

} // namespace mcview
