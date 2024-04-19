# TCP client example

This example provides a basic implementation of tcp client for ESP32 with drivers from this repository

## About the example
TCP client is designed to periodically transmit to the server, and accept responses, which are printed to the console.

## Configuring the example
Configure the example using `idf.py menuconfig`, according to your setup, provide settings for Ethernet initialization. Next go to `Example options` and input the IP address of your PC in the local networkto wich ESP32 is connected. 

## Running the example

Ater you obtain an IP address you will see message reading `TCP client is started, waiting for the server to accept a connection.`. 

Start the server with `tcp_server.py`. Command to do so is `tcp_server.py IP` to start listening on specified IP address.

**Important**: use the IP address of your PC in the local network to whic ESP32 is connected. It **must** match the IP address you've set in `Example options`.


You will see incoming messages as output of `tcp_server.py`.