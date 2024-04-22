# TCP client example

This example provides a basic implementation of TCP client for ESP32 with drivers from this repository

## About the example
TCP client is designed to periodically transmit to the server, and accept responses, which are printed to the console.

## Configuring the example
Configure the example using `idf.py menuconfig`, according to your setup, provide settings for Ethernet initialization. Next, go to `Example options` and input the IP address of your PC in the local network to which ESP32 is connected. If you want to connect this to another ESP32 device running `tcp_server` example, make sure to the the `Server IP address` to 196.168.1.1

## Running the example

After you obtain an IP address you will see a message reading `TCP client is started, waiting for the server to accept a connection.`. 

If you are connecting the device to a PC - start the server with `tcp_server.py`. The command to do so is `tcp_server.py IP` to start listening on the specified IP address.

**Important**: use the IP address of your PC in the local network to which ESP32 is connected. It **must** match the IP address you've set in the `Example options`.


You will see incoming messages in ESP32 console or as an as output of `tcp_server.py` if you have ran it.