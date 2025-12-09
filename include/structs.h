/**
 * @file structs.h
 * @brief Main structures file
 *
 * This structures file contains all the key structs like bitmaps, statenodes,
 * qentry, and solvestate
 *
 * @author Remy Bozung
 * @date 2025-12-07
 */
#pragma once

#include "config.h"
#include <stdint.h>
#include <omp.h>
#include <stddef.h>


// --- Bitmapping ---
typedef struct {
    uint64_t map[(ANSWERS + 63) / 64];
} state_bitmap_t;

typedef struct {
    uint64_t map[(GUESSES + 63) / 64];
} action_bitmap_t;

// --- State Node ---

// Q Entry
typedef struct {
    double q;               // Current Q value
    int total_children;     // Number of offshoots
    int solved_children;    // How many children are done

    omp_lock_t lock;
    // We know it's solved when solved_children == total_children
} q_entry_t;

// State statuses
typedef enum {
    STATUS_NONE = 0,        // Has been seen when initializing a parent, but not ever explored
    STATUS_INIT = 1,        // We have hit at least once and initialized children, but not solved
    STATUS_SOLVED = 2,      // This state is completely solved
} state_status_t;

// State Node, pretty much the main struct
typedef struct {
    state_bitmap_t state;   // The answers still possible in this state
    // action_bitmap_t action; // The informationally unique remaining actions
    uint64_t hash;          // For lookups in the main table

    double v;
    int best_action;        // Which Q gives us that V?

    int num_actions;        // How large is the following array?
    q_entry_t *q_values;    // Q values. With action pruning, this should be separated in memory for space efficiency

    state_status_t status;

    omp_lock_t lock;        // Mutex lock for status and V
} state_node_t;

// --- Global Struct ---

// TODO find a smarter way to enable or disable tracking actions based on if we have action pruning enabled or not

// Configuration type for global
typedef struct {
    int dp_threshold;       // Amount of remaining possible answers to trigger full DP
    int pure_dp_mode;       // Running a pure DP solution instead of algorithm
} run_config_t;

// Global struct passed to all workers
typedef struct {
    state_node_t *hash_table;  // Hash table with all the states // TODO consider if this is worth it's own struct
    size_t table_size;
    run_config_t config;
    // TODO: Some sort of statistics
} global_state_t;
