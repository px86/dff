#include <filesystem>
#include <iostream>

#include "include/dff.hpp"
#include "include/sha256.hpp"

using pr::dff;
using pr::Record;

Record::Record(Record &&rec)
    : m_size(rec.m_size), m_files(std::move(rec.m_files))
{
  rec.m_size = 0;
  rec.m_files.clear();
}

auto Record::size() const -> std::size_t { return m_size; }

void Record::insert(fs::path path)
{
  m_files.push_front(path);
  ++m_size;
}

auto Record::begin() const -> std::forward_list<fs::path>::const_iterator
{
  return m_files.cbegin();
}

auto Record::end() const -> std::forward_list<fs::path>::const_iterator
{
  return m_files.cend();
}

dff::dff(const char *rootdir, bool follow_symlinks)
    : m_rootdir(rootdir), m_follow_symlinks(follow_symlinks) {}

auto dff::find_dups() noexcept -> bool
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

    for (const auto &dirent : rd_itr) {
      if (!dirent.is_regular_file() || dirent.file_size() == 0)
        continue;

      auto opt_hash = sha256_hash_file(dirent.path().c_str());
      if (!opt_hash.has_value())
        continue;

      auto itr = m_store.find(opt_hash.value());

      if (itr != m_store.end()) {
        itr->second.insert(dirent.path());
      } else {
        Record rec;
        rec.insert(dirent.path());
        m_store.insert({opt_hash.value(), std::move(rec)});
      }
    }

  } catch (const fs::filesystem_error &err) {
    std::cerr << "\x1b[31m" << err.what() << "\x1b[m\n";
    return false;
  } catch (...) {
    std::cerr << "\x1b[31m"
              << "Error: some error occured"
              << "\x1b[m\n";
    return false;
  }
  // true if no error occured.
  return true;
}

void dff::print_dups() noexcept
{
  for (const auto &[_, rec] : m_store) {
    if (rec.size() > 1) {
      std::cout << "\n\x1b[33m";
      for (const auto& path: rec) std::cout << path << '\n';
      std::cout << "\x1b[m\n";
    }
  }
}
