/**
 * @file wordle.c
 * @brief All of the complex highly optimized Wordle logic
 *
 * Handles all the Wordle logic. Can either run in a pre-computed or JIT mode for Wordle simulations
 *
 * @author Remy Bozung
 * @date 2025-12-07
 */

#include "structs.h"
#include "wordle.h"
#include "config.h"
#include <string.h>
#include <ctype.h>

// Values for Base-3 encoding
#define COLOR_GRAY 0
#define COLOR_YELLOW 1
#define COLOR_GREEN 2

uint8_t generate_pattern_lookup(global_state_t *global, int action_ind, int answer_ind);
const char* get_answer_str(global_state_t *global, int answer_ind);
const char* get_action_str(global_state_t *global, int answer_ind);


/**
 * step_bitmap - Main Wordle computation, takes an action in a game and narrows down the possibilities
 * @param old_state - The old bitmap indicating the answers previously reduced
 * @param new_state - Pointer to place the new bitmap after eliminations
 * @param action_ind - Action bitmap index to take
 * @param answer_ind - Answer/State bitmap index that is the answer to evaluate on
 */
void step_bitmap(global_state_t *global, const state_bitmap_t *old_state, state_bitmap_t *new_state, int action_ind, int answer_ind) {
    uint8_t match_pattern = generate_pattern_lookup(global, action_ind, answer_ind);

    for (int i = 0; i < sizeof(state_bitmap_t) * 8; i++) {
        // For each state in the bitmap
        // TODO: This is the idea, but there's a lot of optimization to be done here
        if (bitmap_get(old_state, i) == 0)
            bitmap_set(new_state, i, 0);
        else
            bitmap_set(new_state, i, match_pattern == generate_pattern(action_ind, i));

        /* Optimization:
         *  Memset the new state all to 0 initially (since most of the time there are only a few 1s in low nodes)
         *  Having a lookup table makes this SOOOOO fast, because the CPU can use vector operations if we organize it right
         *  There's some sort of ctzll builtin thing I can use to identify the states
         *  Just skips when whole blocks of 64 are 0
         */
    }
}


/**
 * generate_pattern - Given an action and answer, returns the pattern of what colors work
 * @param action_ind - Dictionary index of the action / guess being taken
 * @param answer_ind - Dictionary index of the answer to evaluate from
 * @returns color_pattern - The pattern ID representing the color set returned
 */
uint8_t generate_pattern(char *guess, char *target, global_state_t *global) {
    uint8_t colors[5] = {0};
    int target_freq[26] = {0};

    // Green and target frequencies
    for (int i = 0; i < 5; i++) {
        if (guess[i] == target[i])
            colors[i] = COLOR_GREEN;
        else
            target_freq[target[i] - 'a']++; // Char to index
    }

    // Yellow
    for (int i = 0; i < 5; i++) {
        if (colors[i] == COLOR_GREEN) continue; // Already handled

        int char_idx = guess[i] - 'a';
        if (target_freq[char_idx] > 0) {
            colors[i] = COLOR_YELLOW;
            target_freq[char_idx]--;
        }
        // Gray would go here as 0, but that's already initialized
    }

    // Encode Base-3
    uint8_t pattern = 0;
    int multiplier = 1;
    for (int i = 0; i < 5; i++) {
        pattern += colors[i] * multiplier;
        multiplier *= 3;
    }

    return pattern;
}

/**
 * generate_pattern_lookup - Fast O(1) retrieval from the LUT
 * @param global - Global state, used for LUT memory access
 * @param action_ind - Index of the action
 * @param answer_ind - Index of the answer
 * @returns The precomputed pattern
 */
uint8_t generate_pattern_lookup(global_state_t *global, int action_ind, int answer_ind) {
    // The LUT is a flat array of size [GUESSES * ANSWERS], row = action, col = answer
    return global->pattern_lut[action_ind * ANSWERS + answer_ind];
}

/**
 * get_answer_string - Converts an answer bitmap index into a string
 * @param global - For accessing the global dict
 * @param answer_ind - Answer bitmap index to convert
 * @return string of length 5 (plus a null terminator) with the answer word
 */
const char* get_answer_str(global_state_t *global, int answer_ind) {

}

/**
 * get_action_string - Converts an action bitmap index into a string
 * @param global - For accessing the global dict
 * @param answer_ind - Answer bitmap index to convert
 * @return string of length 5 (plus a null terminator) with the answer word
 */
const char* get_action_str(global_state_t *global, int answer_ind) {

}
