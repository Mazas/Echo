#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// define sensor pins
int triggerL = 8;
int triggerR = 10;
int echoL = 9;
int echoR = 11;

// declare the variables for time and distance 
// distance is distance from the sensor to the object 
long time;
int dist, distL, distR;

// define network settings
const char* server = "192.168.1.181"; // local ip address
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 125);
IPAddress myDns(192, 168, 0, 1); // dns server is local area router
EthernetClient ethClient;
PubSubClient mqttClient(ethClient); // MQTT client
char topic[] = "echo"; //MQTT topic publish to
char message[16];

void setup() {
  // set up pins for echo sensors
  Serial.begin(9600);
  pinMode(triggerL,OUTPUT);
  pinMode(triggerR,OUTPUT);
  pinMode(echoL,INPUT);
  pinMode(echoR,INPUT);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  mqttClient.setServer(server, 1883);
}

void reconnect() {
  // Loop until its connected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("connected");
      // Connected, send a message
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  // reconnect if connection is lost
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop(); // required for MQTT library
  
  // get the distance for both sensors
  getDistance(triggerL, echoL);
  distL = dist;
  getDistance(triggerR,echoR);
  distR = dist;


  // Both sensors
  if(distL<=30&&distR<=30){
    Serial.println("BothLocked");
    memset(message, 0, strlen(message));
    strcat(message, "BothLocked");
    sendMessage(message);
    delay(1000);    
  }

  //Left sensor: object is close
  if(distL<=15&&distR>30){
    Serial.println("LeftPushed");
    memset(message, 0, strlen(message));
    strcat(message, "LeftPushed");
    sendMessage(message);
    delay(1000);
  }
  //Left sensor: object is far
  if(distL>15&&distL<30&&distR>30){
    Serial.println("LeftPulled");
    memset(message, 0, strlen(message));
    strcpy(message, "LeftPulled");
    delay(1000);
  }

    //Right sensor: object is close
  if(distR<=15&&distL>30){
    Serial.println("RightPushed");
    memset(message, 0, strlen(message));
    strcpy(message, "RightPushed");
    delay(1000);
  }
  //Right sensor: object is far
  if(distR>15&&distR<30&&distL>30){
    Serial.println("RightPulled");
    memset(message, 0, strlen(message));
    strcpy(message, "RightPulled");
    delay(1000);
  }

  // delay 0.2 seconds
  delay(200);
}

void sendMessage(char* message){
  //Send the message
  int length = strlen(message);
  boolean retained = true;
  mqttClient.publish(topic,(byte*)message,length,retained);
}

void getDistance(int trigger, int echo){
  // turn off the signal to clear it.
  digitalWrite(trigger,LOW);
  delayMicroseconds(2);
  // send out a signal to detect an object
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  // turn off the signal
  digitalWrite(trigger, LOW);

  // listen for the echo
  time = pulseIn(echo, HIGH);
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  dist = time/29/2;
  if (dist>50){
    //if distance is more than 50 cm, its idle, set it to 50
    dist = 50;
  }
}
