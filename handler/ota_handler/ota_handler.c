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

static const char *TAG = "OTA_HANDLER_Client";
#define NVS_NAMESPACE "ota_client"
#define NVS_KEY_FLAG "ota_flag"
#define NVS_KEY_TASKID "task_id"


static bool ota_flag = false; // 掉电保持的标志
static char ota_task_id[64]= {0}; //save even reset; 


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

        size_t len=sizeof(ota_task_id);
        if (nvs_get_str(nvs, NVS_KEY_TASKID, ota_task_id, &len) == ESP_OK){
            ESP_LOGI(TAG, "Loaded task_id = %s", ota_task_id);
        }
        nvs_close(nvs);
    }
    ESP_LOGI(TAG, "OTA Handler initialized, ota_flag=%d, task_id= %s", ota_flag, ota_task_id);
}

// 保存 ota_flag and task_id;
static void ota_set_flag_task(bool ota_flag , const char *task_id) {
    nvs_handle_t nvs;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs) == ESP_OK){
        // save ota_flag
        nvs_set_u8(nvs, NVS_KEY_FLAG, ota_flag);
        // save task id 
        nvs_set_str(nvs, NVS_KEY_TASKID, task_id);
        nvs_commit(nvs);
        nvs_close(nvs);
        ESP_LOGI(TAG, "Saved ota_flag =1, and task_id = %s", task_id);
    }
}

static void ota_handler_clear_flag(){
    nvs_handle_t nvs;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs) ==ESP_OK){
        nvs_set_u8(nvs, NVS_KEY_FLAG,0);
        nvs_erase_key(nvs, NVS_KEY_TASKID);
        nvs_commit(nvs);
        nvs_close(nvs);
        ota_flag= false;
        ota_task_id[0]='\0';
        ESP_LOGI(TAG, "Cleared OTA flag and ota_task_id record after tx to GW...");
    }
}

// 构造ota state JSON 并通过 TCP 发送给 GW
esp_err_t client_send_ota_result(int sock) {
    if (sock < 0) {
        ESP_LOGE(TAG, "Invalid socket, cannot send ota info");
        return ESP_FAIL;
    }

    // Only send if there's a pending OTA record saved (persisted across restart)
    if (!ota_flag || strlen(ota_task_id) == 0) {
        ESP_LOGI(TAG, "No OTA result to report");
        return ESP_OK;
    }

    cJSON *root = cJSON_CreateObject();     // create ota result json and send to GW
    cJSON_AddStringToObject(root, "msg_type", "ota_result");
    cJSON_AddStringToObject(root, "task_id", ota_task_id);
    cJSON_AddStringToObject(root, "state", "complete");

    char *json_str = cJSON_PrintUnformatted(root);  //transfer to string format
    if (!json_str) {
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    int json_len = strlen(json_str);

    ESP_LOGI(TAG, "Sending OTA result to GW: %s", json_str);

    char *json_with_newline = malloc(json_len + 2);
    if (!json_with_newline) {
        ESP_LOGE(TAG, "Malloc failed when sending ota result");
        cJSON_Delete(root);
        free(json_str);
        return ESP_FAIL;
    }

    memcpy(json_with_newline, json_str, json_len);
    json_with_newline[json_len] = '\n';
    json_with_newline[json_len+1] = '\0';

    esp_err_t ret = tcp_client_send(json_with_newline);

    free(json_with_newline);
    cJSON_Delete(root);
    free(json_str);

    if (ret == ESP_OK) {
        // Clear persisted OTA flag only after successful transmission
        ota_handler_clear_flag();
        ESP_LOGI(TAG, "OTA result reported and cleared: %s", ota_task_id);
    } else {
        ESP_LOGW(TAG, "Failed to report OTA result, will retry later");
    }

    return ret;
}


static esp_err_t send_json_gw(const char *task_json){
    // 构造 JSON
    cJSON *root = cJSON_Parse(task_json);
    cJSON *progress = cJSON_CreateObject();
    const char *task_id = cJSON_GetObjectItem(root, "task_id")->valuestring;
    cJSON_AddStringToObject(progress, "msg_type", "ota_progress");
    cJSON_AddStringToObject(progress, "task_id", task_id);
    cJSON_AddStringToObject(progress, "state", "DWLD_DONE");
    char *progress_str = cJSON_PrintUnformatted(progress);
    int json_len = strlen(progress_str);

    ESP_LOGI(TAG, "Sending progress info to GW: %s", progress_str);

    char *json_with_newline = malloc(json_len + 2) ;
    if (json_with_newline){
        memcpy(json_with_newline, progress_str, json_len);
        json_with_newline[json_len] = '\n';
        json_with_newline[json_len+1]='\0';
        esp_err_t ret;
        ret = tcp_client_send(json_with_newline);
        free(json_with_newline);
        return ret;
    }

    ESP_LOGI(TAG, "Send ota progress DWLD_DONE : %s", progress_str);
    cJSON_Delete(root);
    free(progress_str);

    return ESP_OK;

}

// 处理 GW 下发的 OTA 任务
void ota_handler_process(const char *task_json) {
    cJSON *root = cJSON_Parse(task_json);
    if (!root) {
        ESP_LOGE(TAG, "Invalid JSON task");
        return;
    }

    const char *url       = cJSON_GetObjectItem(root, "firmware_url")->valuestring;
    const char *task_id   = cJSON_GetObjectItem(root, "task_id")->valuestring;
    ESP_LOGI(TAG, "Starting OTA from URL: %s", url);

    // 设置 ota_flag = true，表示有 OTA 任务
    ota_set_flag_task(1, task_id);
    //send DWLD_DONE to GW. 
    //constrcut ota_task_progress json to GW
    //cJSON *progress = cJSON_CreateObject();
    send_json_gw(task_json);

    //delay 5s then download and upgrade.
    vTaskDelay(pdMS_TO_TICKS(5000));
    

    // HTTP 客户端配置，加入证书
    esp_http_client_config_t http_config = {
        .url = url,
        .cert_pem = rootCA_pem_start, // 使用 server_cert.pem
    };

    // OTA 配置
    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };
    // send url


    // 执行 OTA，无论版本是升级还是降级
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Succeeded, restarting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA Failed");
        ota_set_flag_task(0, task_id); // 清除标志
        //ota_report_result("unknown", false);
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

}


