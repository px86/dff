#pragma once

#include <filesystem>
#include <forward_list>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>


#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define RESET "\x1b[m"

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
  void insert(const fs::path& path);
  auto begin() const -> std::forward_list<fs::path>::const_iterator;
  auto end() const -> std::forward_list<fs::path>::const_iterator;

private:
  std::size_t m_size = 0;
  std::forward_list<fs::path> m_files {};
};

class dff {
public:
  dff() = delete;
  dff(const dff&) = delete;
  dff(const char *rootdir, bool follow_symlinks);
  auto find_dups() noexcept -> void;
  auto print_dups() noexcept -> void;
  auto export_dups(const char *filename) -> void;
  dff& operator=(const dff&) = delete;

private:
  const char *m_rootdir {nullptr};
  bool m_follow_symlinks {false};
  Store m_store {};
  std::mutex m_mutex{};
  std::condition_variable m_cv{};
  std::queue<fs::path> m_tasks{};
  std::vector<std::thread> m_threads{};
  bool m_finished{false};
  auto pick_and_execute() -> void;
};

} // namespace pr
