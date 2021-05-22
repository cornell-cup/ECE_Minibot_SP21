#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

//Bi-directinal SPI 
//13 SCK  (serial clock)
//12 MISO (master in slave out)
//11 MOSI (master out slave in)
//10 SS (slave select)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
//RFID module
#define PN532_IRQ   (2)  //pin 3 of the RJ12 17 (2)
#define PN532_RESET (3)  // pin 4 of the RJ12 9  (3)

//Motors
int enA=10;//ENABLE for Motor A 
int in1=4;//1A motor driver input
int in2=5;//2A motor driver input
int motorSpeedR=10;
int motorSpeedL=15;
int enB=11;//ENABLE for Motor B
int in3=6;// motor driver input
int in4=7;// motor driver input

int send_data[6];

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);


//initialize the buffer
int bufSize = 4;
char buf [4];
volatile byte pos = 0;

int tag[2][5];
int card;
int overf;

//Mario Melody
/*
  Arduino Mario Bros Tunes
  With Piezo Buzzer and PWM

  Connect the positive side of the Buzzer to pin 3,
  then the negative side to a 1k ohm resistor. Connect
  the other side of the 1 k ohm resistor to
  ground(GND) pin on the Arduino.

  by: Dipto Pratyaksa
  last updated: 31/3/13
*/

/*************************************************
 * Public Constants
 *************************************************/

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#define melodyPin 3
//Mario main theme melody
int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
};
//Mario main them tempo
int tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

void setup(){
  Serial.begin(115200);
  pinMode (MISO, OUTPUT);
  SPCR |= bit (SPE); //turn on SPI in slave mode
//  pinMode(trigPin, OUTPUT);
//  pinMode(echoPin, INPUT);
//  pinMode(LED, OUTPUT);
  pos = 0;
  //process_it = false;
 
  //turn on the interrupt
  SPI.attachInterrupt();
  
  //RFID initialization
  nfc.begin();

 uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  // configure board to read RFID tags
  nfc.SAMConfig();

  //buzzer
  pinMode(9, OUTPUT);//buzzer

  //Set all the motor control pins to outputs
  pinMode(enA,OUTPUT);
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(enB,OUTPUT);
  pinMode(in3,OUTPUT);
  pinMode(in4,OUTPUT);
 
  //Turn off motors ‐ Initial state 
  digitalWrite(in1,LOW);
  digitalWrite(in2,LOW);
  digitalWrite(in3,LOW);
  digitalWrite(in4,LOW);

   tag[0][0] = -4699;
   tag[1][0] = 1;
   tag[0][1] = -18523;
   tag[1][1] = 2;
   overf = 3;
   tag[0][2] = -22619;
   tag[1][2] = 3;
   
   tag[0][0] = 6566;
   tag[1][0] = 4;
   tag[0][1] = -22363;
   tag[1][1] = 5;
}

//SPI ISR (Interrupt Service Routine)

//ISR (SPI_STC_vect){
//  
//  byte c = SPDR; //get byte from the SPI data register
//  //detect the beginning of the buffer, do not put it in the buffer
//  if (c == '\n'){ 
//    valid = true;
//  }
//  //detect the end character
//  else if (c == '\r'){
//    valid = false;
////    buf[0] = 0;
////    buf[1] = 0;
//      pos = 0;
////  process_it = true;
//  }
//  //put data into the buffer
//  if ((valid == true) && (c != '\n') && (c != '\r')){
//    if (pos < bufSize ){  ///sizeof buffer
//    buf [pos] = c;
//    pos ++;
//  }
//  }
//}

void RFID_reading(){
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  //the RFID tag we are using has UID of 4 bytes
  uint8_t uidLength;        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  bool found_a_tag = false;
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    bool found_a_tag = true;
    //the correct order is 209, 21, 230, and 219
    //for the one with blue tag
    //19-->25
    //E0--> 224
    //71 -->113
    //7C --> 124
    Serial.println("Found a card!");
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");

    if (found_a_tag){
        for (uint8_t i=0; i < uidLength; i++) 
        {
          //Serial.print(" 0x");
          //Serial.println(uid[i], HEX);
          Serial.print("sending");
          Serial.println(uid[i],DEC);
          SPDR = uid[i];
        }
      found_a_tag = false;
    }
  }

       else
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Timed out waiting for a card");
  }

  delay(500);
  SPDR = 0;
 }

 void readTag(){
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
 
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    
    
    if (uidLength == 4){
      // We probably have a Mifare Classic card ... 
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];  
      cardid <<= 8;
      cardid |= uid[3]; 
      Serial.print("Seems to be a Mifare Classic card #");
      Serial.println(cardid);
      card=cardid;
    }
    Serial.println("");
  }
   Serial.println("is going here");
   Serial.println(card);
  
}

