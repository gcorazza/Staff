// (c) Michael Schoeffler 2017, http://www.mschoeffler.de

#include "Wire.h" // This library allows you to communicate with I2C devices.
#include <Arduino.h>

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

#define SDA_PIN 22
#define SDL_PIN 21

int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data

char tmp_str[15]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%d", i);
  return tmp_str;
}

void fifoReset() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6A);        // USER_CTRL
  Wire.write(0b00000100);  // FIFO_RESET
  Wire.endTransmission();
}

void fifoEnable() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6A);        // USER_CTRL
  Wire.write(0b01000000);  // FIFO_EN
  Wire.endTransmission();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x23);        // FIFO_EN register
  Wire.write(0b01111000);  // Accel + Gyro XYZ
  Wire.endTransmission();
}

uint16_t fifoCount() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x72); // FIFO_COUNTH
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);

  return (Wire.read() << 8) | Wire.read();
}

void setupGy() {
  Wire.begin(SDA_PIN, SDL_PIN);
  Wire.setClock(400000);

  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // Sample Rate: 1kHz / (1 + 9) = 100 Hz
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x19); // SMPLRT_DIV
  Wire.write(9);
  Wire.endTransmission();

  // Digital Low Pass Filter (~42 Hz)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A); // CONFIG
  Wire.write(0x03);
  Wire.endTransmission();

  // Gyro ±250 °/s
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();

  // Accel ±2 g
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission();

  fifoReset();
  fifoEnable();
}

void loopGy() {
  uint16_t count = fifoCount();

  // Überlauf-Schutz
  if (count >= 1024) {
    fifoReset();
    fifoEnable();
    return;
  }

  // Ein Sample = 12 Bytes
  while (count >= 12) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x74); // FIFO_R_W
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 12, true);

    accelerometer_x = Wire.read()<<8 | Wire.read();
    accelerometer_y = Wire.read()<<8 | Wire.read();
    accelerometer_z = Wire.read()<<8 | Wire.read();
    gyro_x = Wire.read()<<8 | Wire.read();
    gyro_y = Wire.read()<<8 | Wire.read();
    gyro_z = Wire.read()<<8 | Wire.read();

    count -= 12;

    // Beispiel-Ausgabe (optional!)
    Serial.print("aX:"); Serial.print(accelerometer_x);
    Serial.print(",aY:"); Serial.print(accelerometer_y);
    Serial.print(",aZ:"); Serial.print(accelerometer_z);
    Serial.print(",gX:"); Serial.print(gyro_x);
    Serial.print(",gY:"); Serial.print(gyro_y);
    Serial.print(",gZ:"); Serial.println(gyro_z);
  }
}
