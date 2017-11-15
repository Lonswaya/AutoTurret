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
        type_num = input('type: ')
        type_num = int(type_num)
        data = input('data: ')
        data = int(data)
        packet = struct.pack('ii', type_num, data)
        sock.send(packet)
