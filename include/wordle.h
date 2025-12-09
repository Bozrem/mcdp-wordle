/**
 * @file utils.h
 * @brief Header file for utilities
 *
 * Header file for all the utilities used throughout the algorithm
 *
 * @author Remy Bozung
 * @date 2025-12-07
 */

#include "structs.h"

void step_bitmap(const state_bitmap_t *old_state, state_bitmap_t *new_state, int action_ind, int answer_ind);
uint8_t generate_pattern(int action_ind, int answer_ind);
