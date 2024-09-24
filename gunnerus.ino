#include "sync.h"
#include "network.h"
#include "imu.h"


/*
Setup required modules. 
*/
void setup() {
  // Setup serial comms (Only for debugging! Remove for deployment/testing)
  Serial.begin(115200);
  while (!Serial);

  // Network setup must be before sync setup
  networkSetup();

  syncSetup();

  imuSetup();
}



/*
All function calls in loop() should be as efficient as possible. This is crucial for optimal timing performance.

- No serial prints, this is very slow... Use it only for debugging
- Don't wait wait for response, e.g. in NTP requests. Send a request, exit the loop, check for an answer next time around. This will free up resources while waiting for a response.
- Never use delay()
*/
void loop() {
  syncUpdate();

  networkUpdate();

  imuUpdate();
}
