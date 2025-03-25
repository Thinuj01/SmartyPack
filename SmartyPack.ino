#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <MPU6050.h>
SoftwareSerial gpsSerial(8, 9);
TinyGPSPlus gps;
float lattitude = 0.00;
float longitude = 0.00;
bool breakLoops = true;
#define BUZZER_PIN 6
#define LED_PIN 10
#define LDR_PIN A0      
SoftwareSerial sim(3, 2);
int _timeout;
String _buffer;
String number = "+94727909306";

MPU6050 mpu;

void setup() {

  gpsSerial.begin(9600);
  Serial.begin(9600);
  readLocation();
  siminit();
  pinMode(10, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT); 
  Wire.begin();
  mpu.initialize();


  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }
  
}

void loop() {

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  Serial.print("MPU6050 - Accelerometer: ");
  Serial.print("X: ");
  Serial.print(ax);
  Serial.print(" | Y: ");
  Serial.print(ay);
  Serial.print(" | Z: ");
  Serial.println(az);


  float accel = sqrt(pow(ax, 2) + pow(ay, 2) + pow(az, 2)) / 16384.0;  
  if (accel > 0.90) {
    readLocation();
    SendMessage();
    triggerBuzzer();

  }
  Serial.print("Accel: ");
  Serial.println(accel);
    int ldrValue = analogRead(LDR_PIN);

  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  if(ldrValue < 200){
    digitalWrite(10,HIGH);
    Serial.println("LDR - Light detected! LEDs OFF");
    delay(200); 
  }else{
    digitalWrite(10,LOW);
    Serial.println("LDR - Darkness detected! LEDs ON");
    delay(200);  
  }
  delay(1000);
}

void siminit() {
  delay(7000);
  Serial.begin(9600);
  _buffer.reserve(50);
  Serial.println("Sistem Started...");
  sim.begin(9600);
  delay(1000);
}

void readLocation() {
  while(breakLoops){
  while(gpsSerial.available()) {
    int data = gpsSerial.read();
    if (gps.encode(data)) {
      lattitude = (gps.location.lat());
      longitude = (gps.location.lng());
      Serial.print("lattitude: ");
      Serial.println(lattitude);
      Serial.print("longitude: ");
      Serial.println(longitude);
      breakLoops = false;
      break;
    }else{
       lattitude = 2.0000;
      longitude = 2.0000;
    }
  }
  }
}
void SendMessage()
{
  sim.println("AT+CMGF=1"); 
  delay(1000);
  sim.println("AT+CMGS=\"" + number + "\"\r"); 
  delay(1000);

  String googleMapsLink = "https://www.google.com/maps?q=" + String(lattitude, 6) + "," + String(longitude, 6);
  

  String SMS = "Accident Detected, here is the location: " + googleMapsLink;

  sim.println(SMS);
  delay(100);
  sim.println((char)26); 
  delay(1000);
  _buffer = _readSerial();
}

void RecieveMessage()
{
  Serial.println ("SIM800L Read an SMS");
  delay (1000);
  sim.println("AT+CNMI=2,2,0,0,0"); 
  delay(1000);
  Serial.write ("Unread Message done");
}

String _readSerial() {
  _timeout = 0;
  while  (!sim.available() && _timeout < 12000  )
  {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
}

void callNumber() {
  sim.print (F("ATD"));
  sim.print (number);
  sim.print (F(";\r\n"));
  _buffer = _readSerial();
  Serial.println(_buffer);
}

void triggerBuzzer() {
  Serial.println("Buzzer - Impact Detected! Triggering Alarm...");
  tone(BUZZER_PIN, 300,250);
  delay(5000);
}