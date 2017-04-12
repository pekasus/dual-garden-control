
#include <Wire.h>
#include <OneWire.h>

#define slaveAddress 10

OneWire  ds(10); // temp sensor on pin 10 / need 4k7 trans
byte addr[8] = {0x28,0xFF,0x12,0x54,0x22,0x14,0x00,0x6D}; // sensor WT1

// PWM 3, 5, 6, 9, 10, and 11

const byte o1 = 6;
const byte o2 = 7;
const byte o3 = 8;
const byte o4 = 9;
const byte a2 = A2;
const byte a3 = A3;
const byte p1 = 6;
const byte p2 = 9;
byte outs[4] = {o1,o2,o3,o4};

int read2;
int read3;
float celsius, fahrenheit;
int intcel, intfahr;
int inTents;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    pinMode(outs[i], OUTPUT);
  }
  Wire.begin(slaveAddress);
  Wire.onReceive(i2cReceive);
  Wire.onRequest(i2cTransmit);
}

void loop() {
  read2 = analogRead(a2);
  read3 = analogRead(a3);
  byte i;
  byte present = 0;
  byte data[12];
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1); 
  
  delay(1000);
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
//    Serial.print(data[i], HEX);
//    Serial.print(" ");
  }
//  Serial.println();
  int16_t raw = (data[1] << 8) | data[0];
  byte cfg = (data[4] & 0x60);

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  intcel = celsius * 100;
  intfahr = fahrenheit * 100;
}


void i2cReceive(int byteCount) {
  if (byteCount == 0) return;
  byte command = Wire.read();

  if (command < 0x80) {
    i2cHandleRx(command, byteCount - 1);
  } 
  else {
    i2cHandleTx(command);
  } 
  
}

byte i2cHandleRx(byte command, int numBytes) {
  switch (command) {
    case 0x46:
      digitalWrite(o1,LOW);
      Serial.println("O1 Low");
      break;
    case 0x56:
      digitalWrite(o1, HIGH);
      Serial.println("O1 High");
      break;
    case 0x47:
      digitalWrite(o2,LOW);
      Serial.println("O2 Low");
      break;
    case 0x57:
      digitalWrite(o2, HIGH);
      Serial.println("O2 High");
      break;
    case 0x48:
      digitalWrite(o3,LOW);
      Serial.println("o3 Low");
      break;
    case 0x58:
      digitalWrite(o3,HIGH);
      Serial.println("o3 High");
      break;
      case 0x49:
      digitalWrite(o4,LOW);
      Serial.println("o4 Low");
      break;
    case 0x59:
      digitalWrite(o4,HIGH);
      Serial.println("o4 High");
      break;
    case 0x10:
      inTents = Wire.read();
      analogWrite(p1, inTents);
      break;
    case 0x11:
      inTents = Wire.read();
      analogWrite(p2, inTents);
      break;

  }
}


byte i2cHandleTx(byte command) {
  //This is used when the master requests info. If there is anything to do
  //before the interrupt for the read to take place, do it here.
  return 0;
}


void i2cTransmit() {
  byte fahrhigh = ((intfahr >> 8) & 0xFF);
  byte fahrlow = (intfahr & 0xFF);
  byte celhigh = ((intcel >> 8) & 0xFF);
  byte cellow = (intfahr & 0xFF);
  byte read2high = ((read2 >> 8) & 0xFF);
  byte read2low = (read2 & 0xFF);
  byte read3high = ((read3 >> 8) & 0xFF);
  byte read3low = (read3 & 0xFF);
  byte bytesToSend[] = {fahrhigh, fahrlow, celhigh, cellow, read2high, read2low, read3high, read3low};
  Serial.print(intcel);
  Serial.print(" / ");
  Serial.println(intfahr);
  Serial.print(read2);
  Serial.print(" / ");
  Serial.println(read3);
  Serial.print("Data Sent: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(bytesToSend[i]);
    Serial.print(",");
  }
  Serial.println();
  Wire.write(bytesToSend,8);
}
