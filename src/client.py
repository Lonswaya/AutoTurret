import sys
import socket
import struct

if __name__ == '__main__':
    if(len(sys.argv) < 3):
        sys.exit('Wrong args')
    
    print('Connecting to ', sys.argv[1], ': ', sys.argv[2]);
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((sys.argv[1], int(sys.argv[2])))

    while True:
        num = input("> ")
        num = int(num)
        packet = struct.pack('ii', num, num)
        sock.send(packet)
