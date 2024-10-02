#pragma once
#include <string>
#include <span>
#include "ints.hpp"

constexpr std::string to_hex_string(std::span<const u8> span, bool lower_case = true)
{
   constexpr static auto lcmap = std::array{
      '0', '1', '2', '3',
      '4', '5', '6', '7',
      '8', '9', 'a', 'b',
      'c', 'd', 'e', 'f'
   };
   constexpr static auto ucmap = std::array{
      '0', '1', '2', '3',
      '4', '5', '6', '7',
      '8', '9', 'A', 'B',
      'C', 'D', 'E', 'F'
   };

   auto u8Span = std::span<const u8>(span.data(), span.size_bytes());
   std::string output;
   output.reserve(span.size() * 2);
   const auto& map = lower_case ? lcmap : ucmap;
   for (auto i : u8Span)
   {
      output.push_back(map[i >> 4]);
      output.push_back(map[i & 0xF]);
   }
   return output;
}

constexpr bool is_all_zero(std::ranges::contiguous_range auto const& range)
{
   return std::ranges::all_of(range, [](auto&& v) { return v == 0; });
}

template <typename... T>
void exit_assert(bool cond, std::format_string<T...> reason = "", T&&... t)
{
   if (cond)
      return;

   //std::print(stderr, "[Assertion failed]");
   //std::println(stderr, reason, std::forward<T>(t)...);
   std::exit(EXIT_FAILURE);
}
