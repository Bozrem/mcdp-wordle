#include "structs.h"
#include "wordle.h"

uint8_t generate_pattern_lookup(int action_ind, int answer_ind);

/**
 * step_bitmap - Main Wordle computation, takes an action in a game and narrows down the possibilities
 * @param old_state - The old bitmap indicating the answers previously reduced
 * @param new_state - Pointer to place the new bitmap after eliminations
 * @param action_ind - Action bitmap index to take
 * @param answer_ind - Answer/State bitmap index that is the answer to evaluate on
 */
void step_bitmap(const state_bitmap_t *old_state, state_bitmap_t *new_state, int action_ind, int answer_ind) {
    
}


/**
 * generate_pattern - Given an action and answer, returns the pattern of what colors work
 * @param action_ind - Dictionary index of the action / guess being taken
 * @param answer_ind - Dictionary index of the answer to evaluate from
 * @returns color_pattern - The pattern ID representing the color set returned
 */
uint8_t generate_pattern(int action_ind, int answer_ind) {
    // Not implemented yet
    return 0;
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
