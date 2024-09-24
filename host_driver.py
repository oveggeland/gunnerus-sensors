import socket
import struct

import rospy
from sensor_msgs.msg import Imu, NavSatFix

import netifaces as ni
UDP_IP = "192.168.1.52" # ni.ifaddresses('enp87s0')[ni.AF_INET][0]['addr']
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

IMU_HEADER = "$IMU"
GNSS_HEADER = "$GNSS"
STATUS_HEADER = "$ST"

IMU_STRUCT_SIZE = 24
GNSS_STRUCT_SIZE = 25
STATUS_STRUCT_SIZE = 28


def parse_imu_data(data):
    if len(data) != IMU_STRUCT_SIZE:
        print("IMU struct length error...")
    
    t_sec, t_usec, acc_x, acc_y, acc_z, rate_x, rate_y, rate_z = struct.unpack("@IIhhhhhh", data[len(IMU_HEADER):])

    # Acceleration is transmitted in milli-g's, where g is the gravity norm at factory calibration (I think).
    # Angular velocity is transmitted with as a signed integer where the scale is 0.05 deg/sec
    
    acc_x *= 9.81e-3 # NB: Not sure about the exact scale used for calibration of the IMU. 9.81 is a "safe guess". 
    acc_y *= 9.81e-3
    acc_z *= 9.81e-3
    
    rate_x *= 0.05
    rate_y *= 0.05
    rate_z *= 0.05

    print(f"{t_sec}.{t_usec:06d} - Acc: [{acc_x:+09.2f}, {acc_y:+09.2f}, {acc_z:+09.2f}], Rate: [{rate_x:+07.2f}, {rate_y:+07.2f}, {rate_z:+07.2f}]")


def parse_gnss_data(data):
    if len(data) != GNSS_STRUCT_SIZE:
        print("GNSS struct length error...")

    t_sec, t_usec, lat, lng, alt = struct.unpack("@IIiii", data[len(GNSS_HEADER):])

    print(f"{t_sec}.{t_usec:06d} - Lat: {lat}, Lng: {lng}, Alt: {alt}")

if __name__ == "__main__":
    print("Using ip:", UDP_IP)

    while not rospy.is_shutdown():
        data, addr = sock.recvfrom(2048) 

        idx = data.find(b'$') # '$' is the header sign, this is placed at the beginning of each package
        while idx != -1:
            data = data[idx:] # Remove random bytes before header start ('$')
            
            # Check for different headers
            if data[:len(IMU_HEADER)] == bytes(IMU_HEADER, 'utf-8'):
                imu_msg = parse_imu_data(data[:IMU_STRUCT_SIZE])

                data = data[IMU_STRUCT_SIZE:]

            elif data[:len(GNSS_HEADER)] == bytes(GNSS_HEADER, 'utf-8'):
                gnss_msg = parse_gnss_data(data[:GNSS_STRUCT_SIZE])

                data = data[GNSS_STRUCT_SIZE:]


            else: # Some unknown header, move on
                data = data[1:]

            idx = data.find(b'$')