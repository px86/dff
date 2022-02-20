#include <filesystem>
#include <iostream>

#include "include/dff.hpp"
#include "include/sha256.hpp"

using pr::dff;

dff::dff(const char *rootdir, bool follow_symlinks)
    : m_rootdir(rootdir), m_follow_symlinks(follow_symlinks) {}

bool dff::find_dups() noexcept
{
  try {
    auto rd_itr =
        (m_follow_symlinks)
            ? fs::recursive_directory_iterator(
                  m_rootdir, fs::directory_options::follow_directory_symlink)
            : fs::recursive_directory_iterator(m_rootdir);

    std::cout << "\x1b[32m"
              << "scanning directory: " << fs::canonical(m_rootdir)
              << "\x1b[m\n\n";

    for (const auto &dirent: rd_itr) {
      if (!dirent.is_regular_file() || dirent.file_size() == 0)
        continue;

      auto opt_hash = sha256_hash_file(dirent.path().c_str());
      if (!opt_hash.has_value())
        continue;

      auto [itr, inserted] = m_store.insert({opt_hash.value(), {dirent.path()}});
      if (!inserted)
        itr->second.push_back(dirent.path());
    }

  } catch (const fs::filesystem_error &err) {
    std::cerr << "\x1b[31m" << err.what() << "\x1b[m\n";
    return false;
  } catch (...) {
    std::cerr << "\x1b[31m" << "Error: some error occured" << "\x1b[m\n";
    return false;
  }
  // true if no error occured.
  return true;
}

void dff::print_dups() noexcept
{
  for (const auto &[_, paths] : m_store) {
    if (paths.size() > 1) {
      std::cout << "\n\x1b[33m";
      for (const auto &path : paths)
        std::cout << path.c_str() << '\n';
      std::cout << "\x1b[m\n";
    }
  }
}
