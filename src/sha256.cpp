#include "include/sha256.hpp"

#include <array>
#include <ios>
#include <optional>
#include <endian.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstring>

#define ROTRIGHT(a, b) ((a) >> (b) | (a) << (32 - (b)))

void sha256sum::feed(std::vector<std::uint8_t> &data)
{
  size_t data_pos = 0;

  while (data_pos < data.size()) {
    m_block.at(m_block_pos++) = data.at(data_pos++);

    if (m_block_pos == 64) {
      m_block_pos = 0;
      m_cummulative_bitlen += 512;

      update_md();
    }
  }
}

void sha256sum::feed(const char *data, size_t size)
{
  size_t data_pos = 0;

  while (data_pos < size) {
    m_block[m_block_pos++] = data[data_pos++];

    if (m_block_pos == 64) {
      m_block_pos = 0;
      m_cummulative_bitlen += 512;

      update_md();
    }
  }
}

auto sha256sum::get() -> std::array<std::uint32_t, 8>
{
  auto i = m_block_pos;

  if (m_block_pos < 56) {
    m_block[i++] = 0x80;
    while (i < 56) m_block[i++] = 0x00;
  } else {
    m_block[i++] = 0x80;
    while (i < 64) m_block[i++] = 0x00;
    update_md();
    memset(m_block.data(), 0, 56);
  }

  m_cummulative_bitlen += m_block_pos * 8;
  std::uint64_t bitlen_be = htobe64(m_cummulative_bitlen);
  memcpy(m_block.data() + 56, &bitlen_be, 8);

  update_md();

  return  md;
}

auto sha256sum::get_str() -> std::string
{
  std::ostringstream ss;
  ss << std::hex;

  auto hash = get();
  for (auto n: hash) ss << std::setw(8) << std::setfill('0') << n;

  return ss.str();
}

void sha256sum::update_md() noexcept
{
  for (int i = 0; i < 16; ++i)
    w[i] = htobe32(*(std::uint32_t *)&m_block[i * 4]);

  for (int i = 16; i < 64; ++i) {
    auto s0 = ROTRIGHT(w[i - 15], 7) ^ ROTRIGHT(w[i - 15], 18) ^
              (w[i - 15] >> 3);
    auto s1 = ROTRIGHT(w[i - 2], 17) ^ ROTRIGHT(w[i - 2], 19) ^
              (w[i - 2] >> 10);

    w[i] = w[i - 16] + s0 + w[i - 7] + s1;

  }

  auto [ a, b, c, d, e, f, g, h ] = md;

  for (int i = 0; i < 64; ++i) {
    auto S0 = ROTRIGHT(a, 2) ^ ROTRIGHT(a, 13) ^ ROTRIGHT(a, 22);
    auto maj = (a & b) ^ (a & c) ^ (b & c);
    auto t2 = S0 + maj;

    auto S1 = ROTRIGHT(e, 6) ^ ROTRIGHT(e, 11) ^ ROTRIGHT(e, 25);
    auto ch = (e & f) ^ ((~e) & g);
    auto t1 = h + S1 + ch + K[i] + w[i];

    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  md[0] += a;
  md[1] += b;
  md[2] += c;
  md[3] += d;
  md[4] += e;
  md[5] += f;
  md[6] += g;
  md[7] += h;
}

auto sha256_hash_file(const char *filepath) -> std::optional<std::string>
{
  try {
    auto file = std::ifstream(filepath, std::ifstream::binary);
    file.exceptions(std::ifstream::badbit);

    constexpr size_t buff_size = 4096;
    char buffer[buff_size];

    sha256sum ssum;

    while (file) {
      file.read(buffer, 4096);
      ssum.feed(buffer, file.gcount());
    }

    return { ssum.get_str() };
  } catch (const std::ios_base::failure &fail) {
    std::cerr << "\x1b[31m" << "Error: file "
	      << filepath << " can not be opened : "
	      << fail.what() << "\x1b[m\n";
  }

  return {};
}
