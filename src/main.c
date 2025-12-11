/**
 * @file main.c
 * @brief Main code for high level control and checkpointing
 *
 * Sets up the environment, runs the main loop with parallel, and does checkpointing
 *
 * @author Remy Bozung
 * @date 2025-12-07
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "structs.h"
#include "memory.h"
#include "episode.h"

void parse_inputs(int argc, char **argv, run_config_t *config);
int initialize_hash_table(global_state_t *global);

int main(int argc, char **argv) {
    global_state_t global;
    parse_inputs(argc, argv, &global.config);

    initialize_memory_module(); // Some sort of call to memory.c for initial setup
    initialize_hash_table(&global);
    
    build_transition_table(&global);

    // Do I need to initialize the root node somehow here? Is it best for that to always be at a set location instead of in the hash table?

    episode_stats_t total_stats; // Is this already all zeroes?

    while (1) { // This needs to check the root node somehow?
        #pragma omp parallel for
        for (int i = 0; i < global.config.batch_size; i++) {
            episode_stats_t episode_stats = run_episode(&global);
            // Add the episode stats to the total
            // Is this an efficient design for the stats? Does it need mutex? Is that too slow?
        }
        // Take some sort of checkpoint here, probably either should go in memory.c or it's own?
        // Also want to take a snapshot of the current stats for nice graphing when it's done
        // How can I structure it to work with getting stopped by OS or ctrl+c?
    }

    deallocate_memory();

    return 0;
}

/**
 * parse_inputs - Verifies and parses arg inputs into a config struct
 * @param argc
 * @param argv
 * @param config - Pointer to the configuration struct to fill out
 */
void parse_inputs(int argc, char **argv, run_config_t *config) {
    // TODO: Figure out getopt_long and what inputs it should take
    // This can just call exit(1) if the wrong inputs are given
}

/**
 * initialize_hash_table - Builds the global state hash table
 * @param global - Pointer to the global struct to place a pointer to the hash table
 * @returns a status code, -1 indicates failure
 */
int initialize_hash_table(global_state_t *global) {
    // TODO: Implement
}


