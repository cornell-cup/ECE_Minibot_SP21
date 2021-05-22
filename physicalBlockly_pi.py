# File: physicalBlockly_pi
import time

import spidev



cs0 = 0

cs1 = 1



pi = spidev.SpiDev()

# spi.open(bus, device)

# cs1 is RFID

# cs0 is motor

stop_motor = False

right = False

left = False
driveForward = False

tx = []

x = []

stop_time = time.time() + 200



def send_data(data):

    global tx

    pi.open(0, cs0)

    pi.mode = 0

    pi.max_speed_hz = 115200

    for i in range(len(data)):

        tx = pi.writebytes([ord(data[i])])  # send a single character

    pi.close()





def check_input():

    global x

    global stop_motor

    global right

    global left

    global driveForward

    pi.open(0, cs1)

    pi.mode = 0

    pi.max_speed_hz = 115200

    x = pi.readbytes(1)

    if x == [1]:

        stop_motor = True

        right = False

        left = False

	driveForward = False

    if x == [2]:

        stop_motor = False

        right = True

        left = False

	driveForward = False

    if x == [3]:

        stop_motor = False

        right = False

        left = True

	driveForward = False

    if x == [3]:

        stop_motor = False

        right = False

        left = False

	driveForward = True

    pi.close()





while (time.time() < stop_time):

    data = ['\n','s','\r']

    send_data(data)

    check_input()

    #when stop is detected

    while stop_motor and not right and not left and not driveFoward:

        print("stop is received")

        data = ['\n','s','\r']

        send_data(data)

        check_input()

    while right and not stop_motor and not left and not driveForward:

        print("right is received")

        data = ['\n','r','\r']

        send_data(data)

        check_input()

    while left and not stop_motor and not right and not driveForward:

        print("left is received")

        data = ['\n','l','\r']

        send_data(data)

        check_input()

    while driveForward and not stop_motor and not right and not left:

        print("Forward is received")

        data = ['\n','f','\r']

        send_data(data)

        check_input()

