#include "imu.h"

bool drdy = false;
imuPackage pkg;

// Status/Diagnostics variables
uint16_t readReg(uint8_t addr){
  SPI.beginTransaction(IMU_SPI_SETTINGS);  
  digitalWrite(IMU_CS_PIN, LOW); // Chip select
  
  // SPI COMMANDS
  SPI.transfer16(addr << 8); // Read request from regAddr
  uint16_t output = SPI.transfer16(SPI_NOP); // Send dummy bytes to read the output

  digitalWrite(IMU_CS_PIN, HIGH); // Chip select
  SPI.endTransaction();

  return output;
}

/*
Input uint16_t addr contains page number (first byte), and register addr (second byte)
Input uint16_t value contains the value to write to selected register
*/
void writeReg(uint8_t addr, uint16_t data){
  SPI.beginTransaction(IMU_SPI_SETTINGS);  
  digitalWrite(IMU_CS_PIN, LOW); // Chip select
  
  // SPI COMMANDS
  SPI.transfer16(0x8000 | (addr << 8) | (data & 0x00FF)); // Write to lower byte first
  SPI.transfer16(0x8000 | (addr+1 << 8) | (data >> 8)); // Write to upper byte

  digitalWrite(IMU_CS_PIN, HIGH); // Chip select
  SPI.endTransaction();
}

void drdyISR(void){
  getCurrentTime(pkg.sec, pkg.usec);
  drdy = true;
}

void imuSetup(){
  SPI.begin(); // This is also called by Ethernet.h, but no worries about that. Safer to do both

  // Setup pins
  pinMode(IMU_CS_PIN, OUTPUT); // Set CS pin to be an output
  digitalWrite(IMU_CS_PIN, HIGH); // Initialize CS pin to be high

  // Wake up IMU
  digitalWrite(IMU_CS_PIN, LOW);
  delay(1);
  digitalWrite(IMU_CS_PIN, HIGH);

  // Software resets
  writeReg(GLOB_CMD, 1 << 7); // This is the equivalent of triggering the RST pin (takes 1.8 seconds according to datasheet)
  delay(1800);

  // Configure interrupt on data ready (on ADIS)
  writeReg(MSC_CTRL, 0b111); // Bits are: 1. Data ready enable, 2. Data ready polarity (active high), 3. Data ready line select (Using DIO2)

  // Setup interrupt on Arduino
  pinMode(IMU_DR_PIN, INPUT);
  pinMode(IMU_DR_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(IMU_DR_PIN), drdyISR, RISING);
}

int16_t register_to_signed_int(uint16_t reg, uint8_t shift){
  return ((int16_t)(reg << shift)) >> shift;
}

void burstRead(int16_t* acc, int16_t* rate){
  SPI.beginTransaction(IMU_SPI_SETTINGS);
  digitalWrite(IMU_CS_PIN, LOW);

  SPI.transfer16(XACCL_OUT << 8);
  uint16_t x_acc_raw = SPI.transfer16(YACCL_OUT << 8);
  uint16_t y_acc_raw = SPI.transfer16(ZACCL_OUT << 8);
  uint16_t z_acc_raw = SPI.transfer16(XGYRO_OUT << 8);
  uint16_t x_gyro_raw = SPI.transfer16(YGYRO_OUT << 8);
  uint16_t y_gyro_raw = SPI.transfer16(ZGYRO_OUT << 8);
  uint16_t z_gyro_raw = SPI.transfer16(0x0000);

  acc[0] = register_to_signed_int(x_acc_raw, 2);
  acc[1] = register_to_signed_int(y_acc_raw, 2);
  acc[2] = register_to_signed_int(z_acc_raw, 2);
  rate[0] = register_to_signed_int(x_gyro_raw, 2);
  rate[1] = register_to_signed_int(y_gyro_raw, 2);
  rate[2] = register_to_signed_int(z_gyro_raw, 2);

  digitalWrite(IMU_CS_PIN, HIGH);
  SPI.endTransaction();
}

uint32_t false_cnt = 0;
void imuUpdate(){
  if (drdy){
    // New data ready
    // printTime(pkg.sec, pkg.usec, true);
    burstRead(pkg.acc, pkg.rate);
    Serial.println(pkg.acc[2]);
    networkPushData((uint8_t*) &pkg, sizeof(pkg));

    drdy=false;
  }
}
