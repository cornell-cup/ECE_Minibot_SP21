/* The script for bottom arduino on Minibot.
   Sensors and motors:
     IR sensor: J5 port, digital pin 4
     DC motors and Encoders 
          right: J6 port, (motor) digital pin 2 and 3, (encoder) analog pin A1
          left : J7 port, (motor) digital pin 8 and 5, (encoder) analog pin A2
     Ultrasonic sensor: J10 port, ????Not sure about pins
     Reflectance sensors
          right: J10 port, analog pin A3
          left : J11 port, analog pin A6
          
   Functions: move forward/back/left/right, line following 
*/

#include <SPI.h>
#include <elapsedMillis.h>
#include <QTRSensors.h>

elapsedMillis timeElapsed;

// Define constants for locomotion
/** Right motor drivers */
int motor0pin1 = 2;
int motor0pin2 = 3; // pwm pin, controls voltage signal
int pwm0 = 80; 
int isForward0 = 1; // = whether the right motor moves forward

/** Left motor drivers */
int motor1pin1 = 8; 
int motor1pin2 = 5; // pwm pin
int pwm1 = 80; 
int isForward1 = 1; // = whether the left motor moves forward

// Encoders regulate two motors to spin at same speed
// For more information, see PID algorithm on ECE documentation
int encoder0PinA = A1; // J3 motor on board
int encoder0Pos = 0; // Motor's angular position read by the encoder
int encoder0PinALast = LOW;

int encoder1PinA = A2; // J4 motor on board
int encoder1Pos = 0;
int encoder1PinALast = LOW;


int setpoint = 120; // turn rate for comparison (degrees/sec) 
double Integral0 = 0; // accumulated error with motors from desired number of turns
double Integral1 = 0; // accumulated error with motors from desired number of turns
int n = LOW;
int m = LOW;

int encoder0PrevCount = 0;
int lastSpeed0 = 0;
int encoder1PrevCount = 0;
int lastSpeed1 = 0;

double timeSec = 1; // update rate of the PID algorithm

//Precision Turning Parameters and Constants
const unsigned int DEF_LATERAL_WH_SPD = 120; //Default wheel speed for lateral movement, does not apply for turns
const unsigned int INNER_MOTOR_FULL_TURN_CT = 400; //Initial guess for how many encoder counts for the inner motor encoder in order to achieve a 360 degree turn with given Inner motor speed and Outer motor speed
const unsigned int OUTER_MOTOR_FULL_TURN_CT = 500; //Initial guess for how many encoder counts for the outer motor encoder in order to achieve a 360 degree turn with given Inner motor speed and Outer motor speed
const int INNER_MOTOR_SETPOINT = 70; //Initial guess for how target speed for inner wheel during turning, could be off, needs ECE check to see if realistic
const int OUTER_MOTOR_SETPOINT = 120; //Initial guess for how target speed for outer wheel during turning, could be off, needs ECE check to see if realistic
unsigned int DEFAULT_TURN_DEGREE = 45; //Default turn degree if not specified by user in blockly
unsigned int turnDegrees = 0; //Variable used to change turn degree if different from the default
//const unsigned double WHEEL_DISTANCE = 50; //distance between the two wheel measured in mm, to be used in speed/position control
//const unsigned double WHEEL_CIRCUMFERENCE = 14; //wheel circumference as measured in mm, all wheels are assumed equivalent
//int ANGULAR_SPEED = 15; //angular speed setpoint from center point for default turn



//PID constants
//P (proportional) is how much to adjust when turn rate is not equal to set rate. Matters most.
double kP = 0.25;//0.20 or .15
//I (integral) is how much to adjust based on accumulated error
double kI = 0.2;//0.01 or .05
//D (derivative) how quickly it deviates from set rate. Adjusts quicker for greater rates
double kD = 0.211;//0.01 or .01


// Line follow vars
QTRSensors qtr;
int set = 0;
double kPline = 0.05;
double kDline = 0.005;
const uint8_t SensorCount = 4;
uint16_t sensorValues[SensorCount];
int r2sensor = A0;
int r1sensor= A1;
int l1sensor = A2;
int l2sensor=A3;
int threshold= 60;
int lastError = 0;



// Constants to get the reflectance of a lie, adjustable to fit line in use
int left_line_refl=870;
int right_line_refl=835;

int left_line=false;
int right_line=false;


int test;
//char buff [50]; Use multiple parameters
volatile byte indx;
volatile boolean process;
int  interruptPin = 10;

int IRPin = 4; //S4 on J5
int in; 
int trigPin = 9; //J10 on board
int echoPin = A3; //this is the ADC pin
long duration,cm;


