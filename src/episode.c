/**
 * @file episode.c
 * @brief The main algorithm; Runs an exploration and does an update
 *
 * Outlines both high level flow and the heavy loop computation for an episode
 *
 * @author Remy Bozung
 * @date 2025-12-07
 */

#include "structs.h"
#include "wordle.h"

typedef struct {
    state_node_t *node;
    int action_ind;
} step_t;

/**
 * run_episode - Parallel split point, runs and exploration and update
 * @param global - The global state struct designed by main as a guide for every episode iteration
 * @returns stats - Episode statistics to save and graph later
 */
episode_stats_t run_episode(global_state_t *global) {
    // TODO:
    return (episode_stats_t){0}; // Not implemented
}

/**
 * expand - A lot of the core of the algorithm, this is where we build and init the children of a node
 * @param global - Global state to use for allocations and accesses
 * @param parent - Parent node to expand from
 */ 
void expand(global_state_t *global, state_node_t *parent) { // Consider making int for status codes?
    // 0. Gain lock

    // 1. Get all child node bitmaps across all actions
    //      Using the wordle.c functions

    // 2. Prune actions
    //      Use the hash of those bitmaps to identify any duplicates
    //      If two guesses result in identical bitmaps, they are informationally identical, so we only track one
    //      We can do the same for any that result in the already existing bitmap, since that means they're useless
    //      TODO: Is that second one even possible if the first is being applied by the parent? (except in the root)

    // 3. For the remaining actions, initialize a q entry in the parent
    //      This requires an allocation for the q array
    //      Also set the state size within that q entry to be used for the heuristic

    // 4. Finalize parent metadata and unlock
}

/**
 * propagate_update - The update rule for this algorithm, goes up only when new path is better
 * @param global - Pointer to the global struct for accessing state nodes and q entries
 * @param trajectory - An array of the steps taken during this episode
 * @param trajectory_len - Length of array above, cannot be >6
 * @param final_v - The V value hit at the bottom from DP or solved children
 */
void propagate_update(global_state_t *global, step_t *trajectory, int trajectory_len, double final_v) {
    // 1. Iterate backwards up trajectory, curious how much worse it is to do recursively?
    // 2. Get the delta
    // 3. Update the Q value (with concurrency)
    // 4. If the next parent's V is worse than this new Q, replace it and go back to 1, otherwise done
}

/**
 * dp_evaluate_node - Classical dynamic programming algorithm to fully solve a node
 * @param global - The global struct used for accessing states and config
 * @param parent - Parent node to evaluate
 * @returns The true expected guesses for this state with optimal play
 */ 
double dp_evaluate_node(global_state_t *global, state_node_t *parent) {
    // TODO:
    //  Likely recursive
    //  Need to consider also how we do updates with subnodes
    //      If we can reach the same state in multiple ways, and this DP solves it, is there some need for updates elsewhere, or marking the state?
    //      Difficult. Not sure if it's even possible for that to happen, in terms of encountering an unsolved but initialized one that is already below threshold
    return 0.0; // Not implemented
}
