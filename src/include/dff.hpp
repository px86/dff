#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace pr {

namespace fs = std::filesystem;
using Store = std::unordered_map<std::string, std::vector<fs::path>>;

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
