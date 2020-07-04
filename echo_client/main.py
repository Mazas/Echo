import time
import os
import paho.mqtt.client as mqtt

broker_address = "192.168.1.181"


# Define the commands for easier editing
# opens terminal
left_pulled_command = "gnome-terminal"
# opens gedit
left_pushed_command = "gedit & disown"
# opens opera browser
right_pulled_command = "opera & disown"
# displays date and time
right_pushed_command = "date"
# writes "hello"
both_locked_command = "echo hello"


# Switcher method to identify the message
def interpret_message(message):
    switcher = {
        "LeftPushed": left_pushed_command,
        "LeftPulled": left_pulled_command,
        "RightPushed": right_pushed_command,
        "RightPulled": right_pulled_command,
        "BothLocked": both_locked_command
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
    os.system(interpret_message(received))
    print("Message received: ", received)


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
