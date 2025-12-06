#include "ota_handler.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_app_desc.h"
#include "tcp_client.h"   // 用于上报结果给 GW

// 引用 server_cert.pem 的链接符号
extern const char server_cert_pem_start[] asm("_binary_server_cert_pem_start");
extern const char server_cert_pem_end[]   asm("_binary_server_cert_pem_end");

extern const char rootCA_pem_start[] asm("_binary_rootCA_pem_start");
extern const char rootCA_pem_end[]   asm("_binary_rootCA_pem_end");

static const char *TAG = "OTA_HANDLER";
static const char *NVS_NAMESPACE = "ota_ns";
static const char *NVS_KEY_FLAG = "ota_flag";

static bool ota_flag = false; // 掉电保持的标志

// 初始化 NVS
void ota_handler_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 读取 ota_flag
    nvs_handle_t nvs;
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (ret == ESP_OK) {
        uint8_t flag = 0;
        if (nvs_get_u8(nvs, NVS_KEY_FLAG, &flag) == ESP_OK) {
            ota_flag = (flag == 1);
        }
        nvs_close(nvs);
    }
    ESP_LOGI(TAG, "OTA Handler initialized, ota_flag=%d", ota_flag);
}

// 保存 ota_flag
static void ota_set_flag(bool flag) {
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs));
    ESP_ERROR_CHECK(nvs_set_u8(nvs, NVS_KEY_FLAG, flag ? 1 : 0));
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
    ota_flag = flag;
}

// 处理 GW 下发的 OTA 任务
void ota_handler_process(const char *task_json) {
    cJSON *root = cJSON_Parse(task_json);
    if (!root) {
        ESP_LOGE(TAG, "Invalid JSON task");
        return;
    }

    const char *url = cJSON_GetObjectItem(root, "firmware_url")->valuestring;
    ESP_LOGI(TAG, "Starting OTA from URL: %s", url);

    // 设置 ota_flag = true，表示有 OTA 任务
    ota_set_flag(true);

    // HTTP 客户端配置，加入证书
    esp_http_client_config_t http_config = {
        .url = url,
        .cert_pem = rootCA_pem_start, // 使用 server_cert.pem
    };

    // OTA 配置
    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    // 执行 OTA，无论版本是升级还是降级
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Succeeded, restarting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA Failed");
        ota_set_flag(false); // 清除标志
        ota_report_result("unknown", false);
    }

    cJSON_Delete(root);
}

// 启动时检查 OTA 记录
void ota_record_check(void) {
    if (!ota_flag) {
        ESP_LOGI(TAG, "No previous OTA task");
        return;
    }

    // 获取当前运行版本
    const esp_app_desc_t *app_desc = esp_app_get_description();
    const char *version = app_desc->version;

    ESP_LOGI(TAG, "OTA record found, current version=%s", version);

    // 上报成功结果
    ota_report_result(version, true);

    // 清除标志
    ota_set_flag(false);
}

// 上报结果给 GW
void ota_report_result(const char *version, bool success) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "{\"client\":\"Client_1\",\"version\":\"%s\",\"result\":\"%s\"}",
             version, success ? "success" : "fail");

    tcp_client_send(buffer);
    ESP_LOGI(TAG, "Reported OTA result: %s", buffer);
}
