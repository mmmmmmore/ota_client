#include "build_info.h"
#include "build_info_version.h"

#ifndef BUILD_OTA_VER
#define BUILD_OTA_VER "0.0.0"
#endif

const char build_ota_ver[] = BUILD_OTA_VER; // embed build-time OTA version
