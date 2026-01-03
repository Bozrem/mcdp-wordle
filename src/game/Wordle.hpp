#pragma once
#include "GameTypes.hpp"
#include <vector>
#include <string>
#include <array>

enum class Color : uint8_t {
    Gray = 0,
    Yellow = 1,
    Green = 2
};

class Wordle {
private:
    std::vector<std::string> answers;
    std::vector<std::string> guesses;

    std::vector<uint8_t> pattern_lut;

public:
    Wordle();
 
    // LUT orchestrator, uses compute_pattern
    void build_lut();

    // Actual number crunching for playing a Wordle guess
    static uint8_t compute_pattern(const std::string& guess, const std::string& target);

    // Prune the remaining possible answers with a guess
    void apply_guess(const StateBitmap& current_state, StateBitmap& next_state, int action_index, int answer_index) const;

    // Quick lookup for the private lut
    uint8_t get_pattern_lookup(int action_index, int answer_index) const {
        return pattern_lut[action_index * NUM_ANSWERS + answer_index];
    }

    int get_num_answers() const {return answers.size(); }
    int get_num_guesses() const {return guesses.size(); }
};