void setup() {  
  Serial.begin(115200);
  
// Locomotion
  pinMode (encoder0PinA, INPUT);
  pinMode (motor0pin1, OUTPUT);
  pinMode (motor0pin2, OUTPUT);

  pinMode (encoder1PinA, INPUT);
  pinMode (motor1pin1, OUTPUT);
  pinMode (motor1pin2, OUTPUT);

  pinMode(MISO,OUTPUT); //init spi

  pinMode(20,OUTPUT);
  pinMode(22,OUTPUT);
  
// SPI
  SPCR |= bit (SPE); // slave control register
  indx = 0; //buffer empty
  process = false;

  pinMode(interruptPin,INPUT);
  int val = digitalRead(interruptPin);

  delay(1000);
  SPI.attachInterrupt();

  
  pinMode(IRPin, INPUT); 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  test = 0;

  //lINE FOLLOW
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3}, SensorCount);
  delay(500);
  

  }

ISR (SPI_STC_vect) { //SPI Interrupt Service Routine
 // digitalWrite(3,1);
  Serial.println("entered ISR"); //debugging print line to indicate the beginning of the interrupt sequence
  //if (SPDR != c)  //if the new value does not equal the value already contained in SPDR      
  Serial.println("Value has been changed"); //debugging print line to show value has changed successfully
  byte c = SPDR; //read byte from SPI data register
  updated = c;
  Serial.print("ISR Value: ");
  Serial.println(updated); //debugging statement to check if value has been changed successfully
  // if (c == '\r') //check for the end of the word
  process = true;

  //Below is commented out until variable argument transfers are implemented from the Pi 
  /**if(indx < (sizeof(buff) - 1)){
  buff[indx++] = c;
     if(c == 'e')
      process = True;
  }
  **/ 
  
}  

/** Adjust PWM for PID algorithm */
//add specification for PWM and pins
void adjustPWM(char action = 'F') {
  //adjust set points (target wheel speed) based upon intended action
  //possible actions at this time are 'F' for forward, 'B' for reverse, 'R' for right turn, and 'L' for left turn
  if(action == 'F' || action == 'B'){
  setPointL = DEF_LATERAL_WH_SPD;
  setPointR = DEF_LATERAL_WH_SPD;
   }

   else if (action == 'R'){
  setPointL = OUTER_MOTOR_SETPOINT;
  setPointR = INNER_MOTOR_SETPOINT;
   }
   else if (action == 'L'){
    setPointL = INNER_MOTOR_SETPOINT;
  setPointR = OUTER_MOTOR_SETPOINT;
   }
  
  int speedNow0 = calculateSpeed0(); // calculate the current speed for the right motor
  int error0 = setpointR - speedNow0; // calculate the error between the current speed and the set speed
  double dError0 = ((double)speedNow0 - (double)lastSpeed0) / timeSec;
  Integral0 += (double) error0; // update integral of the error

  int speedNow1 = calculateSpeed1(); // calculate the current speed for the left motor
  int error1 = setpointL - speedNow1;
  double dError1 = ((double)speedNow1 - (double)lastSpeed1) / timeSec;
  Integral1 += (double) error1;

  // cap the integral value within 0..255
  if (Integral0 > 255) Integral0 = 255;
  else if (Integral0 < 0) Integral0 = 0;

  if (Integral1 > 255) Integral1 = 255;
  else if (Integral1 < 0) Integral1 = 0;

  // calculate the value for speed adjustments
  int adjust0 = (kP * (double)error0) + kI * Integral0 + kD * dError0;
  int adjust1 = (kP * (double)error1) + kI * Integral1 + kD * dError1;

  // update pwm values according to the moving direction
  if (isForward0 == 0) pwm0 += adjust0;
  else pwm0 -= adjust0;

  if (isForward1 == 0) pwm1 += adjust1;    
  else pwm1 -= adjust1;

  // cap the pwm values within 0..255
  if (pwm0 > 255) pwm0 = 255;
  else if (pwm0 < 0) pwm0 = 0;
  
  if (pwm1 > 255) pwm1 = 255;
  else if (pwm1 < 0) pwm1 = 0;

  // store the current speeds
  lastSpeed0 = speedNow0;
  lastSpeed1 = speedNow1;
}

/** Return the current rotational speed of right motor with encoder data. */
int calculateSpeed0() {
  int speedDetect = (encoder0Pos - encoder0PrevCount) / timeSec;
  encoder0PrevCount = encoder0Pos;
  return speedDetect;
}


/** Return the current rotational speed of left motor with encoder data. */
int calculateSpeed1() {
  int speedDetect = (encoder1Pos - encoder1PrevCount) / timeSec;
  encoder1PrevCount = encoder1Pos;
  return speedDetect;
}

