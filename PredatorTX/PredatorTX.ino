#include <SPI.h>
#include "RF24.h"

RF24 radio(9, 10);

const uint64_t pipeOut = 0xE8E8F0F0E1LL;

// The sizeof this struct should not exceed 32 bytes
struct PredatorData
{
    byte throttle;
    byte yaw;
    byte pitch;
    byte roll;
};
PredatorData data;

int throttlePin = A3; // Газ
int pitchPin = A4;    // Вверх вниз тангаж
int rollPin = A5;     // Влево вправо крен
int yawPin = A2;      // Рысканье

// #define DEBUG

void resetData()
{
    data.throttle = 0;
    data.yaw = 127;
    data.pitch = 127;
    data.roll = 127;
}

void setup(void)
{
    setupPins();
    radio.begin();

#ifdef DEBUG
    bool check = radio.isChipConnected();
    Serial.begin(9600);
    Serial.print("check-");
    Serial.println(check);
#endif

    setupRadio();
    resetData();
}

void setupRadio()
{
    radio.setAutoAck(false);
    // radio.setRetries(0, 15);
    // radio.enableAckPayload();
    // radio.setPayloadSize(32);
    // radio.setChannel(0x60);
    // radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    // radio.powerUp();
    // radio.stopListening();
    radio.openWritingPipe(pipeOut);
}

void setupPins()
{
    pinMode(throttlePin, INPUT);
    pinMode(pitchPin, INPUT);
    pinMode(rollPin, INPUT);
    pinMode(yawPin, INPUT);
}

int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse)
{
    val = constrain(val, lower, upper);
    if (val < middle)
        val = map(val, lower, middle, 0, 128);
    else
        val = map(val, middle, upper, 128, 255);
    return (reverse ? 255 - val : val);
}

void loop()
{
    // Газ
    data.throttle = mapJoystickValues(analogRead(throttlePin), 0, 511, 1023, false);
    // Тангаж
    data.pitch = mapJoystickValues(analogRead(pitchPin), 0, 511, 1023, false);
    // Крен
    data.roll = mapJoystickValues(analogRead(rollPin), 0, 511, 1023, true);
    // Рысканье
    data.yaw = mapJoystickValues(analogRead(yawPin), 0, 511, 1023, false);
    // Передача по nrf
    radio.write(&data, sizeof(PredatorData));

#ifdef DEBUG
    Serial.print("Transmitted ");
    Serial.print(data.throttle); Serial.print(" ");
    Serial.print(data.pitch); Serial.print(" ");
    Serial.print(data.roll); Serial.print(" ");
    Serial.print(data.yaw); Serial.print(" ");
    Serial.println();
#endif
}
