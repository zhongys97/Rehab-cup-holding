#include <WiFi.h>
#include "AzureIotHub.h"
#include "Esp32MQTTClient.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "SR04.h"


#define INTERVAL 10  //time interval between two consecutive messages in milliseconds

#define DEVICE_ID "ESP32Device"   //device Id you wish to give To identify the device

#define MESSAGE_MAX_LEN 256    //256 bytes is the maximum data

#define BNO055_SAMPLERATE_DELAY_MS (100)
#define TRIG_PIN 19
#define ECHO_PIN 18

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
double filteredDist = 0;
double measuredDist = 0;

double orientationX, orientationY, orientationZ;
double t0orientationX, t0orientationY, t0orientationZ;

static bool t0Flag=true;

// Please input the SSID and password of WiFi
const char* ssid     = "NETGEAR59"; // Name of your Wi-Fi Network.
const char* password = "exoticbutter789"; // Enter your Wi-Fi Network Security Key / Password
//Replace the above placeholders with your WiFi network's credentials.

int messageCount = 1;    //initialize mesage to one
static const char* connectionString = "HostName=rehab-cup-holding-miblab.azure-devices.net;DeviceId=cup-device;SharedAccessKey=KyPiyfGdfSKRgk28DzasS5+JL0pJ9o7DTRbItZFT68g=";
//Replace the above placeholder with the primary connection string of your IoT device.

const char *messageData = "{\"deviceId\":\"%s\", \"messageId\":%d, \"orientationX\":%f, \"orientationY\":%f, \"orientationZ\":%f, \"distance\":%f}"; //message format or the format template

static bool hasWifi = false;

static bool messageSending = true;
static uint64_t send_interval_ms;

// Utilities

static void InitWifi()
{
  Serial.println("Connecting...");

  WiFi.begin(ssid, password);   //wifi begin method is used to connect to cloud
  
  while (WiFi.status() != WL_CONNECTED) {      //returns connected when successfully connected 
    delay(500);
    Serial.print(".");
  }
  
  hasWifi = true;
  
  Serial.println("WiFi connected");
  
  Serial.println("IP address: ");  //we also print Ip Address
  Serial.println(WiFi.localIP());
}

//Function definition

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    Serial.println("Send Confirmation Callback finished.");
  }
}


void displaySensorStatus(void)
{
  /* Get the system status values (mostly for debugging purposes) */
  uint8_t system_status, self_test_results, system_error;
  system_status = self_test_results = system_error = 0;
  bno.getSystemStatus(&system_status, &self_test_results, &system_error);

  /* Display the results in the Serial Monitor */
  Serial.println("");
  Serial.print("System Status: 0x");
  Serial.println(system_status, HEX);
  Serial.print("Self Test:     0x");
  Serial.println(self_test_results, HEX);
  Serial.print("System Error:  0x");
  Serial.println(system_error, HEX);
  Serial.println("");
  delay(500);
}

void displayCalStatus(void)
{
  /* Get the four calibration values (0..3) */
  /* Any sensor data reporting 0 should be ignored, */
  /* 3 means 'fully calibrated" */
  uint8_t system, gyro, accel, mag;
  system = gyro = accel = mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);

  /* The data should be ignored until the system calibration is > 0 */
  Serial.print("\t");
  if (!system)
  {
    Serial.print("! ");
  }

  /* Display the individual values */
  Serial.print("Sys:");
  Serial.print(system, DEC);
  Serial.print(" G:");
  Serial.print(gyro, DEC);
  Serial.print(" A:");
  Serial.print(accel, DEC);
  Serial.print(" M:");
  Serial.print(mag, DEC);
}

double simplifyAngle(double rawReadingAngle) {
  if (rawReadingAngle < -180.0){
    return rawReadingAngle + 360;
  }
  else if (rawReadingAngle > 180.0){
    return rawReadingAngle - 360;
  }
  else{
    return rawReadingAngle;
  }
}


