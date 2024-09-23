#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <Arduino.h>

#define NTP_DRIFT_ATOL 0.1 // Tolerance for drift deviation (away from 1.0 corresponding to no drift)
#define DEFAULT_DRIFT_FACTOR 1.003

#define NTP_UPDATE_INTERVAL 4e6

// This struct defines the system wide time
typedef struct{
  uint32_t sec;
  uint32_t usec;
} timeval;

typedef enum {
    NTP_IDLE,
    NTP_REQUEST
} ntpState;

void syncSetup();
void syncUpdate();

void getCurrentTime(uint32_t &sec, uint32_t &usec);
void getCurrentTime(timeval &t);
timeval getCurrentTime();

void printTime(uint32_t sec, uint32_t usec, bool new_line);
void printTime(timeval t, bool new_line);

int32_t getTimeDiff(uint32_t t0_sec, uint32_t t0_usec, uint32_t t1_sec, uint32_t t1_usec);
int32_t getTimeDiff(timeval t0, timeval t1);

void newReference(uint32_t local_ref, timeval global_ref);