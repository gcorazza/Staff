#include "StickGesture.h"
#include "GY521.h"

#define ACCEL_SCALE          16384.0f   // MPU6050 ±2g
#define IMPACT_COOLDOWN_MS   400
#define GYRO_MOVEMENT_THRESHOLD_DPS 50.0f
#define STILLNESS_TIMEOUT_MS 2000UL
#define IMPACT_THRESHOLD_G_HIT   5.5f
#define IMPACT_THRESHOLD_G_TAB   1.2f
#define DOUBLE_TAP_MIN_MS        300
#define DOUBLE_TAP_MAX_MS        500

StickGesture::StickGesture()
    : lastImpactTime(0),
      lastTabTime(0),
      lastMovementTimestamp(0),
      movementState(MovementState::Moving),
      tabState(TabState::Idle)
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

    // (Double) Tap Detection
    switch (tabState) {
        case TabState::Idle:
            if (accMag > IMPACT_THRESHOLD_G_TAB && accMag < IMPACT_THRESHOLD_G_HIT) {
                lastTabTime = now;
                tabState = TabState::FirstTap;
            }
            break;
        case TabState::FirstTap:
            if (now - lastTabTime > DOUBLE_TAP_MAX_MS) {
                tabState = TabState::Idle;
                Serial.println(F("[StickGesture] Tap detected!"));
				return Gesture::Tap;
            } else if (accMag > IMPACT_THRESHOLD_G_TAB && accMag < IMPACT_THRESHOLD_G_HIT) {
                unsigned long dt = now - lastTabTime;
                if (dt >= DOUBLE_TAP_MIN_MS && dt <= DOUBLE_TAP_MAX_MS) {
                    tabState = TabState::Idle;
                    lastTabTime = 0;
                    Serial.println(F("[StickGesture] DoubleTap detected!"));
                    return Gesture::DoubleTap;
                }
            }
            break;
    }

    // HitGround Detection (wie bisher)
    if (accMag > IMPACT_THRESHOLD_G_HIT)
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

    //for testing
	if (lastgyroMagnitude - gyroMagnitude > 1.2){
       Serial.println("[StickGesture] Big Gyro magnitude: " + String(lastgyroMagnitude - gyroMagnitude));
       Serial.print("gyro_magnitude:"); Serial.println(lastgyroMagnitude - gyroMagnitude);
    }

    const bool currentlyMoving = abs(lastgyroMagnitude - gyroMagnitude) >= GYRO_MOVEMENT_THRESHOLD_DPS;
	lastgyroMagnitude = gyroMagnitude;

    if (currentlyMoving)
    {
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
