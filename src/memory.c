/**
 * @file memory.c
 * @brief a custom memory management module for best performance
 *
 * Custom memory management
 *
 * @author Remy Bozung
 * @date 2025-12-10
 */

#include "structs.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>

void reinit_locks_post_restore(global_state_t *global);

global_state_t* setup_memory(run_config_t config) {
    size_t capacity = (size_t)config.megabytes_alloc * 1024L * 1024L;

    void *base_ptr = mmap(config.base_address, capacity,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANON | MAP_FIXED,
                          -1, 0); // no file descriptor or offset
 
    if (base_ptr == MAP_FAILED) {
        if (errno == EEXIST)
            fprintf(stderr, "ERROR: Requested base address of %p is already in use.\n", config.base_address);
        else
            perror("ERROR: mmap failed");
        exit(1);
    }

    global_state_t *global = (global_state_t *)base_ptr;

    if (config.restore_file) {
        printf("Restoring from saved checkpoint...\n");

        size_t read_bytes = fread(base_ptr, 1, capacity, config.restore_file);
        if (read_bytes <= 0) {
            perror("Failed to read restore file");
            exit(1);
        }

        // global->config = config; // If trying to overwrite the config of the restored. Probably a bad idea, but this is where it can be done

        reinit_locks_post_restore(global);
    } else {
        memset(global, 0, sizeof(global_state_t));

        global->mem_base = base_ptr;
        global->mem_capacity = capacity;
        global->config = config;

        global->mem_top = sizeof(global_state_t); // Because global is at base
    }

    return global;
    // TODO: Need a way to identify what stage we were in with a reload.
    //  Can definitely require everything being initialized, but how do we know if we are still building the LUT or running episodes?
}

/**
 * save_checkpoint - memcopies the whole current block of written data to the checkpoint fd
 * @param global - Has the checkpoint file descriptor and memory info
 * @returns status - -1 for failure
 */
int save_checkpoint(global_state_t *global) {
    if (!global->config.checkpoint_write) return 0; // Disabled

    size_t bytes_to_write = global->mem_top; // Read this once to avoid concurrency issues
    // Nothing gets deleted in this memory system, so if anything changes it's just after this and doesn't get saved

    rewind(global->config.checkpoint_write); // Go to start
    size_t written = fwrite(global->mem_base, 1, bytes_to_write, global->config.checkpoint_write);

    if (written != bytes_to_write) {
        perror("Failed to make a full checkpoint");
        return -1;
    }

    fflush(global->config.checkpoint_write); // Make sure it all goes through
    printf("Checkpoint saved, mem size is %lu MB\n", bytes_to_write / (1024*1024));
    return 0;
}

/**
 * mem_alloc - Acts as a fast malloc replacement - 
 * @param global - Used to see base memory information
 * @param size - The amount of bytes to allocate
 * @returns pointer to new block
 */
void* mem_alloc(global_state_t *global, size_t bytes) {
    size_t aligned = (bytes + 7) & ~7; // Ceils to the nearest word

    size_t old_top = __sync_fetch_and_add(&global->mem_top, aligned); // Atomic instruction

    if (old_top + aligned > global->mem_capacity) {
        fprintf(stderr, "ERROR: Out of memory! Failed with a request for %lu more, capacity is %lu\n",
                bytes, global->mem_capacity);
        exit(1);
    }

    return (void *)((uint8_t*)global->mem_base + old_top);
}

/**
 * init_pattern_lut - Initializes the wordle game lookup table in memory, requires answer_count and guess_count to be set
 * @param global - For memory info and guess and answer counts
 * @returns status - -1 for failure
 */
int init_pattern_lut(global_state_t *global) {
    // TODO:
    // Makes a matrix answers x guesses
    // These are uint8_t, since all possible color patterns can fit between 0 and 242 (3^5)
    // Should memset all to 255, which represents unknown
}

/**
 * init_lock_array - Initializes the locking array for the hashmap
 * @param global - For memory info and hashmap size
 * @returns status - -1 for failure
 */
int init_lock_array(global_state_t *global) {
    // TODO:
    // Need to figure out how to calculate how many locks we want based on hashmap size that we can have a consistent mask
    // Or better yet, make a config value specifying how many locks per hash (as an 2 ^ exponent)
}

/**
 * reinit_locks_post_restore - Unfortunately, locks are processs specific and can't be restored, so they need to be reset
 * @param global - All the info needed for every lock location
 */
void reinit_locks_post_restore(global_state_t *global) {
    printf("Reinitializing hashmap locks...\n");

    // Hashmap locks
    #pragma omp parallel for
    for (int i = 0; i < global->num_locks; i++)
        omp_init_lock(&global->bucket_locks[i]);

    printf("Reinitializing state and Q locks...\n");

    // Individual state locks
    // This is slow and sucks, but there's not really a better way to do it
    #pragma omp parallel for schedule(dynamic) // Dynamic ensures threads don't sit around if there's no states in a bucket
    for (int i = 0; i < global->table_size; i++) {
        state_node_t *node = global->states_table[i];
        while (node) {
            omp_init_lock(&node->lock);

            // Q entries if it has them
            if (node->q_values) {
                for (int q = 0; q < node->num_actions; q++)
                    omp_init_lock(&node->q_values[q].lock);
            }
            node = node->next_state; // Follow the chain
        }
    }
}

/**
 * init_lock_array - Initializes the main state hashmap, which are just pointers to states
 * @param global - For memory info and hashmap size
 * @returns status - -1 for failure
 */
int init_hashmap(global_state_t *global) {
    // TODO:
    // This uses the global config value for size. Making it exponents of 2 makes the hashing faster for modulo
}

/**
 * get_or_create_node - Goes through the hashmap to find a state
 * @param global - Global for memory info, lock locations, hashmap locations
 */
state_node_t *get_or_create_node(global_state_t *global, state_bitmap_t *state) {
    // TODO:
    // Hash the state
    // Lock that part of the hashmap
    // Move through the chain either until we find it or there are no more
    // Either return it or allocate a new one
    // Need to figure out if we should be initializing Q tables here too for more cache coherence
}

