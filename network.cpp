#include "network.h"


EthernetUDP pcb;

uint8_t output_buffer[OUTPUT_BUFFER_SIZE];
uint16_t output_buffer_cnt;


/*
Starts network module. Called from main setup() function
*/
void networkSetup(){
  SPI.begin(); 

  // Disable SD card
  pinMode(SD_CARD_PIN, OUTPUT); 
  digitalWrite(SD_CARD_PIN, HIGH);

  // Start ethernet
  byte mac[] = MAC_ADDRESS;
  if (USE_STATIC_IP)
    Ethernet.begin(mac, LOCAL_IP);
  else
    Ethernet.begin(mac);
  
  // Connet to port
  pcb.begin(LOCAL_PORT);
}



/*
Sends a packet over UDP. Reports error (returns false) if anything goes wrong in the process. 
*/
bool sendUdpMsg(EthernetUDP *pcb, IPAddress dst_ip, int dst_port, uint8_t *buffer, uint16_t size) {
  // Begin packet
  if (!pcb->beginPacket(dst_ip, dst_port)) {
    return false;
  };

  // Write bytes
  uint32_t bytesWritten = pcb->write(buffer, size);
  if (bytesWritten != size) {
    return false;
  };

  // Send packet
  if (!pcb->endPacket()) {
    return false;
  }

  return true;
}


/*
Called from the main loop.
Maintains DHCP if required. 
Clears network buffer and sends payload regulary
*/
void networkUpdate(){
  // Maintain DHCP if required
  if (!USE_STATIC_IP)
    Ethernet.maintain();

  // Check if buffer size is big enough for sending
  if (output_buffer_cnt < UDP_MIN_PAYLOAD_SIZE)
    return;

  // Send payloads and clear buffer on success
  if (sendUdpMsg(&pcb, HOST_IP, HOST_PORT, output_buffer, output_buffer_cnt))   {} 
    output_buffer_cnt = 0;
}


/*
Any module can use this function to push data over the network. The buffer is checked and sent regularly.
*/
void networkPushData(uint8_t* src_buffer, uint16_t size){
  if (size <= OUTPUT_BUFFER_SIZE - output_buffer_cnt){
    memcpy(output_buffer + output_buffer_cnt, src_buffer, size);
    output_buffer_cnt += size;
  }
};