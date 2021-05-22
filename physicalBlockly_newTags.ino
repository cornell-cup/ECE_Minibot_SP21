#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>



// If using the breakout with SPI, define the pins for SPI communication.
//#define PN532_SCK  (2)
//#define PN532_MOSI (3)
//#define PN532_SS   (4)
//#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a SPI connection:
//Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
int LED =9;
int tag[2][5];
int card;
int overf;

void setup(void) {
  pinMode(LED,OUTPUT);
  
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  Serial.println("Hello!");

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
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
  
  
   tag[0][2] = -18523;
   tag[1][2] = 3;
   overf = 3;
   tag[0][0] = 0;
   tag[1][0] = 0;
   tag[0][1] = 0;
   tag[1][1] = 0;
   tag[0][0] = -4699;
   tag[1][0] = 1;
   tag[0][1] = -22619;
   tag[1][1] = 2;
  
 }



void loop(void) {
    for(int j = 0; j<5; j++){
      Serial.println(tag[0][j]);
      Serial.println(tag[1][j]);
    }

  Serial.println("Hello! Please choose what you want to do \n \t1. Read \n \t2. Write ");
  int key1 = 0;
  while(key1==0){
    key1=Serial.parseInt();
  }
  Serial.print("Reading... ");
  Serial.println(key1);
  if(key1==1){ 
    Serial.println("Press 9 to exit");
    int i = 0;
    while(i!=9){
      readTag();
      getAction();
      i=Serial.parseInt();
      delay(1000);
    }
  }else if (key1==2){
    Serial.println("Press 9 to exit");
    int i = 0;
    writeTag();
  }
}

void writeTag(){
  int a = 0; 
  int b;
  Serial.println("Choose an action: \n\t1. Turn off\n\t2. Turn on\n\t3.Blink three times\n\t4. Blink 5 times\n\t5.Blink leave on");
  while(a==0){
      a = Serial.parseInt();
      b = a;
  }
  Serial.println("Tap tag");
  readTag();
  delay(3000);
  int j;
  boolean used = false;
  for(int i = 0;i<5; i++){
    if(card == tag[0][i]){
      used = true;
      j = i;
    }
  }
  if(used){
      tag[1][j]=b;
  } else {
    tag[0][overf]=card;
    tag[1][overf] = b;
    overf++;
    if(overf==5){
      overf = 0;
      }
   }
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
void blinky(){
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(500); 
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
    digitalWrite(LED,HIGH);
  } else if(tag[1][i] == 2){
    digitalWrite(LED,LOW);
  } else if(tag[1][i] == 3){
    blinky();
    blinky();
    blinky();
  } else if (tag[1][i] == 4){
    blinky();
    blinky();
    blinky();
    blinky();
    blinky();
  } else if(tag[1][i] == 5){
    digitalWrite(LED,HIGH);
    delay(500);
    digitalWrite(LED,LOW);
    delay(400);
    digitalWrite(LED,HIGH);
  }
  }
  
}


/* writing plan for how to make it more interactive, ask for some input if anything,
 *  create like a little cute interface where the person can choose to either write or read a tag
 *  have an array of arrays (QUESTION MARK) (IS IT POSSIBLE TO HAVE 2D ARRAYS ON ARDUINOS)
 *  ANYWHO have a variable constantly checking if there is enough space to write a new one or if there is need to 
 *  rewrite a current one, give some options but assume this would be have to be implemented with the cs interface
 *  maybe have prewirtten one if anything 
 *  if writing then choose one of the empty array thingys put the tag (check tag is not being used in another one)
 *  save the tag and give it a purpose (have to really think how to implement this HAVE TO UNDERSTAND HOW CHLOE WAS
 *  WORKING WITH THIS BEFORE), if the tag is being already used or the array is full then ask if want to rewite
 *  if just reading then it should be fine QUESTION MARK?????
 */
