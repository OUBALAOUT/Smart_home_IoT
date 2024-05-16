#include <ESP32Servo.h>
#include "DHTesp.h"
#include <WiFi.h>
#include "ThingSpeak.h"

//thingspeak details
const char* WIFI_NAME = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int myChannelNumber = 2548694;
const char* write_API_Key = "BQB0UTHZT0NBOKFT";
const char* server = "api.thingspeak.com";
WiFiClient client;

DHTesp dhtCaptur;
Servo servo;

//Variables pour enregistrer les INPUTs
int temp;
int humidity;
int detectCO2;
int brightness;

//Pins
int bleu = 33;
int green = 26;
int red = 14;
int dhtPin = 12;
int ldrPin = 39;
int co2Pin = 36;
int ledONE = 19;
int ledTWO = 18;
int ledTHREE = 5;
int ledFOUR = 17;

//seuil de température et CO2 et luminosité
int seuilChauf = 15;
int seuilClima = 34;
int maxCO2_ = 1500;
float veryBright = 30200.00;
float bright = 10000.00;
float light = 400.00;
float dark = 10.00;

//manipuler les couleurs RGB
void colorRGB(int re_d , int gree_n , int ble_u)
{
  analogWrite(red, re_d);
  analogWrite(green, gree_n);
  analogWrite(bleu, ble_u);
}

//Convertir AnalogRead() vers Lux
float AnalogToLux(int ldr)
{
   float voltage = ldr / 4096.0 * 5.0;
  float resistance = 2000.0 * voltage / (1.0 - voltage / 5.0);
  return pow(50.0 * 1000.0 * pow(10.0, 0.7) / resistance,1.0/0.7);
}


//contrôler segment LED Bar Graph
void Eclairage(int l)
{
  switch(l){
    case 1:
    digitalWrite(ledONE, HIGH);
    digitalWrite(ledTWO, LOW);
    digitalWrite(ledTHREE, LOW);
    digitalWrite(ledFOUR, LOW);
    break;

    case 2:
    digitalWrite(ledONE, HIGH);
    digitalWrite(ledTWO, HIGH);
    digitalWrite(ledTHREE, LOW);
    digitalWrite(ledFOUR, LOW);
    break;

    case 3:
    digitalWrite(ledONE, HIGH);
    digitalWrite(ledTWO, HIGH);
    digitalWrite(ledTHREE, HIGH);
    digitalWrite(ledFOUR, LOW);
    break;

    case 4:
    digitalWrite(ledONE, HIGH);
    digitalWrite(ledTWO, HIGH);
    digitalWrite(ledTHREE, HIGH);
    digitalWrite(ledFOUR, HIGH);
    break;
  }
}

//Deplacer servoMotor à n dégréé
void open(int n)
{
  servo.write(n);
}

//rester en place "0 dégréé"
void close()
{
  servo.write(0);
}


void setup() {
  servo.attach(23);
  dhtCaptur.setup(dhtPin, DHTesp::DHT22);
  pinMode(bleu, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(ledONE, OUTPUT);
  pinMode(ledTWO, OUTPUT);
  pinMode(ledTHREE, OUTPUT);
  pinMode(ledFOUR, OUTPUT);
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
 
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Wifi not connected");
  }
  Serial.println("Wifi connected !");
  Serial.println("Local IP: " + String(WiFi.localIP()));
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  
}

void loop() {
  //stocker les valeurs 
  detectCO2 = map(analogRead(co2Pin), 0, 4095, 0, maxCO2_);
  int ld_r = analogRead(ldrPin);
  TempAndHumidity  data = dhtCaptur.getTempAndHumidity();
  temp = data.temperature;
  humidity = data.humidity;
  float lux = AnalogToLux(ld_r);
  
  //Saisir les valeurs
  Serial.print("Température: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("CO2:");
  Serial.println(detectCO2);
  Serial.print("Luminosité:");
  Serial.print(lux);

  //Les conditions de contrôler le servoMotor
  if(detectCO2 > 1000 || temp > 25 || humidity > 70) open(90);
  else if(detectCO2 > 700 && detectCO2<1000) open(30);
  else close();

  //Les conditions de contrôler RGB LED
  if(temp < seuilChauf) colorRGB(255, 0, 0);
  else if(temp > seuilClima) colorRGB(0, 0, 255);
  else colorRGB(0, 0, 0);

  //Les conditions de contrôler segment LED Bar
  if(lux <= dark)
  {
    Eclairage(1);
    Serial.println(":: Dark");
    brightness = 0;
  }
  else if(lux > dark && lux < light)
  {
    Eclairage(2);
    Serial.println(":: Light");
    brightness = 1;
  }
  else if(lux >= light && lux < bright)
  {
    Eclairage(3);
    Serial.println(" :: Light");
    brightness = 1;
  }
  else if(lux >= bright)
  {
    Eclairage(4);
    Serial.println(":: Bright");
    brightness = 2;
  }
  
  delay(10);

  sendDataToThingSpeak();
  delay(16000);
}

//fonction de transmettre les données vers ThingSpeak
void sendDataToThingSpeak()
{
  ThingSpeak.setField(1,temp);
  delay(500);
  ThingSpeak.setField(2,humidity);
  delay(500);
  ThingSpeak.setField(3,detectCO2);
  delay(500);
  ThingSpeak.setField(4,brightness);
  
  
  int writeResult = ThingSpeak.writeFields(myChannelNumber,write_API_Key);
  
  if(writeResult == 200){
    Serial.println("Data pushed successfull");
  }else{
    Serial.println("Push error" + String(writeResult));
  }
  Serial.println("---");
}
