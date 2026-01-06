cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# 添加额外组件目录
set(EXTRA_COMPONENT_DIRS 
        app/led_control
        app/motor_control
        app/client_register
        components/common_gpio
        protocol/wifi_sta
        protocol/control_mgmt
        protocol/tcp_client
        handler/init
        handler/motor_handler
        handler/ota_handler
        handler/msg_handler
        utility/cJSON
        )

# ====== 新增版本号配置 ======
# 可以在命令行通过 -DPROJECT_VER=1.0.1 覆盖
set(PROJECT_VER "1.1.0" CACHE STRING "Project version (default)")

# OTA version: can be overridden at configure time with -DOTA_VER=...
set(OTA_VER "${PROJECT_VER}" CACHE STRING "OTA version to embed in build and binary name")

# 2.0.1 as big change version, add ota feature

# 定義項目名稱和版本
project(ota_client VERSION ${PROJECT_VER})

add_definitions(-DPROJECT_VER="${PROJECT_VER}")
add_definitions(-DOTA_VER="${OTA_VER}")

# ====== 新增固件重命名規則 ======
# 編譯完成後，將生成的固件文件重命名為帶版本號的文件
# Create a custom command that copies the generated .bin to a versioned file
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_v${OTA_VER}.bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_v${OTA_VER}.bin
    COMMENT "Renaming firmware to ${CMAKE_PROJECT_NAME}_v${OTA_VER}.bin"
)
add_custom_target(copy_firmware ALL DEPENDS ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_v${OTA_VER}.bin)
# Ensure the standard project binary is generated before copying
add_dependencies(copy_firmware gen_project_binary)


# SPIFFS 构建规则（如果启用）
# spiffs_create_partition_image(spiffs spiffs FLASH_IN_PROJECT "spiffs")

# ====== 新增固件重命名规则 ======
# 编译完成后，将生成的固件文件重命名为带版本号的文件
#add_custom_command(
#    TARGET ${CMAKE_PROJECT_NAME}.bin POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
#            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_v${PROJECT_VER}.bin
#    COMMENT "Renaming firmware to ${CMAKE_PROJECT_NAME}_v${PROJECT_VER}.bin"
#)

