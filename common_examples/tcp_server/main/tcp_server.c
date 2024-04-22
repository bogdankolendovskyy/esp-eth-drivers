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

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}

static void start_dhcp_server_at_connection(void *esp_netif, esp_event_base_t base, int32_t event_id, void *data)
{
    esp_netif_dhcps_start(esp_netif);
}

void app_main(void)
{
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Initialize Ethernet driver
    uint8_t eth_port_cnt = 0;
    esp_eth_handle_t *eth_handles;
    ESP_ERROR_CHECK(ethernet_init_all(&eth_handles, &eth_port_cnt));
    esp_netif_init();
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));
#if CONFIG_EXAMPLE_ACT_AS_DHCP_SERVER
    // Act as DHCP server
    esp_netif_ip_info_t ip_info = {
        .ip = {.addr = ESP_IP4TOADDR(192, 168, 1, 1)},
        .netmask = {.addr =  ESP_IP4TOADDR(255, 255, 255, 0)},
        .gw = {.addr = ESP_IP4TOADDR(192, 168, 1, 1)}
    };
    const esp_netif_inherent_config_t eth_behav_cfg = {
        .get_ip_event = IP_EVENT_ETH_GOT_IP,
        .lost_ip_event = 0,
        .flags = ESP_NETIF_DHCP_SERVER,
        .ip_info = &ip_info,
        .if_key = "ETH_DHCPS",
        .if_desc = "eth",
        .route_prio = 50
    };
    esp_netif_config_t cfg = { .base = &eth_behav_cfg, .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH };

    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    esp_eth_handle_t eth_handle = eth_handles[0];
    esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handles[0]));

    esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_CONNECTED, start_dhcp_server_at_connection, eth_netif);
    esp_netif_dhcpc_stop(eth_netif);
    esp_netif_set_ip_info(eth_netif, &ip_info);
    esp_eth_start(eth_handle);
#else
    // Act as DHCP client, usual behaviour
    char if_key_str[10];
    char if_desc_str[10];
    // Create instance(s) of esp-netif for Ethernet(s)
    if (eth_port_cnt == 1) {
        // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
        // default esp-netif configuration parameters.
        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
        esp_netif_t *eth_netif = esp_netif_new(&cfg);
        // Attach Ethernet driver to TCP/IP stack
        ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handles[0])));
    } else {
        // Use ESP_NETIF_INHERENT_DEFAULT_ETH when multiple Ethernet interfaces are used and so you need to modify
        // esp-netif configuration parameters for each interface (name, priority, etc.).
        esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
        esp_netif_config_t cfg_spi = {
            .base = &esp_netif_config,
            .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
        };

        for (int i = 0; i < eth_port_cnt; i++) {
            sprintf(if_key_str, "ETH_%d", i);
            sprintf(if_desc_str, "eth%d", i);
            esp_netif_config.if_key = if_key_str;
            esp_netif_config.if_desc = if_desc_str;
            esp_netif_config.route_prio -= i * 5;
            esp_netif_t *eth_netif = esp_netif_new(&cfg_spi);

            // Attach Ethernet driver to TCP/IP stack
            ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handles[i])));
            esp_eth_start(eth_handles[i]);
        }
    }
#endif
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
            if (!read) {
                break;    // done reading
            }
            ESP_LOGI(TAG, "Received \"%s\"", rxbuffer);
            snprintf(txbuffer, SOCKET_MAX_LENGTH, "Transmission #%d. Hello from ESP32 TCP server", ++transmission_cnt);
            ESP_LOGI(TAG, "Transmitting: \"%s\"", txbuffer);
            send(client_fd, txbuffer, read, 0);
        }
    }
}
