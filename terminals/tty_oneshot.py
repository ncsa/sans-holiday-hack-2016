#!/usr/bin/env python
from socketIO_client import SocketIO, BaseNamespace, WebsocketTransport
import sys

import logging
import time
#logging.getLogger('socketIO-client').setLevel(logging.DEBUG)
logging.basicConfig()

MARKER = 'XXXX EXIT'
MARKER_CMD = "echo XXXX   EXIT"
DIVIDER='X-X-X-X-X-X-X-X-X-X-X-X-X'

class Namespace(BaseNamespace):
    buf = ''

    def on_output(self, data):
        self.buf += data
        if MARKER in data:
            print self.buf.split("\n" + DIVIDER)[1]
            sys.exit(0)

def go(port=60001):
    socketIO = SocketIO('https://docker2016.holidayhackchallenge.com', port, Namespace, resource='wetty/socket.io',verify=False)
    cmd = ' '.join(sys.argv[2:])

    def send(s):
        socketIO.emit('input', s + '\r\n')

    send("echo {};{};echo {}".format(DIVIDER, cmd, DIVIDER))
    send(MARKER_CMD)
    while True:
        socketIO.wait(.1)

if __name__ == "__main__":
    try:
        port = int(sys.argv[1])
    except:
        port = 60001
    go(port)
