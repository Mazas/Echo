import time
import paho.mqtt.client as mqtt

broker_address = "192.168.1.181"


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
# TODO edit to actually process the message
def on_message(client, userdata, message):
    received = str(message.payload.decode("utf-8"))
    print("Message received: ", received)


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
