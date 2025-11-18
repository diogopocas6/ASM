#pragma once
#include <Arduino.h>
struct SensorReadings{bool objectDetected; bool delivered;};

class Sensors{
    public:
    void begin(uint8_t A0,uint8_t A1);
    SensorReadings read();
    private:
    uint8_t _obj, _drop;
};