//allow for using encoder counts both in the PID algorithm and the precision turning functions
void updateCounts(){
  n = digitalRead(encoder0PinA); // store the current digital signal of the encoder
    if ((encoder0PinALast == LOW) && (n == HIGH)) {
      // a switch from HIGH to LOW of the encoder signal marks rotation in 1 degree.
      encoder0Pos++;
     }
     encoder0PinALast = n; // update the last encoder signal for future comparison

    // same process for left encoder
    m = digitalRead(encoder1PinA);
    if ((encoder1PinALast == LOW) && (m == HIGH)) {
      encoder1Pos++;
     }
     encoder1PinALast = m;
}

int calculateTurnTicksI(int deg){
  return (double (INNER_MOTOR_FULL_TURN_CT) / 360) * deg; 
}

int calculateTurnTicksO(int deg){
        return (double (OUTER_MOTOR_FULL_TURN_CT) / 360) * deg;
}


/** Adjust the speed of motors with the PID algorithm. 
    Arguments: char mvmt: this is the same char used in the SPI switch function to determine whether to go foward, backward, left, right, etc
    No line follow integraion**/
void PID(char mvmt = 'F') {
  
  char movement = mvmt;
  // Adjust the rotational speeds by the calculated pwm values.
  if (isForward0 == 1) digitalWrite( motor0pin1, HIGH);
  else digitalWrite( motor0pin1, LOW);
  analogWrite( motor0pin2, pwm0);

  if (isForward1 == 1) digitalWrite( motor1pin1, HIGH);
  else digitalWrite( motor1pin1, LOW);
  analogWrite( motor1pin2, pwm1);

    // Count the degrees of rotation in 0.5 second for each motors. 
    long startTime = millis();
    while ( timeElapsed < 500 ) {
    updateCounts();
    timeElapsed = millis() - startTime;
  }

  //unsigned long CurrentTime = millis();
  //unsigned long ElapsedTime = CurrentTime - StartTime;
  adjustPWM(movement);
}


void moveForward() {  
    //low is nearby, high is far
  in = digitalRead(IRPin);
//  Serial.println(in);
  digitalWrite(trigPin,LOW);
  delayMicroseconds(30);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(30);
  pinMode(echoPin,INPUT);
  duration = pulseIn(echoPin,HIGH);
  //convert the time into distance: 29ms per cm
  cm = (duration/2)/29.1;
  Serial.println(cm);
  if (in == 1 || cm < 30){
    //stop
    digitalWrite(motor0pin2, HIGH);//1 high 2 low is clockwise
    digitalWrite(motor0pin1, LOW);
    digitalWrite(motor1pin2, HIGH);
    digitalWrite(motor1pin1, LOW);
    delay(400);
    digitalWrite(motor0pin2, LOW);
    digitalWrite(motor1pin2, LOW);
    delay(1000);
//    Serial.println("stop");
  }
  else {
    //move
    isForward0 = 1;
    isForward1 = 1;
    PID('F');
//    digitalWrite(motor0pin2, LOW);//1 high 2 low is clockwise
//    digitalWrite(motor0pin1, HIGH);
//    digitalWrite(motor1pin2, LOW);
//    digitalWrite(motor1pin1, HIGH);
    //delay(400);
//    Serial.println("move");
  }
  //check again
   if (in == 1 || cm < 10){
   if( in == 1)
//    Serial.println("IR detects");
    //stop
    digitalWrite(motor0pin2, HIGH);//1 high 2 low is clockwise
    digitalWrite(motor0pin1, LOW);
    digitalWrite(motor1pin2, HIGH);
    digitalWrite(motor1pin1, LOW);
    delay(400);
    digitalWrite(motor0pin2, LOW);
    digitalWrite(motor1pin2, LOW);
    delay(1000);
//    Serial.println("stop");
  } 
}

//***Line follow functions***
void stop(){
  digitalWrite(motor0pin1,LOW);
  digitalWrite(motor1pin1,LOW);
  analogWrite(motor0pin2,0);
  analogWrite(motor1pin2, 0);
}
void veer_right(){
  digitalWrite(motor0pin1,HIGH);
  digitalWrite(motor1pin1,HIGH);
  analogWrite(motor0pin2,140);
  analogWrite(motor1pin2, 50);
}

void veer_left(){
  digitalWrite(motor0pin1,HIGH);
  digitalWrite(motor1pin1,HIGH);
  analogWrite(motor0pin2,50);
  analogWrite(motor1pin2, 140);
}
 void move(){
  digitalWrite(motor0pin1,HIGH);
  digitalWrite(motor1pin1,HIGH);
  analogWrite(motor0pin2,80+motorSpeed);
  analogWrite(motor1pin2, 80 + motorSpeed);
 }

