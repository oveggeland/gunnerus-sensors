#pragma once 

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <stdint.h>
#include <Arduino.h>

#define OUTPUT_BUFFER_SIZE 1472 // MTU for Ethernet shield is 1500, subtract ipv4 and udp headers to get 1472 as maximum buffer size
#define SD_CARD_PIN 4

#define UDP_MIN_PAYLOAD_SIZE 256 // Don't send payload before buffer is this size (This reduces overhead)

// Define addresses and network ports
#define USE_STATIC_IP true

#define MAC_ADDRESS {0xA8, 0x61, 0x0A, 0xAF, 0x15, 0x59}
#define LOCAL_IP IPAddress(192, 168, 1, 82)
#define HOST_IP IPAddress(192, 168, 1, 52)

#define LOCAL_PORT 8888
#define HOST_PORT 5005

void networkSetup();
void networkUpdate();

// Used to push data to the output buffer (E.g. gnss or imu using this to send data to host computer)
void networkPushData(uint8_t* src_buffer, uint16_t size);