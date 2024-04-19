# Iperf example

This exmaple provides a simple way to measure network performance using iperf.

## About the example
The example uses `iperf` and `iperf-cmd` components for an iperf implementation and providing command line interface for it. It provides DHCP server functionality for connecting to another ESP32, instead of a PC.

## Configuring the example
Using `idf.py menuconfig` setup the Ethernet configuration, in `Example option` you can enable DHCP server. 

**Important**: If DHCP server functionality is enabled only first PHY can be used. In DHCP client mode you can use any available.

## Running the example
You will see `esp>` prompt appear in ESP32 console. Run `iperf -h` to see iperf command options.
