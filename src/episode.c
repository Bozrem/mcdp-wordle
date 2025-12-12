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
#include "memory.h"

#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>

typedef struct {
    state_node_t *node;
    int action_ind;
    double old_value;
} step_t;

void expand(global_state_t *global, state_node_t *parent);
double dp_evaluate_node(global_state_t *global, state_node_t *parent);
void propagate_update(global_state_t *global, step_t *trajectory, int trajectory_len, double final_v);

#define INITIAL_V 6.0

/**
 * run_episode - Parallel split point, runs and exploration and update
 * @param global - The global state struct designed by main as a guide for every episode iteration
 * @returns stats - Episode statistics to save and graph later
 */
episode_stats_t run_episode(global_state_t *global) {
    episode_stats_t stats = {0};
    step_t trajectory[20];
    int depth = 0;

    state_bitmap_t root_bitmap;
    bitmap_fill_all(&root_bitmap);

    state_node_t *current = get_or_create_node(global, &root_bitmap);

    double final_value = 0.0;

    while (1) {
        stats.sum_depth++;

        // Check terminated
        omp_set_lock(&current->lock);
        if (current->status == STATUS_SOLVED) {
            final_value = current->v;
            omp_unset_lock(&current->lock);
            break;
        }
        omp_unset_lock(&current->lock);
 
        // Check DP threshold
        int remaining_states = bitmap_total(&current->state);
        if (remaining_states <= global->config.dp_threshold) {
            final_value = dp_evaluate_node(global, current);
            break;
        }

        // Expand empty nodes
        if (current->status == STATUS_NONE)
            expand(global, current);

        // Softmap action selection
        double sum_exp = 0.0;
        double logits[current->num_actions];
        int valid_indicies[current->num_actions];
        int valid_count = 0;

        for (int i = 0; i < current->num_actions; i++) {
            q_entry_t *q_entry = &current->q_values[i];

            // If this child is solved, no point in exploring it
            if (q_entry->solved_children == q_entry->total_children && q_entry->total_children > 0)
                continue;

            // Softmax
            logits[valid_count] = exp(-(q_entry->q) / global->config.heuristic_temp);
            sum_exp += logits[valid_count];
            valid_indicies[valid_count] = i; // TODO: Revisit if this array is needed. May just be able to get away with the 0%?
            valid_count++;
        }

        if (valid_count == 0) {
            omp_set_lock(&current->lock);
            current->status = STATUS_SOLVED;
            final_value = current->v; // When all children are solved, the parent is solved
            omp_unset_lock(&current->lock);
            break; // This should be getting applied in the backup, but I'm going to use this as a sanity check
        }

        // Choosing a weighed random from the softmaxed values
        double r = (double)rand() / RAND_MAX * sum_exp;
        double logit_sum = 0.0;
        int chosen_index;

        // Loop through the valid ones to find the chosen
        for (int i = 0; i < valid_count; i++) {
            logit_sum += logits[i];
            if (r <= logit_sum) {
                chosen_index = valid_indicies[i];
                break;
            }
        }

        // Action selection
        // Randomly choose an answer
        // TODO: Find a way to specifically select an unsolved one
        //   I came up with adding a state_bitmap_t solved varaible to q entries, but that's a ton of memory
        //   You can also try a loop through and check if the children are solved, but that could take a lot of compute?
        int random_answer = bitmap_get_nth_set_bit(&current->state, rand() % remaining_states);

        state_bitmap_t next_bitmap;
        step_bitmap(global, &current->state, &next_bitmap, chosen_index, random_answer);

        state_node_t *child = get_or_create_node(global, &next_bitmap);

        trajectory[depth].node = current;
        trajectory[depth].action_ind = chosen_index;
        if (child->status == STATUS_NONE)
            trajectory[depth].old_value = INITIAL_V;
        else
            trajectory[depth].old_value = child->v;
        depth++;

        current = child;
    }

    propagate_update(global, trajectory, depth, final_value);

    return stats;
}

/**
 * expand - A lot of the core of the algorithm, this is where we build and init the children of a node
 * @param global - Global state to use for allocations and accesses
 * @param parent - Parent node to expand from
 */ 
void expand(global_state_t *global, state_node_t *parent) { // Consider making int for status codes?
    omp_set_lock(&parent->lock);
    if (parent->status != STATUS_NONE) {
        omp_unset_lock(&parent->lock);
        return; // Another thread got the race conditition and has already expanded
    }

    // TODO: This needs to be moved out of the stack, could be too big
 
    // Not implemented yet, but this all would essentially call step_bitmap a bunch of times and check for duplicates
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
    omp_unset_lock(&parent->lock);
}

/**
 * propagate_update - The update rule for this algorithm, goes up only when new path is better
 * @param global - Pointer to the global struct for accessing state nodes and q entries
 * @param trajectory - An array of the steps taken during this episode
 * @param trajectory_len - Length of array above, cannot be >6
 * @param final_v - The V value hit at the bottom from DP or solved children
 */
void propagate_update(global_state_t *global, step_t *trajectory, int trajectory_len, double final_v) {
    double current_val = final_v;

    // Iterate backward through the trajectory
    for (int i = trajectory_len - 1; i >= 0; i--) {
        state_node_t *node = trajectory[i].node;
        int action_ind = trajectory[i].action_ind;
        double delta = current_val - trajectory[i].old_value;

        q_entry_t *q = &node->q_values[action_ind];

        omp_set_lock(&q->lock);

        q->sum_value += (delta / q->total_children); // Pessimistic Rolling Update

        double new_q = q->q;
        omp_unset_lock(&q->lock);

        // Bubble the V
        omp_set_lock(&node->lock);

        if (new_q < node->v) {
            // That means this action is better than the previous best known
            node->v = new_q;
            node->best_action = action_ind;
            current_val = node->v; // Continue propagation
        } else {
            // This path got better, but it's still worse than another
            omp_unset_lock(&node->lock);
            break; // Stop propagation
        }
        omp_unset_lock(&node->lock);
    }
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
