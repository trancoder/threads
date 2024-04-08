import socket
import struct

UDP_IP = "192.168.1.62"  # IP address of the target machine
UDP_PORT = 1236  # Port number that you want to send data to

# Create a UDP socket
sock = socket.socket(socket.AF_INET,  # Internet
                     socket.SOCK_DGRAM)  # UDP

# Define the struct format (3 doubles)
struct_format = 'ddd'
data = struct.pack(struct_format, 1.23, 4.56, 7.89)  # Example data

# Send the packed data
sock.sendto(data, (UDP_IP, UDP_PORT))

print("Message sent:")