//void RFID_sendData(){
//  
//  boolean detector;
//  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID from tag
//  
//  //Orange post it --> indicate turning right
//  uint8_t obj1[] = {0x79, 0xC4, 0x4D, 0x50}; 
//  uint8_t obj2[] = {0x69, 0x12, 0xCF, 0x14}; //Yellow post it
//  uint8_t obj3[] = {0x19, 0xE0, 0x71, 0x7C};
//  uint8_t uidLength;
//
//  Serial.println("Sensor ready!");
//
//      detector = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
//      
//    
//    if(detector){
//      Serial.println("Found a tag!");
//      Serial.println("This is .....");
//      //1 means stop
//      if(memcmp(obj1, uid, 4) == 0){
//        Serial.println("object 1: stop");
//        Serial.println("sending 1");
//        SPDR = 1;
//        delay(10);
//      }
//      // 2 means turn right 90degrees
//      else if (memcmp(obj2, uid, 4) == 0){
//        Serial.println("Object 2: turn left");
//        Serial.println("sending 2");
//        SPDR = 2;
//        delay(10);
//      }
//      //3 means left 90degrees
//      else if (memcmp(obj3, uid, 4) == 0){
//        Serial.println("Object 3");
//        Serial.println("sending 3");
//        SPDR = 3;
//        delay(10);
//      }
//      //4 means spin around
//      else if (memcmp(obj4, uid, 4) == 0){
//        Serial.println("Object 4");
//        SPDR = 4;
//        delay(500);
//      }
//      //5 means victory sound
//      else if (memcmp(obj5, uid, 5) == 0){
//        sing();
//      }
//      
//    }
//}

void loop(){
   readTag();
   getAction();
  
  //clear the buffer when a command is executed
  
//  if (process_it){
//    pos = 0;
//    process_it = false;
//  }
}


void sing() {

    Serial.println(" 'Mario Theme'");
    int size = sizeof(melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {

      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / tempo[thisNote];

      buzz(melodyPin, melody[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);

      // stop the tone playing:
      buzz(melodyPin, 0, noteDuration);
    }
}

void buzz(int targetPin, long frequency, long length) {
  digitalWrite(13, HIGH);
  long delayValue = 1000000 / frequency / 2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }

}

void getAction(){
  Serial.println("in action");
  int i = -1;
  for(int j = 0; j<5; j++){
    if(tag[0][j] == card){
      i = j;
      Serial.println("found tag in array");
    }
  }
  if(i>-1 && i<5){
    if(tag[1][i] == 1){
      Serial.println("Stop");
    stopM();
  } else if(tag[1][i] == 2){
    Serial.println("Forward");
    goForward();
  } else if(tag[1][i] == 3){
    Serial.println("Turn Left");
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
    digitalWrite(in3,LOW);
    digitalWrite(in4,LOW);
    delay(500);
    goLeft();
    delay(500);
    goForward();
  } else if (tag[1][i] == 4){
    Serial.println("Turn Right");
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
    digitalWrite(in3,LOW);
    digitalWrite(in4,LOW);
    delay(500);
    goRight();
    delay(500);
    goForward();
  } else if(tag[1][i] == 5){
    Serial.println("Sing");
    stopM();
    //sing();
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
    digitalWrite(in3,LOW);
    digitalWrite(in4,LOW);
  }
  }
  
}

void goFA(){
  //Motor A FORWARD left
  digitalWrite(in1,HIGH);
  digitalWrite(in2,LOW);
  //Motor A speed
  analogWrite(enA, motorSpeedL);
}

void goBA(){
  //Motor A BACWARD left
  digitalWrite(in1,LOW);
  digitalWrite(in2,HIGH);
  //Motor A speed
  analogWrite(enA, motorSpeedL);
}

void goFB(){
  //Motor B FORWARD right
  digitalWrite(in3,HIGH);
  digitalWrite(in4,LOW);
  //Motor B speed
  analogWrite(enB, motorSpeedR);
}

void goBB(){
  //Motor B BACKWARD right
  digitalWrite(in3,LOW);
  digitalWrite(in4,HIGH);
  //Motor B speed
  analogWrite(enB, motorSpeedR);
}

void goForward(){
  goFA();
  goFB();
}

void goBackwards(){
  goBA();
  goBB();
}

void goRight(){
  goBB();
  goFA();
}

void goLeft(){
  goFB();
  goBA();
}

void stopM(){
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}
 
