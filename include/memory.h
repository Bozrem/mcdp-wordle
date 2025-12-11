#pragma once

#include "structs.h"

// Memory Functions
int save_checkpoint(global_state_t *global);
global_state_t* setup_memory(run_config_t config);

// Node access
state_node_t *get_or_create_node(global_state_t *global, state_bitmap_t *state);

// Init functions
int init_pattern_lut(global_state_t *global);
int init_lock_array(global_state_t *global);
int init_hashmap(global_state_t *global);
