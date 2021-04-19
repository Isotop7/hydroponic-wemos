#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
Define statements for OLED screen
*/
#define OLED_I2C_BUS_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_RESET -1

/*
Define statements for hydroponic unit
*/
#define PIPE_COUNT 2
#define PIPE_RELAIS_UP_TOPIC "hydro/pipe/X/up"
#define PIPE_RELAIS_DOWN_TOPIC "hydro/pipe/X/down"
#define PIPE_TEMP_TOPIC "hydro/pipe/X/temp"
#define TEMP_PIN D6
#define TEMP_READ_DELAY 60000

/*
Define statements for network
Credentials are embedded in the wifiSecrets.h header file
*/
#include <wifiSecrets.h>
#define MQTT_RECONNECT_DELAY 5000

/*
Create needed sensor modules
*/
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
OneWire tempBus (TEMP_PIN);
DallasTemperature tempSensors(&tempBus);
DeviceAddress _tempDeviceAddress;

/*
Create needed network objects
*/
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/*
Global hepler variables
*/
int tempSensorCount = 0;
long lastTempRead = 0;

/*
Writes the deviceAdress to Serial
*/
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

/*
Reads temperatures from temperature bus and pushes them to mqtt broker
var int isbc = index of SBC and therefore the corresponding temperature sensor
*/
void readTemperature(int ipipe) {
  //Check if any temperature sensors are connected
  if (tempSensorCount == 0) {
    Serial.println("+ TEMP | No temperature sensors connected.");
  } else {
    // Send the command to the bus to get temperatures
    tempSensors.requestTemperatures();
    
    // Search the wire for address on given index
    if(tempSensors.getAddress(_tempDeviceAddress, ipipe)){
      //Char arrays for topic and SBC
      char topic[] = PIPE_TEMP_TOPIC;
      char pipe[1];
      //Convert int value of SBC to Char
      sprintf(pipe, "%d", ipipe+1);
      //Replace X value in topic with SBC
      topic[12] = pipe[0];
      //Get read temperature from sensor
      float tempC = tempSensors.getTempC(_tempDeviceAddress);
      //Output to Serial
      Serial.print("+ MQTT->PUB | ");
      Serial.print(topic);
      Serial.print(" | ");
      Serial.println(tempC);
      //Push temperature to mqtt broker
      mqttClient.publish(topic, String(tempC).c_str(), true);
    }
  }
}

/*
Called method when wemos is not connected to mqtt broker
*/
void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("+ MQTT | Attempting connection ...");
    //Try connection to broker and check for returned value; true means connected
    if (mqttClient.connect("hydroponic-wemos", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("+ MQTT | Connected to broker");
    } else {
      //Output error to Serial
      Serial.print("+ MQTT | Connection attempt failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" ; try again in 5 seconds");
      //Wait for given timeout
      delay(MQTT_RECONNECT_DELAY);
    }
  }
}

/*
Arduino setup routine
*/
void setup() {
  //  +++ SERIAL CONSOLE +++
  //Setup serial monitor
  Serial.begin(9600);
  Serial.println("+ SERIAL | Setup complete");

  //  +++ NETWORK +++
  //Connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("+ WIFI | Connecting ");

  //check if connection to wifi was established
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" --> connected!");
  Serial.print("+ WIFI | Got IP: ");  
  Serial.println(WiFi.localIP());

  //Set mqttClient settings
  mqttClient.setServer(MQTT_SERVER, 1883);

  //  +++ TEMPERATURE SENSORS +++
  //Setup temperature sensors
  tempSensors.begin();
  Serial.println("+ TEMP | Counting devices on bus...");
  //Counting sensors on bus
  tempSensorCount = tempSensors.getDeviceCount();
  Serial.print("+ TEMP | Found ");
  Serial.print(tempSensorCount, DEC);
  Serial.println(" temperature sensors on the bus");

  // Loop through each device, print out address
  for(int i=0;i<tempSensorCount; i++){
    // Search the wire for address
    if(tempSensors.getAddress(_tempDeviceAddress, i)){
      Serial.print("+ TEMP | Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(_tempDeviceAddress);
      Serial.println();
    }
  }

  //TODO: Needed for OLED, check as this is likely going to interfere with the temp sensors
  //  +++ Wire bus +++
  //Setup wire
  Wire.begin();
  Serial.print("+ I2C BUS | Wire setup complete");

  //  +++ OLED screen +++
  //Setup OLED and feed with internal source
  if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_BUS_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  // Set default values for OLED screen and show splash screen
  oled.dim(true);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.println("hydro bootup done"); //TODO: Show splash pixel art instead of text
  oled.display();
  delay(2000);
}

/*
Arduino loop routine
*/
void loop() {
  //TODO: Control relais array
  //TODO: Read two temp sensors
  //TODO: Update screen, use pixel art for pump and temperature
  //TODO: Read dip switch

  //Check if wemos is not connected to mqtt broker
  if (!mqttClient.connected()) {
    //Reconnect to mqtt broker
    reconnect();
  }
  //mqtt loop; needed to check for incoming messages
  mqttClient.loop();

  //Set timestamp 'now'
  long now = millis();
  //Check if delay for last temperature read is reached
  if ((now - lastTempRead) > TEMP_READ_DELAY) {
    //Loop through all connected temperature sensors and read temperature
    for (int i = 0; i < tempSensorCount; i++) {
      readTemperature(i);
    }
    //Set timestamp of last execution to now
    lastTempRead = now;
  }
}