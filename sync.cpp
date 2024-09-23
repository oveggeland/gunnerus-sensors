#include "sync.h" 

// Time references for GNSS sync
uint32_t local_t_ref_ = 0;
timeval global_t_ref_ = {0};
float drift_factor_ = DEFAULT_DRIFT_FACTOR;

ntpState ntp_state_ = NTP_IDLE;

void syncSetup(){
  return;
  ntp_state_ = NTP_IDLE;
}

void ntpRequest(){
  Serial.println("NTP REQUEST");
  ntp_state_ = NTP_REQUEST;
}

void ntpCheckResponse(){
  Serial.println("NTP RESPONSE");
  ntp_state_ = NTP_IDLE;

  // At some point we have a new reference
  uint32_t us = micros();
  newReference(us, timeval{
    us / 1e6, us - us / 1e6
  });
}

void syncUpdate(){
  switch(ntp_state_){
    case NTP_IDLE:
      if (micros() - local_t_ref_ > NTP_UPDATE_INTERVAL){
        ntpRequest();
      }
      break;
    case NTP_REQUEST:
      ntpCheckResponse();
      break;
    default:
      break;
  }
}



void newReference(uint32_t new_local_ref, timeval new_global_ref){

  int32_t dt_local = new_local_ref - local_t_ref_;
  int32_t dt_global = getTimeDiff(global_t_ref_, new_global_ref);

  float new_drift_factor_ = (float)dt_global / (float)dt_local;
  if (abs(new_drift_factor_ - 1) > NTP_DRIFT_ATOL){
    new_drift_factor_ = drift_factor_;
  }

  noInterrupts();
  global_t_ref_ = new_global_ref;
  local_t_ref_ = new_local_ref;
  drift_factor_ = 0.9*drift_factor_ + 0.1*new_drift_factor_; // Use exponential average for drift (to avoid heavy jitter)
  interrupts();
}


// Microsecond difference (t1 - t0)
int32_t getTimeDiff(uint32_t t0_sec, uint32_t t0_usec, uint32_t t1_sec, uint32_t t1_usec){
  return 1e6*(int32_t)(t1_sec-t0_sec) + (int32_t)(t1_usec - t0_usec);
}

// Microsecond difference (t1 - t0)
int32_t getTimeDiff(timeval t0, timeval t1){
  return getTimeDiff(t0.sec, t0.usec, t1.sec, t1.usec);
}

// Get current time estimate from sync module
void getCurrentTime(uint32_t &sec, uint32_t &usec){
  uint32_t t_local = micros();

  usec = global_t_ref_.usec + drift_factor_*(t_local - local_t_ref_);
  sec = global_t_ref_.sec;

  uint32_t seconds_passed = usec / 1e6;

  usec -= seconds_passed*1e6;
  sec += seconds_passed;
}

// Get current time estimate from sync module
timeval getCurrentTime(){
  timeval t;
  getCurrentTime(t);
  return t;
}

void getCurrentTime(timeval& t){
  getCurrentTime(t.sec, t.usec);
}

// Pretty print helper for timestamps
void printTime(uint32_t sec, uint32_t usec, bool new_line){
  Serial.print(sec);
  Serial.print(".");
  if (usec < 10){ Serial.print("00000"); }
  else if (usec < 100){ Serial.print("0000"); }
  else if (usec < 1000){ Serial.print("000"); }
  else if (usec < 10000) { Serial.print("00"); }
  else if (usec < 100000){ Serial.print("0"); }
  Serial.print(usec);

  if (new_line)
    Serial.println();
}

// Pretty print helper for timestamps
void printTime(timeval t, bool new_line){
  printTime(t.sec, t.usec, new_line);
}