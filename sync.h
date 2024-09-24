#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <Arduino.h>
#include <EthernetUdp.h>

#define NTP_SERVER_IP IPAddress(192, 168, 1, 52)
#define NTP_SERVER_PORT 123
#define NTP_CLIENT_PORT 1023
#define NTP_UPDATE_INTERVAL 4e6 // Microseconds
#define NTP_TIMEOUT 5e5 // Half second timeout
#define NTP_OFFSET 2208988800
#define NTP_DRIFT_ATOL 0.1

// This struct defines the system wide time
typedef struct{
  uint32_t sec;
  uint32_t usec;
} timeval;

// State of NTP
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

typedef struct{
  uint8_t sec[4];
  uint8_t frac[4];
} ntp_ts;

typedef struct{
  uint8_t flags;
  uint8_t stratum;
  uint8_t interval;
  uint8_t precision;
  uint32_t root_delay;
  uint32_t root_disp;
  uint32_t ref_id;
  ntp_ts ts_ref;
  ntp_ts ts_orig;
  ntp_ts ts_rec;
  ntp_ts ts_trans;
} ntp_payload;