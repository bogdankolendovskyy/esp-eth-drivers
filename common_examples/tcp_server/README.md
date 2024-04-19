# TCP server example

This example provides a basic implementation of tcp server for ESP32 with drivers from this repository.

## About the example
TCP server is designed to accept transmissions from client, print it to console and respond with another message "Transmission #XX. Hello from ESP32 TCP server".

## Configuring the example
Configure the example using `idf.py menuconfig`, according to your setup, provide settings for Ethernet initialization. 

## Running the example
To transmit data between your PC and ESP32 you need to wait until it obtains an IP address from DHCP server and run tcp client script.
Minimal command to do it is `tcp_client.py IP` and it will run until script is stopped. Additional parameters are:
* `-c COUNT` to set amount of transmission after which client stops transmitting, terminates connection and presents stats
* `-t TIME` to set periods between transmissions (default: 500ms)
* `-s` to run silently, without printing debug messages. In this mode count is set to 10 if not specified otherwise

You will see incoming messages in ESP32's console, and server's answers as output of `tcp_client.py`