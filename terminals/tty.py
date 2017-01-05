#!/usr/bin/env python
from socketIO_client import SocketIO, BaseNamespace, WebsocketTransport
import sys

import logging
import select
#logging.getLogger('socketIO-client').setLevel(logging.DEBUG)
logging.basicConfig()

def input_with_timeout():
    ready = select.select([sys.stdin],[],[], 0)[0]
    if ready:
        return sys.stdin.readline()

class Namespace(BaseNamespace):

    def on_output(self, data):
        sys.stdout.write(data)
        sys.stdout.flush()

def go(port=60001):
    socketIO = SocketIO('https://docker2016.holidayhackchallenge.com', port, Namespace, resource='wetty/socket.io',verify=False)
    while True:
        i = input_with_timeout()
        if i:
            socketIO.emit('input', i + '\r\n')
        socketIO.wait(.1)

if __name__ == "__main__":
    try:
        port = int(sys.argv[1])
    except:
        port = 60001
    go(port)
