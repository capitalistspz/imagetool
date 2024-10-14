#pragma once
#include <cstdio>
#include <filesystem>
#include <ranges>
#include <utility>
#include <format>
#include <type_traits>

template <typename T>
concept readable = std::is_trivially_copy_constructible_v<T> && std::is_default_constructible_v<T>;

enum class SeekFrom : int
{
   Beg = SEEK_SET,
   End = SEEK_END,
   Cur = SEEK_CUR
};

class RBFile
{
   FILE* m_file;
public:
   explicit RBFile(const std::filesystem::path& path)
      : m_file(std::fopen(path.c_str(), "rb"))
   {
      if (m_file == nullptr)
         throw std::runtime_error("Failed to open file");
   }
   ~RBFile()
   {
      if (m_file)
         std::fclose(m_file);
   }
   RBFile(const RBFile&) = delete;
   RBFile& operator=(const RBFile&) = delete;

   RBFile(RBFile&& old) noexcept
      : m_file(std::exchange(old.m_file, nullptr)) {}

   template <readable T>
   [[nodiscard]] T Read()
   {
      uint8_t buffer[sizeof(T)];
      const auto bytesRead = std::fread(buffer, 1, sizeof(T), m_file);
      if (bytesRead != sizeof(T))
         throw std::runtime_error(std::format("Read only {:#x} bytes when reading {:#x}-byte object", bytesRead, sizeof(T)));
      return std::bit_cast<T>(buffer);
   }


   template <std::ranges::contiguous_range Range> requires readable<std::ranges::range_value_t<Range>>
   void ReadToRange(Range& range)
   {
      using elemType = std::ranges::range_value_t<Range>;
      const auto length = std::ranges::size(range);
      const auto numRead = std::fread(std::ranges::data(range), sizeof(elemType), length, m_file);
      if (numRead != length)
         throw std::runtime_error(std::format("Read only {:#x} elemnts when reading {:#x} elements", numRead, length));
   }

   void Seek(long offset, SeekFrom from = SeekFrom::Beg)
   {
      if (std::fseek(m_file, offset, static_cast<int>(from)) != 0)
         throw std::runtime_error(std::format("Failed to seek to {:#x}", offset));
   }
   void Move(long offset)
   {
      if (std::fseek(m_file, offset,SEEK_CUR) != 0)
         throw std::runtime_error(std::format("Failed to seek by {:+#x}", offset));
   }
   [[nodiscard]] unsigned Tell() const
   {
      auto pos = std::ftell(m_file);
      if (pos < 0)
         throw std::runtime_error("Failed to get file position");

      return pos;
   }
};