#include "StickGesture.h"
#include "GY521.h"

#define ACCEL_SCALE          16384.0f   // MPU6050 ±2g
#define IMPACT_THRESHOLD_G   2.5f
#define IMPACT_COOLDOWN_MS   400
#define GYRO_MOVEMENT_THRESHOLD_DPS 50.0f
#define STILLNESS_TIMEOUT_MS 2000UL

StickGesture::StickGesture()
    : lastImpactTime(0),
      lastMovementTimestamp(0),
      movementState(MovementState::Moving)
{
}

float StickGesture::getAccelerationMagnitudeG()
{
    float ax = accelerometer_x / ACCEL_SCALE;
    float ay = accelerometer_y / ACCEL_SCALE;
    float az = accelerometer_z / ACCEL_SCALE;

    return sqrt(ax * ax + ay * ay + az * az);
}

float StickGesture::getGyroMagnitudeDps() const
{
    float gx = static_cast<float>(gyro_x);
    float gy = static_cast<float>(gyro_y);
    float gz = static_cast<float>(gyro_z);
    return sqrt((gx * gx) + (gy * gy) + (gz * gz));
}

StickGesture::Gesture StickGesture::loopGesture()
{
    float accMag = getAccelerationMagnitudeG();
    float gyroMag = getGyroMagnitudeDps();
    unsigned long now = millis();

    updateMovementState(gyroMag, now);

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

void StickGesture::updateMovementState(float gyroMagnitude, unsigned long now)
{
    if (lastMovementTimestamp == 0)
    {
        lastMovementTimestamp = now;
    }

    if (lastgyroMagnitude == 0)
    {
        lastgyroMagnitude = gyroMagnitude;
    }

    const bool currentlyMoving = abs(lastgyroMagnitude - gyroMagnitude) >= GYRO_MOVEMENT_THRESHOLD_DPS;
	lastgyroMagnitude = gyroMagnitude;

    if (currentlyMoving)
    {
        Serial.println("[StickGesture] Movement detected. Gyro magnitude: " + String(gyroMagnitude));
        lastMovementTimestamp = now;
        if (movementState == MovementState::Still)
        {
            movementState = MovementState::Moving;
            Serial.println(F("[StickGesture] State transition: Still -> Moving"));
        }
        return;
    }

    if (movementState == MovementState::Moving)
    {
        const unsigned long idleDuration = now - lastMovementTimestamp;
        if (idleDuration >= STILLNESS_TIMEOUT_MS)
        {
            movementState = MovementState::Still;
            Serial.println(F("[StickGesture] State transition: Moving -> Still"));
        }
    }
}

StickGesture::MovementState StickGesture::getMovementState() const
{
    return movementState;
}
