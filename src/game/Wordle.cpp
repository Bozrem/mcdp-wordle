#include "Wordle.hpp"

Wordle::Wordle() {
    // TODO: Load files into vectors
    answers.reserve(NUM_ANSWERS);
    guesses.reserve(NUM_GUESSES);
    pattern_lut.resize(NUM_GUESSES * NUM_ANSWERS); // Also inits to 0s
}

void Wordle::build_lut() {
    #pragma omp parallel for collapse(2)
    for (int g = 0; g < NUM_GUESSES; ++g) {
        for (int a = 0; a < NUM_ANSWERS; ++a) {
            pattern_lut[g * NUM_ANSWERS + a] = compute_pattern(guesses[g], answers[a]); // Shouldn't get out of bounds
        }
    }
}

uint8_t Wordle::compute_pattern(const std::string& guess, const std::string& target) {
    std::array<Color, 5> colors = {Color::Gray, Color::Gray, Color::Gray, Color::Gray, Color::Gray};
    std::array<uint8_t, 26> target_freq = {0};

    for (char c : target) target_freq[c - 'a']++; // the - 'a' lowers a to index 0

    // Green pass
    for (int i = 0; i < 5; ++i) {
        if (guess[i] == target[i]) {
            colors[i] = Color::Green;
            target_freq[target[i] - 'a']--;
        }
    }

    // Yellow pass
    for (int i = 0; i < 5; ++i) {
        if (colors[i] == Color::Green) continue;
        int char_index = guess[i] - 'a';
        if (target_freq[char_index] > 0) {
            colors[i] = Color::Yellow;
            target_freq[char_index]--;
        }
    }

    // Base 3 encode to the pattern
    uint8_t pattern = 0;
    int multiplier = 1;
    for (auto c : colors) { // How can I do this with the enum?
        pattern += static_cast<uint8_t>(c) * multiplier;
        multiplier *= 3;
    }
    return pattern;
}

void Wordle::apply_guess(const StateBitmap& current_state, StateBitmap& next_state, int action_index, int answer_index) const {
    uint8_t target_pattern = get_pattern_lookup(action_index, answer_index);
    next_state.reset();

    // TODO: Apply bitmap SIMD optimization
    for (int i = 0; i < NUM_ANSWERS; i++) {
        if (current_state.test(i)) {
            if (get_pattern_lookup(action_index, answer_index) == target_pattern)
                next_state.set(i);
        }
    }
}
