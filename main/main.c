void app_main(void) {
    // 1. 初始化 Wi-Fi STA
    wifi_sta_init("GW_SSID", "GW_PASSWORD");

    // 2. 等待连接成功后，启动 TCP 客户端
    tcp_client_start("192.168.4.1", 5000);

    // 3. 创建任务接收 GW 下发的消息
    xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);

    // 4. 示例：上报进度
    tcp_client_send("{\"client\":\"Client_1\",\"progress\":10,\"result\":\"running\"}");
}
