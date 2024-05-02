#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "ethernet_init.h"
#include "lwip/sockets.h"

#define SOCKET_PORT         5000
#define SOCKET_MAX_LENGTH   128
static const char *TAG = "tcp_server";

esp_netif_ip_info_t ip_info = {
    .ip = {.addr = ESP_IP4TOADDR(192, 168, 1, 1)},
    .netmask = {.addr =  ESP_IP4TOADDR(255, 255, 255, 0)},
    .gw = {.addr = ESP_IP4TOADDR(192, 168, 1, 1)}
};

static void start_dhcp_server_at_connection(void *esp_netif, esp_event_base_t base, int32_t event_id, void *data)
{
    esp_netif_dhcpc_stop(esp_netif);
    esp_netif_set_ip_info(esp_netif, &ip_info);
    esp_netif_dhcps_start(esp_netif);
}


void app_main(void)
{
    uint8_t eth_port_cnt = 0;
    esp_eth_handle_t *eth_handles;
    esp_netif_config_t cfg;
    esp_netif_inherent_config_t eth_netif_cfg;
    esp_netif_init();
    esp_event_loop_create_default();
    ethernet_init_all(&eth_handles, &eth_port_cnt);

#if CONFIG_EXAMPLE_ACT_AS_DHCP_SERVER
    // Config for ESP32 to act as DHCP server
    eth_netif_cfg = (esp_netif_inherent_config_t) {
        .get_ip_event = IP_EVENT_ETH_GOT_IP,
        .lost_ip_event = 0,
        .flags = ESP_NETIF_DHCP_SERVER,
        .ip_info = &ip_info,
        .if_key = "ETH_DHCPS",
        .if_desc = "eth",
        .route_prio = 50
    };
#else
    if (eth_port_cnt == 1) {
        // Use default config when using one interface
        eth_netif_cfg = *(ESP_NETIF_BASE_DEFAULT_ETH);
    } else {
        // Set behavioral config to support multiple interfaces
        eth_netif_cfg = (esp_netif_inherent_config_t) ESP_NETIF_INHERENT_DEFAULT_ETH();
    }
#endif
    char if_key_str[10];
    char if_desc_str[10];
    cfg = (esp_netif_config_t) {
        .base = &eth_netif_cfg,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    };
    for (int i = 0; i < eth_port_cnt; i++) {
        sprintf(if_key_str, "ETH_%d", i);
        sprintf(if_desc_str, "eth%d", i);
        eth_netif_cfg.if_key = if_key_str;
        eth_netif_cfg.if_desc = if_desc_str;
        eth_netif_cfg.route_prio -= i * 5;
        esp_netif_t *eth_netif = esp_netif_new(&cfg);
        ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handles[i])));

#if CONFIG_EXAMPLE_ACT_AS_DHCP_SERVER
        esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_CONNECTED, start_dhcp_server_at_connection, eth_netif);
#endif

        esp_eth_start(eth_handles[i]);
    }
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    char rxbuffer[SOCKET_MAX_LENGTH] = {0};
    char txbuffer[SOCKET_MAX_LENGTH] = {0};
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(SOCKET_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    listen(server_fd, 1);
    int transmission_cnt = 0;
    while (1) {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
        if (client_fd == -1) {
            ESP_LOGE(TAG, "An error occured when acceptig a connection (errno: %d)", errno);
        }
        while (1) {
            int read = recv(client_fd, rxbuffer, SOCKET_MAX_LENGTH, 0);
            if (read == -1) {
                ESP_LOGE(TAG, "An error occured when receiving data (errno: %d)", errno);
            } else if (!read) {
                break;    // done reading
            }
            ESP_LOGI(TAG, "Received \"%s\"", rxbuffer);
            snprintf(txbuffer, SOCKET_MAX_LENGTH, "Transmission #%d. Hello from ESP32 TCP server", ++transmission_cnt);
            ESP_LOGI(TAG, "Transmitting: \"%s\"", txbuffer);
            int write = send(client_fd, txbuffer, read, 0);
            if (write == -1) {
                ESP_LOGE(TAG, "An error occured when sending data (errno: %d)", errno);
            }
        }
    }
}
