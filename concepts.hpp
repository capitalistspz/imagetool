#pragma once
#include <concepts>
#include <ranges>

template <typename Range, typename Elem>
concept contiguous_range_of = std::ranges::contiguous_range<Range> && std::is_same_v<std::ranges::range_value_t<Range>, Elem>;