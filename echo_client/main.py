import time
import os
import paho.mqtt.client as mqtt

broker_address = "192.168.1.181"


# Define the commands for easier editing
left_pulled_command = "mailspring"
left_pushed_command = ""
right_pulled_command = ""
right_pushed_command = ""
both_locked_command = ""


# Helper functions to implement each command separately
def left_pushed():
    os.system(left_pushed_command)
    return "LeftPushed"


def left_pulled():
    os.system(left_pulled_command)
    return "LeftPulled"


def right_pushed():
    os.system(right_pushed_command)
    return "RightPushed"


def right_pulled():
    os.system(right_pulled_command)
    return "RightPulled"


def both_locked():
    os.system(both_locked_command)
    return "BothLocked"


# Switcher method to identify the message
def interpret_message(message):
    switcher = {
        "LeftPushed": left_pushed(),
        "LeftPulled": left_pulled(),
        "RightPushed": right_pushed(),
        "RightPulled": right_pulled(),
        "BothLocked": both_locked()
    }
    return switcher.get(message, "Invalid message")


# connection callback
# 0: Connection successful
# 1: Connection refused – incorrect protocol version
# 2: Connection refused – invalid client identifier
# 3: Connection refused – server unavailable
# 4: Connection refused – bad username or password
# 5: Connection refused – not authorised
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("\r", end="")
        print("Connected")
        client.subscribe("echo")
    else:
        print("\r", end="")
        print("Error code: ", rc)


# Callback to process the message
def on_message(client, userdata, message):
    received = str(message.payload.decode("utf-8"))
    print("Message received: ", interpret_message(received))


# main function
def main():
    print("Connecting...", end="")
    # Create client
    client = mqtt.Client("echo_client")
    # Attach callback methods
    client.on_message = on_message
    client.on_connect = on_connect
    # Connect to the server
    client.connect(broker_address)
    # Start the loop
    client.loop_start()

    # Loop indefinitely and listen for messages
    while True:
        client.loop_read()
        time.sleep(1)


if __name__ == '__main__':
    main()
