#include "StickGesture.h"
#include "GY521.h"

#define ACCEL_SCALE          16384.0f   // MPU6050 ±2g
#define IMPACT_THRESHOLD_G   2.5f
#define IMPACT_COOLDOWN_MS   400

StickGesture::StickGesture()
    : lastImpactTime(0)
{
}

float StickGesture::getAccelerationMagnitudeG()
{
    float ax = accelerometer_x / ACCEL_SCALE;
    float ay = accelerometer_y / ACCEL_SCALE;
    float az = accelerometer_z / ACCEL_SCALE;

    return sqrt(ax * ax + ay * ay + az * az);
}

StickGesture::Gesture StickGesture::loopGesture()
{
    // IMPORTANT:
    // imuUpdate() must be called BEFORE this function

    float accMag = getAccelerationMagnitudeG();
    unsigned long now = millis();

    if (accMag > IMPACT_THRESHOLD_G)
    {
        if (now - lastImpactTime > IMPACT_COOLDOWN_MS)
        {
            lastImpactTime = now;
            return Gesture::HitGround;
        }
    }

    return Gesture::None;
}