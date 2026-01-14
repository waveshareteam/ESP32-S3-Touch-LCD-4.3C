#include "wifi.h"

static const char *TAG_SCAN = "wifi_scan";

wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_OWE:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_OWE");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_ENTERPRISE:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA3_ENTERPRISE:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA3_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA2_WPA3_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_ENT_192:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_WPA3_ENT_192");
        break;
    default:
        ESP_LOGI(TAG_SCAN, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
    switch (pairwise_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    case WIFI_CIPHER_TYPE_AES_CMAC128:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_AES_CMAC128");
        break;
    case WIFI_CIPHER_TYPE_SMS4:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_SMS4");
        break;
    case WIFI_CIPHER_TYPE_GCMP:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP");
        break;
    case WIFI_CIPHER_TYPE_GCMP256:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP256");
        break;
    default:
        ESP_LOGI(TAG_SCAN, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }

    switch (group_cipher) {
    case WIFI_CIPHER_TYPE_NONE:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
        break;
    case WIFI_CIPHER_TYPE_WEP40:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
        break;
    case WIFI_CIPHER_TYPE_WEP104:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
        break;
    case WIFI_CIPHER_TYPE_TKIP:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
        break;
    case WIFI_CIPHER_TYPE_CCMP:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
        break;
    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
        break;
    case WIFI_CIPHER_TYPE_SMS4:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_SMS4");
        break;
    case WIFI_CIPHER_TYPE_GCMP:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP");
        break;
    case WIFI_CIPHER_TYPE_GCMP256:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP256");
        break;
    default:
        ESP_LOGI(TAG_SCAN, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
        break;
    }
}

/* Initialize Wi-Fi as STA (Station) and set scan method */
void wifi_scan(void)
{
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;  // Maximum number of APs to be stored

    uint16_t ap_count = 0;  // Variable to hold the actual number of APs found
    memset(ap_info, 0, sizeof(ap_info));  // Clear the ap_info array

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  // Set Wi-Fi mode to Station (STA) initially
    ESP_ERROR_CHECK(esp_wifi_start());  // Start the Wi-Fi driver
    esp_wifi_scan_start(NULL, true);  // Start Wi-Fi scanning (pass NULL to scan all channels)

    ESP_LOGI(TAG_SCAN, "Max AP number ap_info can hold = %u", number);  // Log the max AP number that can be stored
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));  // Get the actual number of APs found
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));  // Get the AP records into ap_info array

    ESP_LOGI(TAG_SCAN, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);  // Log total and actual scanned APs

    // Loop through each AP and log details
    for (int i = 0; i < number; i++) {
        ESP_LOGI(TAG_SCAN, "SSID \t\t%s", ap_info[i].ssid);  // Log SSID (network name)
        ESP_LOGI(TAG_SCAN, "RSSI \t\t%d", ap_info[i].rssi);  // Log RSSI (signal strength)

        // Print authentication mode of the AP
        print_auth_mode(ap_info[i].authmode);

        // If the authentication mode is not WEP, print cipher types
        if (ap_info[i].authmode != WIFI_AUTH_WEP) {
            print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
        }

        ESP_LOGI(TAG_SCAN, "Channel \t\t%d", ap_info[i].primary);  // Log channel number
    }
}
