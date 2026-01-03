#pragma once
#include <cstdint>
#include <bitset>

constexpr int NUM_ANSWERS = 2315;
constexpr int NUM_GUESSES = 12972;

using StateBitmap = std::bitset<NUM_ANSWERS>;
using ActionBitmap = std::bitset<NUM_GUESSES>;

enum class NodeStatus : uint8_t {
    None = 0,
    Init = 1,
    Sovled = 2
};
