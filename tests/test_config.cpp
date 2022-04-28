#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "./test_config.h"
#include "../src/config.h"

int test_default_tourn()
{
    tourn_settings_t t = DEFAULT_TOURN;

    ASSERT(t.offline_only == DEFAULT_OFFLINE_ONLY);
    ASSERT(t.match_size == DEFAULT_MATCH_SIZE);
    ASSERT(t.deck_count == DEFAULT_DECK_COUNT);
    ASSERT(t.points_win == DEFAULT_POINTS_WIN);
    ASSERT(t.points_loss == DEFAULT_POINTS_LOSS);
    ASSERT(t.points_draw == DEFAULT_POINTS_DRAW);
    ASSERT(t.points_bye == DEFAULT_POINTS_BYE);
    ASSERT(t.type == DEFAULT_TOURN_TYPE);

    return 1;
}

int test_free_error()
{
    config_t config;
    memset(&config, 0, sizeof(config));
    config.logged_in = true;
    free_config(&config);

    memset(&config, 0, sizeof(config));
    free_config(&config);
    return 1;
}

int test_free()
{
    config_t config;
    config.logged_in = true;
    config.user.user_name = (char *) malloc(180);
    config.user.user_token = (char *) malloc(180);
    config.user.uuid = (char *) malloc(180);
    config.tourn_save_path = (char *) malloc(180);

    free_config(&config);

    ASSERT(config.tourn_save_path == NULL);
    ASSERT(config.user.user_name == NULL);
    ASSERT(config.user.user_token == NULL);
    ASSERT(config.user.uuid == NULL);


    // Test logged_in = false
    config.logged_in = false;
    config.tourn_save_path = (char *) malloc(180);
    free_config(&config);
    ASSERT(config.tourn_save_path == NULL);

    return 1;
}

SUB_TEST(config_cpp_tests,
{&test_default_tourn, "Test default tourn settings"},
{&test_free_error, "Test free error case"},
{&test_free, "Test free"}
        )

