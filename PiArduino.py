import binascii
import spidev
import time
import RPi.GPIO as GPIO

spi = spidev.SpiDev()
GPIO.setmode(GPIO.BOARD)
selectPin1 = 7
selectPin2 = 11

#set which arduino to talk to. slave(0) for arduino 1 and slave(1) for arduino 2
def setSlave(PiBus):
  device = 0
  bus = PiBus
  spi.open(device,bus)
  spi.mode = 0
  spi.max_speed_hz = 115200

def transmit(message):
  try:
    while True:
      print (message)
      tx = spi.writebytes([message])
      time.sleep(0.2)
  finally:
    tx = spi.writebytes([ord('S')])
    spi.close()

def setupGPIO():
  GPIO.setup(selectPin1, GPIO.OUT)
  GPIO.setup(selectPin2,GPIO.OUT)
  GPIO.setup(24, GPIO.OUT)

  GPIO.output(24,0)
def fwd(pin1,pin2):
  setSlave(1)
  GPIO.output(selectPin1,pin1)
  GPIO.output(selectPin2,pin2)
  cmd = ord('F')
  #print b
  print (cmd)
  transmit(cmd)
  #cmd = ord(param) -- do some math on the param to separate different speeds.
  #Maybe >100 one speed <100 another set speed

def back():
  setSlave(1)
  cmd = ord('B')
  #print b
  print (cmd)
  transmit(cmd)

def left():
  setSlave(1)
  cmd = ord('L')
  #print b
  print (cmd)
  transmit(cmd)

def right():
  setSlave(1)
  cmd = ord('R')
  #print b
  print (cmd)
  transmit(cmd)

def stop():
  setSlave(1)
  cmd = ord('S')
  #print b
  print (cmd)
  transmit(cmd)

def LineFollow():
  setSlave(1)
  cmd = ord('T') #for tape follow
  #print b
  print (cmd)
  transmit(cmd)

def ObjectDetection():
  setSlave(0)
  cmd = ord('O')
  #print b
  print (cmd)
  transmit(cmd)
