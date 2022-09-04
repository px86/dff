#pragma once

#include <filesystem>
#include <forward_list>
#include <string>
#include <unordered_map>

namespace pr {

class Record;
class dff;

namespace fs = std::filesystem;
using Store = std::unordered_map<std::string, Record>;

class Record {
public:
  Record() = default;
  Record(Record &&rec);
  auto size() const -> std::size_t;
  void insert(fs::path path);
  auto begin() const -> std::forward_list<fs::path>::const_iterator;
  auto end() const -> std::forward_list<fs::path>::const_iterator;

private:
  std::size_t m_size = 0;
  std::forward_list<fs::path> m_files;
};

class dff {
public:
  dff() = delete;
  dff(const char *rootdir, bool follow_symlinks);
  void print_dups() noexcept;
  bool find_dups() noexcept;

private:
  const char *m_rootdir;
  bool m_follow_symlinks;
  Store m_store;
};

} // namespace pr
