#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_console.h"
#include "ethernet_init.h"
#include "iperf_cmd.h"
#include "sdkconfig.h"

static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    printf("Ethernet Got IP Address\n");
    printf("~~~~~~~~~~~\n");
    printf("ETHIP:" IPSTR "\n", IP2STR(&ip_info->ip));
    printf("ETHMASK:" IPSTR "\n", IP2STR(&ip_info->netmask));
    printf("ETHGW:" IPSTR "\n", IP2STR(&ip_info->gw));
    printf("~~~~~~~~~~~\n");
}

static void my_event_connected_handler(void *esp_netif, esp_event_base_t base, int32_t event_id, void *data)
{
    esp_netif_dhcps_start(esp_netif);
}


void app_main(void)
{
    uint8_t eth_port_cnt = 0;
    esp_eth_handle_t *eth_handles;
    esp_netif_init();
    esp_event_loop_create_default();
    ethernet_init_all(&eth_handles, &eth_port_cnt);

    //esp_netif_config_t cfg;
#if CONFIG_EXAMPLE_ACT_AS_DHCP_SERVER
    // Act as DHCP server
    esp_netif_ip_info_t ip_info = {
        .ip = {.addr = ESP_IP4TOADDR(192, 168, 1, 1)},
        .netmask = {.addr =  ESP_IP4TOADDR(255, 255, 255, 0)},
        .gw = {.addr = ESP_IP4TOADDR(192, 168, 1, 255)}
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
#else
    // Act as DHCP client, usual behaviour
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
#endif
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    esp_eth_handle_t eth_handle = eth_handles[0];
    esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle));

    esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, got_ip_event_handler, NULL);

#if CONFIG_EXAMPLE_ACT_AS_DHCP_SERVER
    esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_CONNECTED, my_event_connected_handler, eth_netif);
    esp_netif_dhcpc_stop(eth_netif);
    esp_netif_set_ip_info(eth_netif, &ip_info);
#endif

    esp_eth_start(eth_handle);
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_new_repl_uart(&uart_config, &repl_config, &repl);
    app_register_iperf_commands();
    esp_console_start_repl(repl);
}
