#include "sync.h" 

// NTP 
ntp_payload ntpReq;
EthernetUDP ntpUDP;
ntpState ntp_state_ = NTP_IDLE;
uint32_t t0_local_;


// Time references for GNSS sync
uint32_t local_t_ref_ = 0;
timeval global_t_ref_ = {0};
float drift_factor_ = 1;

void syncSetup(){
  ntpUDP.begin(NTP_CLIENT_PORT);

  // Setup ntpReq 
  ntpReq.flags = 0b00100011; // Ntp type 4, Client
}

bool ntpRequest() {
  if (!ntpUDP.beginPacket(NTP_SERVER_IP, NTP_SERVER_PORT))
    return false;

  size_t bytesWritten = ntpUDP.write((uint8_t*) &ntpReq, sizeof(ntpReq));
  if (bytesWritten != sizeof(ntpReq))
    return false;

  if (!ntpUDP.endPacket())
    return false;

  t0_local_ = micros();
  ntp_state_ = NTP_REQUEST;

  return true;
}


uint32_t byteArrayToUint32(uint8_t* arr){
  return (arr[0] << 24) | (arr[1] << 16) | (arr[2] << 8) | (arr[3] << 0);
}

timeval getTimeval(ntp_ts ts){
  return timeval{
    byteArrayToUint32(ts.sec) - NTP_OFFSET,
    (uint32_t)((double) byteArrayToUint32(ts.frac) * 1.0e6 / (double)0xFFFFFFFF)
  };
}

void ntpResponse(uint32_t t3_local){
  // Read NTP response
  ntp_payload ntpResp;
  ntpUDP.read((uint8_t*) &ntpResp, sizeof(ntpReq)); // read the packet into the buffer

  // 1. Calculate one-way delay (in microseconds)    
  timeval t1_ntp = getTimeval(ntpResp.ts_rec);
  timeval t2_ntp = getTimeval(ntpResp.ts_trans);

  uint32_t delay = (drift_factor_*(t3_local - t0_local_) - getTimeDiff(t1_ntp, t2_ntp)) / 2; // Microsecond delay (one-way)
  
  newReference(t0_local_ + delay, t1_ntp);
}

void ntpCheckResponse(){
  if (ntpUDP.parsePacket() == sizeof(ntp_payload)){
    uint32_t t3_local = micros(); // Time of reception
    ntpResponse(t3_local);
  };
}

void syncUpdate(){
  switch (ntp_state_){
    case NTP_IDLE:
      if (drift_factor_*(micros() - local_t_ref_) > NTP_UPDATE_INTERVAL)
        ntpRequest();
      break;
    case NTP_REQUEST:
      ntpCheckResponse();
      if (micros() - t0_local_ > NTP_TIMEOUT)
        ntp_state_ = NTP_IDLE;
      break;
    default:
      ntp_state_ = NTP_IDLE;
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
  drift_factor_ = 0.9*drift_factor_ + 0.1*new_drift_factor_; // Use exponential average for drift
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