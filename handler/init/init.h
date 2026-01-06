#ifndef INIT_H
#define INIT_H
// handler/init/init.h
#pragma once

#include "esp_err.h"
#include <stdbool.h>

// Initialize platform subsystems (idempotent)
esp_err_t platform_init(void);

// Deinitialize platform subsystems (reverse of platform_init)
esp_err_t platform_deinit(void);

// Query whether platform is initialized
bool platform_is_initialized(void);

// Manage IGN (monitor GPIO and call init/deinit accordingly); starts a background task
esp_err_t ign_mgmt(void);

#endif  //define init.h
