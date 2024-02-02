import os
import socket
from datetime import datetime
from time import sleep

TIMESTAMP_FORMAT = r'%Y-%m-%d %H:%M:%S'
PERIOD = 30


sock = socket.socket()
sock.connect((os.environ['SERVER_IP'], 80))

with open('sensors.log', 'a') as file:
    while True:
        sock.send(b' ')
        data = sock.recv(64).decode()
        file.write(
            '{} {}'.format(
                datetime.now().strftime(TIMESTAMP_FORMAT),
                data,
            )
        )
        sleep(PERIOD)
