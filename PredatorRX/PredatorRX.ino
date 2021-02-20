// Необходимо установить библиотеку для радиомодуля nRF24
// MQ-9 21 02
#include <Servo.h>
#include <SPI.h>
#include "RF24.h"

RF24 radio(9, 10);

const uint64_t pipeIn = 0xE8E8F0F0E1LL;

struct PredatorData
{
    byte throttle;
    byte yaw;
    byte pitch;
    byte roll;
};
PredatorData data;
unsigned long lastRecvTime = 0;

int throttlePin = 7;    // Газ
int leftEleronPin = 6;  // Левое крыло
int rightEleronPin = 5; // Правое крыло
int keelPin = 4;        // Киль
int tailLeftPin = 3;    // Хвост левый
int tailRightPin = 2;    // Хвост правый

Servo throttleServo;
Servo leftEleronServo;
Servo rightEleronServo;
Servo keelServo;
Servo leftTailServo;
Servo rightTailServo;

const int motorMaxVal = 2300;
const int motorMinVal = 800;
const int tailLeftMinVal = 65;
const int tailLeftMaxVal = 115;
const int tailRightMinVal = 75;
const int tailRightMaxVal = 125;
const int eleronRightMinVal = 60;
const int eleronRightMaxVal = 120;
const int eleronLeftMinVal = 60;
const int eleronLeftMaxVal = 120;
const int keelMinVal = 60;
const int keelMaxVal = 160;

// #define DEBUG

void resetData()
{
    data.throttle = 0;
    data.yaw = 127;
    data.pitch = 127;
    data.roll = 127;
}

void setup()
{
    setupPins();
    setupServos();
    radio.begin();

#ifdef DEBUG
    bool check = radio.isChipConnected();
    Serial.begin(9600);
    Serial.print("check-");
    Serial.println(check);
#endif

    setupRadio();
    calibrateMotor();
    resetData();
}

void setupPins()
{
    pinMode(throttlePin, OUTPUT);
}

void setupServos()
{
    throttleServo.attach(throttlePin);
    leftEleronServo.attach(leftEleronPin);
    rightEleronServo.attach(rightEleronPin);
    keelServo.attach(keelPin);
    leftTailServo.attach(tailLeftPin);
    rightTailServo.attach(tailRightPin);
}

void setupRadio()
{
    // radio.setChannel(0x60);
    radio.setAutoAck(false);
    // radio.setRetries(0, 15);
    // radio.enableAckPayload();
    // radio.setPayloadSize(32);
    // radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    // radio.powerUp();
    radio.openReadingPipe(1, pipeIn);
    radio.startListening();
}

void calibrateMotor()
{
    throttleServo.writeMicroseconds(motorMaxVal);
    delay(2000);
    throttleServo.writeMicroseconds(motorMinVal);
    delay(6000);
}

void loop()
{
    while (radio.available())
    {
        radio.read(&data, sizeof(PredatorData));
        lastRecvTime = millis();
    }

#ifdef DEBUG
    Serial.print("Recieved ");
    Serial.print(data.throttle);
    Serial.print(" ");
    Serial.print(data.pitch);
    Serial.print(" ");
    Serial.print(data.roll);
    Serial.print(" ");
    Serial.print(data.yaw);
    Serial.print(" ");
    Serial.println();
#endif

    // Газ
    int torque = map(data.throttle, 0, 255, motorMinVal, motorMaxVal);
    torque = constrain(torque, motorMinVal, motorMaxVal);
    throttleServo.writeMicroseconds(torque);

    // Тангаж
    int eleronLeft = map(255 - data.pitch, 0, 255, eleronLeftMinVal, eleronLeftMaxVal);
    leftEleronServo.write(eleronLeft);
    
    int eleronRight = map(data.pitch, 0, 255, eleronRightMinVal, eleronRightMaxVal);
    rightEleronServo.write(eleronRight);

    // Крен
    int tailLeft = map(data.roll, 0, 255, tailLeftMinVal, tailLeftMaxVal);
    leftTailServo.write(tailLeft);

    int tailRight = map(data.roll, 0, 255, tailRightMinVal, tailRightMaxVal);
    rightTailServo.write(tailRight);


    // Рысканье
    int yaw = map(data.yaw, 0, 255, keelMinVal, keelMaxVal);
    keelServo.write(yaw);

    unsigned long now = millis();
    if (now - lastRecvTime > 5000)
    {
        resetData();
    }
}
