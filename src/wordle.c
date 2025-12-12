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

#define PATTERN_UNKNOWN 255

uint8_t generate_pattern_lookup(int action_ind, int answer_ind);



/**
 * step_bitmap - Main Wordle computation, takes an action in a game and narrows down the possibilities
 * @param old_state - The old bitmap indicating the answers previously reduced
 * @param new_state - Pointer to place the new bitmap after eliminations
 * @param action_ind - Action bitmap index to take
 * @param answer_ind - Answer/State bitmap index that is the answer to evaluate on
 */
void step_bitmap(global_state_t *global, const state_bitmap_t *old_state, state_bitmap_t *new_state, int action_ind, int answer_ind) {
    uint8_t match_pattern = generate_pattern_lookup(action_ind, answer_ind);

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
uint8_t generate_pattern(global_state_t *global, int action_ind, int answer_ind) {
    // This ONLY gets used in actually crunching the transition table, but that's 30 million combinations, so this needs to be FAST
    // To fully crunch this in under an hour with all 48 cores, we need to be able to do 174 simulations / second
    // I may want to consider multiple versions step_bitmap, one that is truely an anytime algorithm but much slower, and one that forces a full crunch first

    return 0; // Not implemented
}

/**
 * generate_pattern_lookup - Does generate_pattern but relies on having a pre-computed table
 * @param action_ind - Dictionary index of the action / guess being taken
 * @param answer_ind - Dictionary index of the answer to evaluate from
 * @returns color_pattern - The pattern ID representing the color set returned
 */ 
uint8_t generate_pattern_lookup(int action_ind, int answer_ind) {
    // NOT IMPLEMENTED
    return 0;
}

/**
 * get_answer_string - Converts an answer bitmap index into a string
 * @param global - For accessing the global dict
 * @param answer_ind - Answer bitmap index to convert
 * @return string of length 5 (plus a null terminator) with the answer word
 */
char* get_answer_string(global_state_t *global, int answer_ind) {

}

/**
 * get_action_string - Converts an action bitmap index into a string
 * @param global - For accessing the global dict
 * @param answer_ind - Answer bitmap index to convert
 * @return string of length 5 (plus a null terminator) with the answer word
 */
char* get_action_string(global_state_t *global, int answer_ind) {

}
