#ifndef IGN_HANDLER_H
#define IGN_HANDLER_H

#include "esp_err.h"
#include <stdbool.h>

// Initialize IGN handler (sets up GPIO input and event task)
esp_err_t ign_init(void);

// Get last known IGN pin state
bool ign_get_state(void);

#endif // IGN_HANDLER_H
