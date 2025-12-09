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

void parse_inputs(int argc, char **argv, run_config_t *config);

int main(int argc, char **argv) {
    run_config_t config;
    parse_inputs(argc, argv, &config);

    return 0;
}


/**
 * parse_inputs - Verifies and parses arg inputs into a config struct
 */
void parse_inputs(int argc, char **argv, run_config_t *config) {
/*
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"help",    no_argument,    0,  'h'},
        {"version", no_argument,    0,  'v'},
        {"checkpoint", required_argument, 0, 'o'},
    }
*/
}
