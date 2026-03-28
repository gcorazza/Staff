#ifndef GY521_H
#define GY521_H

#include <Arduino.h>

void setupGy();
void loopGy();

extern int16_t accelerometer_x, accelerometer_y, accelerometer_z;
extern int16_t gyro_x, gyro_y, gyro_z;
extern int16_t temperature;

#endif  // GY521_H
