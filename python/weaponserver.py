import socket
import struct
import time

# Define host and port
HOST = '192.168.1.220'  # Loopback address for localhost
PORT = 1235  # Choose any available port

# Create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the host and port
server_socket.bind((HOST, PORT))

# Listen for incoming connections (max queue of 5)
server_socket.listen()
print(f"Server listening on {HOST}:{PORT}")

# Accept incoming connections
client_socket, client_address = server_socket.accept()
print(f"Connection from {client_address}")

# Wait for the user on the server to hit Enter
input("Press Enter to launch the missile")

# Timestamp the launch event to millisecond precision in UTC timezone
# and send to the connected fireControl client as an encoded string.
timestamp = time.clock_gettime(time.CLOCK_REALTIME)
timeSec       = int(timestamp)
timeNano      = int((timestamp % timeSec) * (10 ** 9))
print(f'Timestamp: {timestamp}\n'
            f'\tSeconds       : {timeSec}\n'
            f'\tNanoseconds   : {timeNano}'
)

client_socket.send(struct.pack('!LL', timeSec, timeNano))

# Close the connection
client_socket.close()
server_socket.close()
