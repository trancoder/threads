import socket
import struct

UDP_IP = "192.168.1.62"  # IP address to listen on
UDP_PORT = 1236  # Port number to listen on

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the IP address and port
sock.bind((UDP_IP, UDP_PORT))

print("Waiting for UDP data...")

while True:
    # Receive data
    data, addr = sock.recvfrom(1024)  # Buffer size is 1024 bytes

    # Unpack the received data into a struct of three doubles
    received_data = struct.unpack('ddd', data)

    print("Received message:")
    print("x:", received_data[0])
    print("y:", received_data[1])
    print("z:", received_data[2])

# Close the socket (unreachable due to infinite loop)
sock.close()

