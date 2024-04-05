#!/usr/bin/env python3
import socket
import argparse
import logging
import signal
import fcntl
import struct

def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,  # SIOCGIFADDR
        struct.pack('256s', ifname[:15])
    )[20:24])

parser = argparse.ArgumentParser(description='Run TCP connection using berkley sockets and wait for connections', epilog='Part of the tcp_client example for esp_eth_drivers')
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
sock.listen(0)
logger.info(f"Listening on {args.ip}:{args.port}")
