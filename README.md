# ota_gw
This code repository for GW function based on ESP32


client_project/
├── main/
│   ├── main.c                # 程序入口，初始化 Wi-Fi、OTA Handler、设备控制
│   └── CMakeLists.txt
├── protocol/
│   ├── wifi_sta/             # Wi-Fi STA 模块
│   │   ├── wifi_sta.c
│   │   ├── wifi_sta.h
│   │   └── CMakeLists.txt
|----handler
│----├── ota_handler/          # OTA Handler 模块（AB分区）
│   │   ├── ota_handler.c
│   │   ├── ota_handler.h
│   │   └── CMakeLists.txt
│   ├── device_control/       # 电机和LED控制模块
│   │   ├── device_control.c
│   │   ├── device_control.h
│   │   └── CMakeLists.txt
│   └── common/               # 公共工具（日志、配置解析等）
│       ├── utils.c
│       ├── utils.h
│       └── CMakeLists.txt
└── sdkconfig                 # 配置文件，包含分区表、Wi-Fi参数等



