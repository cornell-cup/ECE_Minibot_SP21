#include <SPI.h>
#include <Servo.h>

//Bi-directinal SPI 
//13 SCK  (serial clock)
//12 MISO (master in slave out)
//11 MOSI (master out slave in)
//10 SS (slave select)
//send string of Hello to Raspberry Pi

//initializing variables
Servo myservo;
int bufSize = 4;
char buf [4];
volatile byte pos = 0;
volatile boolean process_it;
//ultrasonic setup
int trigPin = A2;//A0;//A1; //digital pin
int echoPin = 5;//6;//3; //analog pin
int infra;
int line;
int encoderPin;
int motorpin1;
int motorpin2;
long duration,cm;
int n = 0;
bool valid = false;
String x = "";
int LED = 5;


void setup() {
  Serial.begin(9600);
  pinMode (MISO, OUTPUT); //output because sensor output
  SPCR |= bit (SPE); //turn on SPI in slave mode
  pinMode(LED, OUTPUT);
  pos = 0;
  process_it = false;
  //turn on the interrupt
  SPI.attachInterrupt();

  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

//SPI ISR (Interrupt Service Routine)

ISR (SPI_STC_vect){ //pi/arduino communication, this is they key portion of bidirectional spi
  
  byte c = SPDR; //get byte from the SPI data register
  if (c == '\n'){ //the beginning of the data
    valid = true; //data can start being read, if false, data is useless
  }
  else if (c == '\r'){
    valid = false; //end of character, indicates end of data so stop reading
    process_it = true; //done with this process
  }
  if ((valid == true) && (c != '\n')){
    if (pos < bufSize ){  ///sizeof buf
    buf [pos] = c;
    pos ++;
  }
  }
  //end of room available
}
  

//function for reading distance data 
void get_distance(){
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  pinMode(echoPin,INPUT);
  duration = pulseIn(echoPin,HIGH);
  cm = (duration/2)/29.1;
  Serial.println(cm);
  delay(100);
}

//function for sending distance data to Pi
void bi_directional(){
  if (buf[0] == 'd'){
    if (buf[1] == 'u'){ //array du is a call that information needs to be collected
      get_distance();
      SPDR = cm; //setting data into the register to be processed
    }
  }
}

//setting appropriate port based on commands
//refer to documentation for encoded commands explanations
void set_ports(){ 
    if (buf[0] == '2'){
    //right motor
      if ((buf[1] == 'L') && (buf[2] == 'M') ){
          encoderPin = A0;
          motorpin1 = 4;
          motorpin2 = 6;
          pinMode(motorpin1, OUTPUT);
          pinMode(motorpin2, OUTPUT);
          pinMode(encoderPin, INPUT);        
    }
    //right motor
      else if ((buf[1] == 'L') && (buf[2] == 'M')){
          encoderPin = A0;
          motorpin1 = 4;
          motorpin2 = 6;
          pinMode(motorpin1, OUTPUT);
          pinMode(motorpin2, OUTPUT);
          pinMode(encoderPin, INPUT);      
    }
    //ultrasonic
      else if (buf[1] == 'U'){
        echoPin = A0;
        trigPin = 4;
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
        Serial.println("Port 2 has been set up for ultrasonics sensor");
    }
    //infrared sensor
      else if (buf[1] == 'I'){
        infra = 4;
        pinMode(infra, INPUT);
      
  }
      else if (buf[1] == 'R'){
      
    }
    //Line Sensors
      else if (buf[1] == 'L'){
        line = A0;
        pinMode(line, INPUT);
      
    }
  }
  
 
else if (buf[0] == '3'){
    //left motor
    if ((buf[1] == 'L') && (buf[2] == 'M') ){
      encoderPin = A1;
      motorpin1 = 2;
      motorpin2 = 3;
      pinMode(motorpin1, OUTPUT);
      pinMode(motorpin2, OUTPUT);
      pinMode(encoderPin, INPUT); 
    }
    //right motor
    else if ((buf[1] == 'R') && (buf[2] == 'M')){
      encoderPin = A1;
      motorpin1 = 2;
      motorpin2 = 3;
      pinMode(motorpin1, OUTPUT);
      pinMode(motorpin2, OUTPUT);
      pinMode(encoderPin, INPUT); 
    }
    //ultrasonic
    else if (buf[1] == 'U'){
        echoPin = A1;
        trigPin = 3;
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
        Serial.println("Port 3 has been set up for ultrasonic sensor");
    }
    //infrared sensor
    else if (buf[1] == 'I'){
        infra = 7;
        pinMode(infra, INPUT);
      
  }
    else if (buf[1] == 'R'){
      
    }
    //Line Sensors
    else if (buf[1] == 'L'){
        line = A1;
        pinMode(line, INPUT);
      
    }
  }
  if (buf[0] == '4'){
    //right motor
    if ((buf[1] == 'L') && (buf[2] == 'M') ){
          encoderPin = A2;
          motorpin1 = 8;
          motorpin2 = 5; //pwm pins
          pinMode(motorpin1, OUTPUT);
          pinMode(motorpin2, OUTPUT);
          pinMode(encoderPin, INPUT);       
    }
    //right motor
    else if ((buf[1] == 'L') && (buf[2] == 'M')){
          encoderPin = A2;
          motorpin1 = 8;
          motorpin2 = 5; //pwm pins
          pinMode(motorpin1, OUTPUT);
          pinMode(motorpin2, OUTPUT);
          pinMode(encoderPin, INPUT);       
    }
    //ultrasonic
    else if (buf[1] == 'U'){
        echoPin = A2;
        trigPin = 5;
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }
    //infrared sensor
    else if (buf[1] == 'I'){
        infra = 8;
        pinMode(infra, INPUT);    
  }
    else if (buf[1] == 'R'){

      
    }
    //Line Sensors
    else if (buf[1] == 'L'){
        line = A2;
        pinMode(line, INPUT);
      
    }
  }
  if (buf[0] == '8'){
    //ultrasonic
    if (buf[1] == 'U'){
        echoPin = A3;
        trigPin = 9;
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }
    //infrared sensor
    else if (buf[1] == 'I'){
        infra = 9;
        pinMode(infra, INPUT);
      
  }
    else if (buf[1] == 'R'){
      
    }
    //Line Sensors
    else if (buf[1] == 'L'){
        line = A3;
        pinMode(infra, INPUT);
      
    }
  }
  else if (buf[0] == '9'){
    if (buf[1] == 'L'){
        line = A5;
        pinMode(line, INPUT);
  }
 }
}
void loop() {  
//    set_ports();
//
//  if (process_it){
//    pos = 0; //resets everything, buffer becomes zero 
//    process_it = false;//resets to before data is processed
//  }
  //Serial.println("loop");
  
  get_distance();
}