// Arduino sketch
void setup()
{
  
  Serial.begin(115200);
  
  Serial.println("ESP32 Device");
  Serial.println("Initializing...");

  //Initialize the WiFi module
  Serial.println(" > WiFi");
  hasWifi = false;
  
  InitWifi();
  
  if (!hasWifi)
  {
    return;
  }
  
  
  Serial.println(" > IoT Hub");
    
  
  Esp32MQTTClient_Init((const uint8_t*)connectionString);

  //bool Esp32MQTTClient_Init(const uint8_t* deviceConnString, bool hasDeviceTwin = false, bool traceOn = false);
/**
*   Initialize a IoT Hub MQTT client for communication with an existing IoT hub.
*           The connection string is load from the EEPROM.
*    deviceConnString   Device connection string.
*     hasDeviceTwin   Enable / disable device twin, default is disable.
*     traceOn         Enable / disable IoT Hub trace, default is disable.
*
*    Return true if initialize successfully, or false if fails.
*/

  Esp32MQTTClient_SetSendConfirmationCallback(SendConfirmationCallback);

//void Esp32MQTTClient_SetSendConfirmationCallback(SEND_CONFIRMATION_CALLBACK send_confirmation_callback);
/**
*    Sets up send confirmation status callback to be invoked representing the status of sending message to IOT Hub.
*/

  send_interval_ms = millis();

  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  displaySensorStatus();
  bno.setExtCrystalUse(true);
  Serial.print("Start running");

  delay(3000);

  sensors_event_t event;
  bno.getEvent(&event);
  t0orientationX = simplifyAngle((double) event.orientation.x);
  t0orientationY = simplifyAngle((double) event.orientation.y);
  t0orientationZ = simplifyAngle((double) event.orientation.z);
  
}

void loop()
{
  if (hasWifi)
  {
    if (messageSending && 
        (int)(millis() - send_interval_ms) >= INTERVAL)
    {
      Serial.println(t0orientationX);
  Serial.println(t0orientationY);
  Serial.println(t0orientationZ);
      char messagePayload[MESSAGE_MAX_LEN];
      
      sensors_event_t event;
      bno.getEvent(&event);
      if (t0Flag == true){
        t0orientationX = simplifyAngle((double) event.orientation.x);
        t0orientationY = simplifyAngle((double) event.orientation.y);
        t0orientationZ = simplifyAngle((double) event.orientation.z);
        orientationX = 0;
        orientationY = 0;
        orientationZ = 0;
        t0Flag = false;
        
      } else {
        orientationX = simplifyAngle((double) event.orientation.x - t0orientationX);
        orientationY = simplifyAngle((double) event.orientation.y - t0orientationY);
        orientationZ = simplifyAngle((double) event.orientation.z - t0orientationZ);
      }
//      orientationX = (double) event.orientation.x - t0orientationX;
//      orientationY = (double) event.orientation.y - t0orientationY;
//      orientationZ = (double) event.orientation.z - t0orientationZ;

      measuredDist=(double)sr04.Distance();
      if (measuredDist < 20) {
        filteredDist = measuredDist;
      }
      
      snprintf(messagePayload,MESSAGE_MAX_LEN,messageData, DEVICE_ID, messageCount++, orientationX,orientationY, orientationZ, filteredDist);
      //snprintf(char *str, size_t size,  const char *format, ...)
      //snprintf(buffer,    maximum size, const char *format (i.e template), other arguments.....)
      
      Serial.println(messagePayload);
      
      EVENT_INSTANCE* message = Esp32MQTTClient_Event_Generate(messagePayload, MESSAGE);

                                //Esp32MQTTClient_Event_Generate(const char *eventString, EVENT_TYPE type);
/**
*    Generate an event with the event string specified by @p eventString.
*
*   eventString             The string of event.
*
*    EVENT_INSTANCE upon success or an error code upon failure.
*/
      
      Esp32MQTTClient_SendEventInstance(message);

//bool Esp32MQTTClient_SendEventInstance(EVENT_INSTANCE *event);
/**
*     Synchronous call to report the event specified by @p event.
*
*    event the event instance.
*
*    Return true if send successfully, or false if fails.
*/
      
      send_interval_ms = millis();
    }
  }
  delay(10);
}
