#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

const int PIN_SWITCH = D1;
const int PIN_LED_WIFI = D4;
const int PIN_LED_COUNT = D5;

int buttonPressedCount =0;
int buttonState =0;
int prevButtonState = 0;

int uploadMinute = 1;
unsigned long uploadTime;

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_SWITCH,INPUT_PULLUP);
  pinMode(PIN_LED_WIFI,OUTPUT);
  pinMode(PIN_LED_COUNT,OUTPUT);
  allLEDOff();

  delay(1000);
  Serial.println("Starting up ...");

  setupWifi();

  uploadTime = millis();
}

void loop() {

  handleButton();
  handleUpload();

  delay(50);
}

void setupWifi(){
  digitalWrite(PIN_LED_WIFI,HIGH);
  Serial.println("Starting WiFi ...");  
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Stone Stamcor","194201514007");
  
  delay(2000);
  digitalWrite(PIN_LED_WIFI,LOW);
}

void handleUpload(){
  unsigned long currentTime = millis();
  if (currentTime >= uploadTime +( uploadMinute *60000) ){
  //if (currentTime >= uploadTime +( uploadMinute * 10000) ){
    if (buttonPressedCount > 0){
      upload();      
    } else {
      Serial.println("No itemcount to upload .. skipping");
    }

    uploadTime = millis();   
  }
}

void upload(){
    // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    digitalWrite(PIN_LED_WIFI,LOW);
    Serial.println("Uploading ... ");

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://10.10.20.35:8080/itemCounter/v1/")) {
      http.addHeader("Content-Type", "application/json");

      // start connection and send HTTP header and body
      String postString = "{ \"machineId\":1, \"amount\":";
      postString += buttonPressedCount;
      postString += " }";
      Serial.println("[HTTP] POST..." + postString);

      digitalWrite(PIN_LED_WIFI,HIGH);
      int httpCode = http.POST(postString);
      digitalWrite(PIN_LED_WIFI,LOW);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK) {
          Serial.println("Clearing buttonpressed");
          buttonPressedCount = 0;          
          //const String& payload = http.getString();    
        }
      } else {        
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        flashWifiLED();
        flashWifiLED();
        flashWifiLED();        
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
      flashWifiLED();
      flashWifiLED();
      flashWifiLED(); 
    }
  } else {
    Serial.println("WIFI Issue?");
    flashWifiLED();
    flashWifiLED();
    flashWifiLED(); 
  } 

}

void handleButton(){
  buttonState = digitalRead(PIN_SWITCH);

  if (buttonState != prevButtonState){
    
    // check if pulled to low
    if (buttonState == LOW ){
      buttonPressedCount++;
      Serial.println(buttonPressedCount);

      flashCountLED();
    }
  }
  prevButtonState=buttonState;
}

void flashCountLED(){
  flashLED(PIN_LED_COUNT,100);
}

void flashWifiLED(){
  flashLED(PIN_LED_WIFI,100);
}

void flashLED(int ledPin,int delayMilli){
  digitalWrite(ledPin,HIGH);
  delay(delayMilli);
  digitalWrite(ledPin,LOW);  
  delay(delayMilli);
}

void allLEDOff(){
  digitalWrite(PIN_LED_COUNT,LOW);
  digitalWrite(PIN_LED_WIFI,LOW);
}
