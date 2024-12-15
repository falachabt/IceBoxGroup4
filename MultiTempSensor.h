#ifndef MULTI_TEMP_SENSOR_H
#define MULTI_TEMP_SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

class MultiTempSensor {
private:
    DallasTemperature* tempSensors[4];
    OneWire* oneWireObjects[4];
    const int sensorPins[4] = {
        PinConfig::AMBIENT_TEMP_SENSOR,
        PinConfig::ICE_SURFACE_TEMP_SENSOR,
        PinConfig::ICE_INNER_TEMP_SENSOR,
        PinConfig::BASE_TEMP_SENSOR
    };

public:
    struct TemperatureReadings {
        float ambient;
        float surface;
        float inner;
        float base;
        bool isValid;
    };

    MultiTempSensor() {
        for (int i = 0; i < 4; i++) {
            oneWireObjects[i] = new OneWire(sensorPins[i]);
            tempSensors[i] = new DallasTemperature(oneWireObjects[i]);
            tempSensors[i]->begin();
            tempSensors[i]->setResolution(12); // Set to 12-bit resolution
        }
    }

    TemperatureReadings getAllTemperatures() {
        TemperatureReadings readings;
        readings.isValid = true;

        // Request temperatures from all sensors
        for (int i = 0; i < 4; i++) {
            tempSensors[i]->requestTemperatures();
        }

        // Read temperatures with error checking
        readings.ambient = tempSensors[0]->getTempCByIndex(0);
        readings.surface = tempSensors[1]->getTempCByIndex(0);
        readings.inner = tempSensors[2]->getTempCByIndex(0);
        readings.base = tempSensors[3]->getTempCByIndex(0);

        // Validate readings
        if (readings.ambient == DEVICE_DISCONNECTED_C ||
            readings.surface == DEVICE_DISCONNECTED_C ||
            readings.inner == DEVICE_DISCONNECTED_C ||
            readings.base == DEVICE_DISCONNECTED_C) {
            readings.isValid = false;
        }

        return readings;
    }

    ~MultiTempSensor() {
        for (int i = 0; i < 4; i++) {
            delete tempSensors[i];
            delete oneWireObjects[i];
        }
    }
};

#endif