#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

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

auto Record::insert(const fs::path &path) -> void
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
    : m_rootdir(rootdir), m_follow_symlinks(follow_symlinks)
{
  auto max_threads = std::thread::hardware_concurrency();
  if (max_threads == 0)
    max_threads = 2u;

  for (auto i = 0u; i < max_threads; ++i) {
    m_threads.push_back(std::thread(&dff::pick_and_execute, this));
  }
}

void dff::pick_and_execute()
{
  while (true) {
    std::unique_lock<std::mutex> lk(m_mutex);
    while (!m_finished && m_tasks.empty())
      m_cv.wait(lk);

    if (m_tasks.empty() && m_finished) break; // Thread will exit
    auto path = m_tasks.front();
    m_tasks.pop();

    lk.unlock();

      auto opt_hash = sha256_hash_file(path.c_str());
      if (!opt_hash.has_value()) continue;

    lk.lock();

    auto itr = m_store.find(opt_hash.value());

    if (itr != m_store.end()) {
      itr->second.insert(path);
    } else {
      Record rec;
      rec.insert(path);
      m_store.insert({opt_hash.value(), std::move(rec)});
    }
  }
}

auto dff::find_dups() noexcept -> void
{
  try {
    auto rd_itr =
        (m_follow_symlinks)
            ? fs::recursive_directory_iterator(
                  m_rootdir, fs::directory_options::follow_directory_symlink)
            : fs::recursive_directory_iterator(m_rootdir);

    std::cout << "scanning directory: "
	      << GREEN << fs::canonical(m_rootdir)
	      << RESET << "\n\n";

    for (const auto &dirent : rd_itr) {
      if (!dirent.is_regular_file() || dirent.file_size() == 0)
        continue;

      std::unique_lock<std::mutex> lk(m_mutex);
      m_tasks.push(dirent.path());
      lk.unlock();
      m_cv.notify_one();
    }

  } catch (const fs::filesystem_error &err) {
    std::cerr << RED << err.what() << RESET << '\n';
  } catch (...) {
    std::cerr << RED << "Error: some error occured" << RESET << '\n';
  }

  m_mutex.lock();
    m_finished = true;
  m_mutex.unlock();

  for (auto &thread : m_threads)
    thread.join();
}

void dff::print_dups() noexcept
{
  for (const auto &[_, rec] : m_store) {
    if (rec.size() > 1) {
      std::cout << "\n\x1b[33m";
      for (const auto &path : rec)
        std::cout << path << '\n';
      std::cout << "\x1b[m\n";
    }
  }
}

auto dff::export_dups(const char *filename) -> void
{
  auto outfile = std::ofstream(filename);
  if (!outfile.is_open()) {
    std::cerr << "Error: can not open file " << filename << std::endl;
    std::exit(1);
  }

  for (const auto &[_, record] : m_store) {
    if (record.size() > 1) {
      for (const auto &path : record)
        outfile << fs::canonical(path) << ',';
      outfile << '\n';
    }
  }
}
