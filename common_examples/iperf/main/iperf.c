#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_console.h"
#include "ethernet_init.h"
#include "iperf_cmd.h"
#include "sdkconfig.h"

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
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_new_repl_uart(&uart_config, &repl_config, &repl);
    app_register_iperf_commands();
    esp_console_start_repl(repl);
}
