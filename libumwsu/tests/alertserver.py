#!/usr/bin/env /usr/bin/python3
import socket

class AlertServer(object):

    def __init__(self, socket_path):
        self.sock = socket.socket(socket.AF_UNIX)
        self.sock.bind(socket_path)
        self.sock.listen(5)

    def run(self):
        print("listening on " + str(self.sock))
        while True:
            (client_sock, client_addr) = self.sock.accept()
            print("Connection from " + str(client_addr))
            f = client_sock.makefile()
            xml = f.read()
            print(xml)
            f.close()
            client_sock.close()

server = AlertServer("/var/tmp/davfi_alert.s")
server.run()

                  
