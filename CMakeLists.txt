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
set(PROJECT_VER "1.0.1" )

# 2.0.1 as big change version, add ota feature

# 定义项目名称和版本
project(ota_client VERSION 1.0.1)

add_definitions(-DPROJECT_VER=\"${PROJECT_VER}\")


# SPIFFS 构建规则（如果启用）
# spiffs_create_partition_image(spiffs spiffs FLASH_IN_PROJECT "spiffs")

# ====== 新增固件重命名规则 ======
# 编译完成后，将生成的固件文件重命名为带版本号的文件
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}.bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
            ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_v${PROJECT_VER}.bin
    COMMENT "Renaming firmware to ${CMAKE_PROJECT_NAME}_v${PROJECT_VER}.bin"
)

