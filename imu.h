#pragma once

// STD libs
#include <SPI.h>

// Code imports
#include "ADIS16364.h"
#include "network.h"
#include "sync.h"

typedef struct{
  char header[4] = {'$', 'I', 'M', 'U'};
  uint32_t sec;
  uint32_t usec;
  int16_t acc[3];
  int16_t rate[3];
} imuPackage;

#define IMU_CS_PIN 7
#define IMU_DR_PIN 2

#define IMU_SPI_SETTINGS SPISettings(300000, MSBFIRST, SPI_MODE3)

void imuSetup();
void imuUpdate();