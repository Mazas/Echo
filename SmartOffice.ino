#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// pins used
int noiceLed = 2; // red
int lightLed = 3; // green
int temperatureLed = 4; // yellow

int lightSensor = A0;
int noiceSensor = 5;
int tempSensor = A2;

int analogValue = 0;
int Vo;
int noiceDetected = HIGH;

// Internet connection
// server is MQTT server
const char* server = "192.168.1.181";
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
// ip is desired IP address
IPAddress ip(192, 168, 1, 125);
// DNS server is the router
IPAddress myDns(192, 168, 0, 1);
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
// MQTT message topic
char* topic = "lab";





void setup() {
  Serial.begin(9600);

  //setup pins
  pinMode(noiceLed, OUTPUT);
  pinMode(lightLed,OUTPUT);
  pinMode(temperatureLed,OUTPUT);
  pinMode (noiceSensor, INPUT);
  
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
  // Loop until reconnected
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
if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop(); // required to keep the connection going

  // check noice
  noiceDetected = digitalRead(noiceSensor);
  // noice string
  char *noiceString;
  Serial.println(noiceDetected);
  // light up the LED if noise is detected
  if(noiceDetected == HIGH){
    noiceString = " noice detected: HIGH;";
    digitalWrite(noiceLed, HIGH);
  }else{
    noiceString = " noice detected:  LOW;";
    digitalWrite(noiceLed,LOW);
  }
  


  // check light
  analogValue = analogRead(lightSensor);
  // light value string
  char lightValueString [10];
  char lightString [40]; 
  dtostrf(analogValue, 8, 6, lightValueString);
  // clear buffer and build the string
  memset(lightString, 0, strlen(lightString));
  strcat(lightString, " Light sensor value: ");
  strcat(lightString, lightValueString);

  if(analogValue < 50){     // dark, turn on the led       
    digitalWrite(lightLed, HIGH);
  }
  else{   // brigt, turn off the led
    digitalWrite(lightLed, LOW);
  }

  // Build the message to print and send
  char tempString [64];
  char message [256];
  checkTemperature(tempString);
  memset(message, 0, strlen(message));
  strcat(message, tempString);
  strcat(message, noiceString);
  strcat(message, lightString);
  // Print the message
  Serial.print(message);
  //Send the message
  int length = strlen(message);
  boolean retained = true;
  mqttClient.publish(topic,(byte*)message,length,retained);
  delay(1000); // wait one second
}

void checkTemperature(char *tempString){ // pointed to the message buffer
  // https://www.bc-robotics.com/tutorials/using-a-tmp36-temperature-sensor-with-arduino/
  Vo = analogRead(tempSensor);
  
  double temp = (double)Vo / 1024;   //find percentage of input reading
  temp = temp * 5;                     //multiply by 5V to get voltage
  temp = temp - 0.5;                   //Subtract the offset 
  temp = temp * 100;                   //Convert to degrees 

  double Tf = (temp * 9.0)/ 5.0 + 32.0; // convert to farenheit

  // build the string for the message and serial monitor
  char farenheit [10];
  char celcius [10];
  dtostrf(Tf, 8,6, farenheit);
  dtostrf(temp, 8,6, celcius);
  memset(tempString, 0, strlen(tempString));
  strcat(tempString, "Temperature: "); 
  strcat(tempString, farenheit);
  strcat(tempString, " F; ");
  strcat(tempString, celcius);
  strcat(tempString, " C;");   

  // if temperature is high, turn on the LED
  if (temp >30){
    digitalWrite(temperatureLed, HIGH);
  }else {
    digitalWrite(temperatureLed, LOW);
  }
  // return the message
  return tempString;
}
