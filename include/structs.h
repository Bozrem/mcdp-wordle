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
#include <stdio.h>


// --- Bitmapping ---
typedef struct {
    uint64_t map[(ANSWERS + 63) / 64]; // TODO: Figure out how to do this without a hardcode
} state_bitmap_t;

typedef struct {
    uint64_t map[(GUESSES + 63) / 64]; // TODO: Above
} action_bitmap_t;

// --- State Node ---

// Q Entry
typedef struct {
    double q;               // Cached version for reading, gets replaced from calc below
    double sum_value;       // Components of Q, just separated for more FP accuracy
    int visit_count;
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

typedef struct state_node_s {
    state_bitmap_t state;   // The answers still possible in this state
    action_bitmap_t action; // The informationally unique and useful remaining actions
    uint64_t hash;          // For lookups in the main table

    double v;
    int best_action;        // Which Q gives us that V?
    state_status_t status;

    int num_actions;
    q_entry_t *q_values;    // Pointer to dynamic array in mem

    omp_lock_t lock;        // Mutex lock for status and V
    struct state_node_s *next_state; // Chaining Linked List
} state_node_t;

// --- Global Memory ---

typedef enum {
    STAGE_FRESH = 0,        // Making this 0 also makes it easy to tell when we didn't load from a restore file
    STAGE_BUILDING_LUT = 1,
    STAGE_SOLVING = 2,
    STAGE_DONE
} run_stage_t;

typedef struct {
    int dp_threshold;       // Amount of remaining possible answers to trigger full DP
    int pure_dp_mode;       // Running a pure DP solution instead of algorithm
    int batch_size;         // How many episodes do we run between checkpoints?
    double heuristic_temp;  // Temperature for heuristic softmax
 
    long megabytes_alloc;   // Amount of memory to allocate, measured in megabytes
    int hashmap_size_exp;   // Exponent for hashmap size (e.g. 2 ^ 29)

    void* base_address;     // The base address to use in memory allocation

    FILE* checkpoint_write; // FD for the checkpoint file to write to. NULL to disable checkpointing
    FILE* restore_file;     // Optional Checkpoint file to restore from, NULL when starting from scratch
    FILE* answers_text;     // File descriptor for the answers text
    FILE* guesses_text;     // File descriptor for the guesses text
} run_config_t;

// Global struct passed to all workers
typedef struct {
    run_stage_t solve_stage;

    void *mem_base;             // Base address for allocated space
    size_t mem_capacity;        // Total memory capacity
    size_t mem_top;             // Filled up to

    uint8_t *pattern_lut;       // Pointer to guesses * answers flat array

    state_node_t **states_table;// Pointer to an array of buckets
    int table_size;             // Must be a power of two for the mask to work
    int table_mask;             // just table_size - 1, but this can be used as a mask instead of modulo

    omp_lock_t *bucket_locks;   // Pointer to array of locks for strips of the hashmap
    int num_locks;              // Total locks (is a power of 2 to match the hashmap)
    int lock_mask;              // Mask for getting lock index from a hash

    run_config_t config;
} global_state_t;

// --- Statistics ---
typedef struct {
    long sum_depth;
    long iterations;
} episode_stats_t;
