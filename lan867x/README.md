# Microchip LAN867x Ethernet PHY Driver

## Overview

The LAN8670/1/2 is a high-performance 10BASE-T1S single-pair Ethernet PHY transceiver for 10 Mbit/s half-duplex networking over a single pair of conductors. 
Utilizing standard Ethernet technology in sensor/actuator networks reduces application costs by eliminating gateways necessary with legacy networking technologies.
More information about the chip can found in the product [datasheets](https://ww1.microchip.com/downloads/aemDocuments/documents/AIS/ProductDocuments/DataSheets/LAN8670-1-2-Data-Sheet-60001573.pdf).

## ESP-IDF Usage

Just add include of `esp_eth_phy_lan867x.h` to your project,

```c
#include "esp_eth_phy_lan867x.h"
```

create a `phy` driver instance by calling `esp_eth_phy_new_lan867x()`

```c
eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

// Update PHY config based on board specific configuration
phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;
esp_eth_phy_t *phy = esp_eth_phy_new_lan867x(&phy_config);
```

and use the Ethernet driver as you are used to. For more information of how to use ESP-IDF Ethernet driver, visit [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_eth.html).
