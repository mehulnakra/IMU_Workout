#include "rpcWiFi.h"
#include <ArduinoMqttClient.h>
#include"TFT_eSPI.h"
#include"Free_Fonts.h" //include the header file
TFT_eSPI tft;
#include <rpcBLEDevice.h>
#include <BLEServer.h>

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214" // Battery Service UUID
#define CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214" // Battery Level UUID

WiFiClient wifiClient;

MqttClient mqttClient(wifiClient);

const char* ssid = "OnePlus 7T";
const char* password =  "12345678.";

const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "real_unique_topic";
String Name="";


int weight; 
int count =0;
int invalid =0;
bool ProfileRec = false;
float CaloriesBurned = 0.0;
static BLECharacteristic *pCharacteristic;

class MyCallbacks: public BLECharacteristicCallbacks {

    String CountStr = String(count);
    
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        //Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++){
          uint8_t Data = rxValue[i];
          //Serial.print(Data);
          if(Data){count = count + 1;
            String CountStr = String(count);
            CaloriesBurned = count*0.08*weight;
            String CBStr = String(CaloriesBurned);
            String InvalidStr = String(invalid);
            //Serial.print("Count: ");
            //Serial.print(count);
            tft.fillScreen(TFT_GREEN);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString("Reps Done :",50,60);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString(CountStr,110,80);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString("Calories Burned :",50,100);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString(CBStr,110,120);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString("Invalid Counts :",110,140);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString(InvalidStr,110,160);
          }
          else{ invalid = invalid +1;

            String CountStr = String(count);
            CaloriesBurned = count*0.08*weight;
            String CBStr = String(CaloriesBurned);
            String InvalidStr = String(invalid);
            //Serial.print("Count: ");
            //Serial.print(count);
            tft.fillScreen(TFT_RED);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString("Reps Done :",50,60);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString(CountStr,110,80);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString("Calories Burned :",50,100);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString(CBStr,110,120);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString("Invalid Counts :",110,140);
            tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
            tft.drawString(InvalidStr,110,160);
          }
          
        }
        //Serial.println();
        //Serial.println("*********");
      }
    }
};



void setup() {

    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_GREEN); //Black background

    //Serial.begin(115200);
    //while(!Serial); // Wait for Serial to be ready

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    //Serial.println("Connecting to WiFi..");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        //Serial.println("Connecting to WiFi..");
        WiFi.begin(ssid, password);
    }
    //Serial.println("Connected to the WiFi network");
    //Serial.print("IP Address: ");
    //Serial.println (WiFi.localIP()); // prints out the device's IP address

    if (!mqttClient.connect(broker, port)) {
    //Serial.print("MQTT connection failed! Error code = ");
    //Serial.println(mqttClient.connectError());
    while (1);
    }

    //Serial.println("You're connected to the MQTT broker!");
    //Serial.println();

    //Serial.println("You're connected to the MQTT broker!");
  //Serial.println();

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);

  //Serial.print("Subscribing to topic: ");
 //Serial.println(topic);
  //Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(topic);


  // ####################### CODE FOR BLE ###############################


  BLEDevice::init("Long name 11");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setAccessPermissions(GATT_PERM_READ | GATT_PERM_WRITE); // Restrict to write-only permissions
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  //pAdvertising->setMinPreferred(0x06);  // helps with iPhone connections
  BLEDevice::startAdvertising();
  //Serial.println("Now you can write to the characteristic from your phone!");



  // ############# CODE FOR BLE END ##################################

  
    }


void loop() {
  if(!ProfileRec){
  mqttClient.poll();
  }
}

void onMqttMessage(int messageSize) {
  String receivedString; 
  // char  mqtt_data_char = mqttClient.read();
  // String mqtt_data_str = String(mqtt_data_char);
  // Serial.println((char)mqttClient.read());
  // Assuming mqttClient.read() returns a String

  // Check if messageSize is greater than 1
  if (messageSize > 2) {
    ProfileRec = true;
    String welcome_message = "Welcome to IISC Gymkhana ";
    
  while (mqttClient.available()) {
    receivedString = receivedString + (char)mqttClient.read();
  }
  // Find the position of the opening and closing parentheses
  int openParenIndex = receivedString.indexOf("(");
  int closeParenIndex = receivedString.indexOf(")");

  Name = receivedString.substring(0, openParenIndex);
  // Extract the weight
  String weightString = receivedString.substring(openParenIndex + 1, closeParenIndex);
  weight = weightString.toInt(); // Convert weight string to integer

  }

  // tft.setFreeFont(&FreeSansBoldOblique12pt7b); //select Free, Sans, Bold, Oblique, 12pt.
  // tft.drawString(welcome_message,70,80);//prints string at (70,80)

  // tft.setFreeFont(FF10);//select Free, Sans, Bold, Oblique, 12pt.
  // tft.drawString(mqtt_data_str,70,80);//prints string at (70,80)

  tft.setFreeFont(FS12); //select Free, Sans, Bold, Oblique, 12pt.
  tft.drawString("Welcome to IISC Gymkhana ",25,80);//prints string at (70,80)
  tft.setFreeFont(FS12); 
  //tft.drawString(mqtt_data_str,50,110);
  tft.drawString(Name,80,110);

  }





