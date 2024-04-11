#!/usr/bin/env python3
import socket
import argparse
import logging
import signal

parser = argparse.ArgumentParser(description='Serve TCP connection using berkley sockets and wait for connections', epilog='Part of the tcp_client example for esp_eth_drivers')
parser.add_argument('ip')
parser.add_argument('-p', '--port', type=int, default=5000, choices=range(1, 65535), metavar="PORT", help='Port to listen on')
parser.add_argument('-s', '--silent', action='store_true', help="Do not log incoming transmissions")
args = parser.parse_args()

# setup sigint handler
signal.signal(signal.SIGINT, lambda s, f : exit(0))

logger = logging.getLogger("tcp_server")
logging.basicConfig(format="%(name)s :: %(levelname)-8s :: %(message)s", level=logging.DEBUG)
if args.silent:
    logger.setLevel(logging.INFO)
logger.info("Listening on %s:%d", args.ip, args.port)

# init server
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((args.ip, args.port))
# listen for incoming connections
sock.listen(1)

counter = 1
while True:
    conn, address = sock.accept()
    logger.debug("Accepted connection from %s:%d", address[0], address[1])
    while True:
        try:
            data = conn.recv(128).decode()
        except ConnectionAbortedError:
            logger.info("Connection closed by client")
            break
        logger.debug("Received: \"%s\"", data)
        msg = f"Transmission {counter}: Hello from Python"
        logger.debug("Transmitting: \"%s\"", msg)
        conn.sendall(str.encode(msg))
        counter += 1
