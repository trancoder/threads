#######
# in case there is no websockets installed:
# sudo pip3 install websockets
######

import json
import asyncio
import websockets
import time

import configparser
import os
import sys
import subprocess
import logging
from datetime import datetime, timezone

# Define the path to the JSON file
JSON_FILE = "sample.json"

afatds_ip = "192.168.1.220"
afatds_port = "1234"
afatds_path = "/launch-event-service/ws"

# Define the WebSocket server handler
async def server(websocket, path):       
    if path == afatds_path:
        print("Client connected")
        try:
            # Wait for the user on the server to hit Enter
            input("Press Enter to launch the missile")

            with open(JSON_FILE, "r") as file:
                json_data = json.load(file)

            current_time = time.clock_gettime(time.CLOCK_REALTIME)
            print("current_time = ", current_time)

            current_time_in_MilliSec       = int(current_time * 1000)
            print("current_time_in_MilliSec = ", current_time_in_MilliSec)
            
            json_data["launchTime"] = current_time_in_MilliSec            

            # convert to json string
            json_string = json.dumps(json_data)
            
            await websocket.send(json_string)

            print("JSON data sent to the client")

        except websockets.exceptions.ConnectionClosedError:
            print("Client disconnected")
    else:
        print("Client connect to wrong path")

# Start the WebSocket server
start_server = websockets.serve(server, afatds_ip, int(afatds_port))

# Run the event loop
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()