void LineFollow() {
  if (set == 0) {
   Serial.println("Begin calibrations");

  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
  // Call calibrate() 400 times to make calibration take about 10 seconds.
  
  for (uint16_t i = 0; i < 150; i++)
  {
    qtr.calibrate();
  }
  veer_right();
  delay(40);
  for (uint16_t i = 0; i < 150; i++)
  {
    qtr.calibrate();
  }
  veer_left();
  delay(80);
   for (uint16_t i = 0; i < 150; i++)
  {
    qtr.calibrate();
  }
  
  // print the calibration minimum values measured when emitters were on
  Serial.println("printing minimum");
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(' ');
  }
  Serial.println();
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();
  delay(100);
  
    set = 1;
  }
 
    uint16_t position = qtr.readLineBlack(sensorValues);
    int error = position - 1500;
    double motorSpeed = KP * error + KD *(error-lastError);//change in PWM(negative if turning left, positive if turning right)
    lastError = error;
    if(motorSpeed>60){
    motorSpeed=60;
    }
    else if(motorSpeed<-60){
    motorSpeed=-60;
    }
    move(motorSpeed);
    delay(20);
 
}
//line follow functions


void loop() {
   
  if (process) {  //process is true if interrupt is received this loop used to reset interrupt
    process = false; //reset flag
    // Serial.println("work");
   
    Serial.println(updated); //debugging statement prints the value of updated for inspection
     
    switch(updated) { //function changes the letter value of updated to a command
      case 'F' : //fwd
        // Serial.println("moving forward");
        moveForward(); //runs move forward function from above
        set = 0; //sets up for line follow mode
        //delay(6000);
        break; //breaks out of the switch loop and continues the original search
          
      case 'B' : //Backwards (back())
        // Serial.println("back");
        isForward0 = 0;
        isForward1 = 0;
        set = 0;
        PID('B');
        // delay(6000);
        break; //breaks out of the switch loop and continues the original search
          
      case 'L' : //left
        // Serial.println("Left");
        //Below shows how we might be able to add an additional aommand passed over spi to tell the robot how many degrees to turn
  /**if(buff[1] == 0)
    turnDegrees = DEF_TURN_DEGREE;
    else 
      turnDegrees = buff[1];
  **/
  turnDegrees = DEF_TURN_DEGREE;
  digitalWrite(motor0pin2, LOW); //right motor, forward
        digitalWrite(motor0pin1, HIGH); 
        digitalWrite(motor1pin2, HIGH); //left motor, backward
        digitalWrite(motor1pin1, LOW); 
        set = 0;
  PID('L');
  while((encoder0Pos < calculateTurnTicksO(turnDegrees)) && (encoder1Pos < calculateTurnTicksI(turnDegrees))){
    updateCounts();
  }
  //stop the motors
  digitalWrite(motor0pin2, LOW);
        digitalWrite(motor0pin1, LOW);
        digitalWrite(motor1pin2, LOW);
        digitalWrite(motor1pin1, LOW);
  
        delay(6000);
        break; //breaks out of the switch loop and continues the original search
          
      case 'R' : //right
        // Serial.println("Right");
  //Below shows how we might be able to add an additional aommand passed over spi to tell the robot how many degrees to turn
        /**if(buff[1] == 0)
                turnDegrees = DEF_TURN_DEGREE;
          else
                turnDegrees = buff[1];
        **/
  turnDegrees = DEF_TURN_DEGREE;
        digitalWrite(motor0pin2, HIGH);//right motor, backward
        digitalWrite(motor0pin1, LOW);
        digitalWrite(motor1pin2, LOW);//left motor, forward
        digitalWrite(motor1pin1, HIGH); 
        set = 0;
  PID('R');
  //
  while((encoder0Pos < calculateTurnTicksI(turnDegrees)) && (encoder1Pos < calculateTurnTicksO(turnDegrees)){
                updateCounts();
        }
  //stop the motors
        digitalWrite(motor0pin2, LOW);
        digitalWrite(motor0pin1, LOW);
        digitalWrite(motor1pin2, LOW);
        digitalWrite(motor1pin1, LOW);
        delay(6000);
        break; //breaks out of the switch loop and continues the original search
          
      case 'S' : //stop, makes all pins low
        digitalWrite(motor0pin2, LOW);
        digitalWrite(motor0pin1, LOW);
        digitalWrite(motor1pin2, LOW);
        digitalWrite(motor1pin1, LOW);
        //delay(6000);
        set = 0;
        break; //breaks out of the switch loop and continues the original search
          
      case 'T' : //Line Follow mode
        LineFollow(); //starts line follow 
        break; //breaks out of the switch loop and continues the original search
          
      default: //code run when none of the cases are met
        set = 0;
        break; //breaks out of the switch loop and continues the original search
    }
  }
  //TODO Case where switching functions 
    //SPDR = data  //sends value to master via SPDR
    //indx = 0; //reset button to zero
}
