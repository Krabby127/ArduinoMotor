/* ECEN 2270 motor speed control */

#include <interrupt.h>
#include <io.h>


//const float TURN_MAX = 3584;
const float TURN_MAX = 1792;    // The supposed number of encoder pulses that we need to travel

// define pins
const int encoder_Left = 2;     // counts the left wheel's encoder pulses
const int encoder_Right = 3;    // counts the right wheel's encoder pulses
const int pinON = 6;            // connect pin 6 to ON/OFF switch, active HIGH
const int pinCW_Left = 7;       // connect pin 7 to clock-wise first PMOS gate
const int pinCC_Left = 8;       // connect pin 8 to counter-clock-wise first PMOS gate
const int pinSpeed_Left = 9;    // connect pin 9 to the right speed control
const int pinSpeed_Right = 10;  // connect pin 10 to the left speed control
const int pinCW_Right = 11;     // connect pin 11 to clock-wise second PMOS gate
const int pinCC_Right = 12;     // connect pin 12 to counter-clock-wise second PMOS gate



volatile int count_Left = 0;               // encode count defined volatile to be used in ISR
volatile int count_Right = 0;              // encode right count defined volatile to be used in ISR

int stepNumber = 1;
//Steps to do for robot command
// 1: wait 1 second
// 2: spin 360 CC
// 3: slow down near end
// 4: stop and wait 1 second
// 5: spin 360 CW
// 6: slow down near end
// 7: stop and end (infinite loop)



void counter_Left() {
  // ISR for left counter
  count_Left++;
}

void counter_Right() {
  // ISR for right counter
  count_Right++;
}

void preciseRobotStraight(float distance, int wheel_speed, boolean wheel_Direction)
{ // Slows down at 80%
  Serial.println("The robot is now going in a 'precise' straight line.\n");
  int step = 1;
  long target = encoderDistance(distance);
  switch (step)
  {
  case 1: // The wheel is going at normal speed until 80% distance
    while ((count_Left <= (target * 0.8)) && (count_Right <= (target * 0.8)))
    {
      wheel_go(wheel_speed, 'B', wheel_Direction);
    }
    break;
  case 2: // Wheel is going slower now
    while ((count_Left <= target) && (count_Right <= target))
    {
      slow_Down('B');
    }
    break;
  default:
    Serial.println("\n\n\n\n\nSomething happened\n\n\n\n\n");
    digitalWrite(13, HIGH); // turn on LED if something went wrong
    all_Stop();
    break;
  }
}

void robotStraight(float distance, int wheel_speed)
{
  long target = encoderDistance(distance); // Target should be the number of encoder pulses needed to travel a 'distance' distance
  while ((count_Left <= target) && (count_Right <= target))
  {
    wheel_go(wheel_speed,'B',true);
  }
  Serial.println("The count has been reached");
  all_Stop();
  reset();
  return;
}

void wheel_go(int wheel_speed, char Wheel, boolean wheel_Direction) {
  // sets the given wheel at the wheel_speed with analogWrite()
  // true for clockwise
  // false for counterclockwise

  switch (Wheel)
  {
  case 'L':
  case 'l':
    if (wheel_Direction) {
      // Make wheel go clockwise
      digitalWrite(pinCC_Left, LOW);
      digitalWrite(pinCW_Left, HIGH);
    }
    if (!wheel_Direction) {
      // Make wheel go counter-clockwise
      digitalWrite(pinCW_Left, LOW);
      digitalWrite(pinCC_Left, HIGH);
    }
    analogWrite(pinSpeed_Left,  0.75*wheel_speed); // actually sets the wheel speed
    break;

  case 'R':
  case 'r':
    if (wheel_Direction) { // TRUE wheel_Direction is clockwise
      // Make wheel go clockwise
      digitalWrite(pinCC_Right, LOW); // Turn off counter-clockwise right
      digitalWrite(pinCW_Right, HIGH); // Turn on clockwise right
    }
    if (!wheel_Direction) {
      // Make wheel go counter-clockwise
      digitalWrite(pinCW_Right, LOW); // Turn off clockwise right
      digitalWrite(pinCC_Right, HIGH); // Turn on counter-clockwise right
    }
    analogWrite(pinSpeed_Right, wheel_speed);
    break;
  case 'B': // User enters 'B'
  case 'b': // User can also enter 'b'
    if (wheel_Direction) { // TRUE wheel_Direction is clockwise
      // Make both wheels go clockwise
      digitalWrite(pinCC_Right, LOW); // Turn off counter-clockwise right
      digitalWrite(pinCC_Left, LOW); // Turn off counter-clockwise left
      digitalWrite(pinCW_Right, HIGH); // Turn on clockwise right
      digitalWrite(pinCW_Left, HIGH); // Turn on clockwise left
    }
    if (!wheel_Direction) { // FALSE wheel_Direction is counter-clockwise
      // Make both wheels go clockwise
      digitalWrite(pinCW_Right, LOW); // Turn off clockwise right
      digitalWrite(pinCW_Left, LOW); // Turn off clockwise left
      digitalWrite(pinCC_Right, HIGH); // Turn on counter-clockwise right
      digitalWrite(pinCC_Left, HIGH); // Turn on counter-clockwise left
    }
    analogWrite(pinSpeed_Left, wheel_speed)); // actually sets the wheel speed
    analogWrite(pinSpeed_Right, wheel_speed); // actually sets the wheel speed
    break;

  default: // Should only reach here if coder error
    Serial.println("\n\n\n\n\nSomething happened\n\n\n\n\n");
    digitalWrite(13, HIGH); // turn on LED if something went wrong
    all_Stop();
    break;
  }

}

void reset(){
  count_Left=0;
  count_Right=0; 
}

void slow_Down(char Wheel) {
  switch (Wheel) // Choose which wheel to slow down
  {
  case 'L':
  case 'l':
    analogWrite(pinSpeed_Left, 26);
    break;

  case 'R':
  case 'r':
    analogWrite(pinSpeed_Right, 26);
    break;

  case 'B':
  case 'b':
    analogWrite(pinSpeed_Left, 26);
    analogWrite(pinSpeed_Right, 26);
    break;

  default:
    digitalWrite(13, HIGH); // turn on LED if something went wrong
    Serial.println("\n\n\n\n\nSomething happened\n\n\n\n\n");
    all_Stop();
    break;

  }
}



void stop_Wheel(char Wheel) {

  // User gives which wheel to stop as a char
  switch (Wheel)
  {
  case 'L': // User specifies left wheel
  case 'l':
    digitalWrite(pinCC_Left, LOW);
    digitalWrite(pinCW_Left, LOW);
    break;

  case 'R': // User specifies right wheel
  case 'r':
    digitalWrite(pinCC_Right, LOW);
    digitalWrite(pinCW_Right, LOW);
    break;

  case 'B': // User specifies both wheels
  case 'b':
    all_Stop();
    break;

  default : // default is to stop both
    Serial.println("\n\n\n\n\nSomething happened\n\n\n\n\n");
    all_Stop();
    digitalWrite(13, HIGH); // turn on LED if something went wrong
    break;

  }
}

void all_Stop() {
  // stop all wheels
  analogWrite(pinSpeed_Right, 0);
  analogWrite(pinSpeed_Left, 0);
  digitalWrite(pinCC_Left, LOW);
  digitalWrite(pinCC_Right, LOW);
  digitalWrite(pinCW_Left, LOW);
  digitalWrite(pinCW_Right, LOW);
}

long encoderDistance(float realDistance)
{
  // Use some formula to convert encoder pulses to distance
  return ((realDistance*13*64) / (TWO_PI*0.065));
}

void leftTurn()
{
  stepNumber=1;
  boolean finished=false;
  while(!finished){
    switch (stepNumber)
    {
    case 1:
      Serial.println("Now at Step 1:\n");
      delay(1000);
      stepNumber = 2;
      break;
    case 2:
      Serial.println("Now at Step 2:\n");
      wheel_go(255, 'L', false); // Start wheel at max speed counterclockwise
      wheel_go(255,'R',true);
      if ((count_Left >= TURN_MAX)&&(count_Right>=TURN_MAX))stepNumber = 3; // Slow down when near target
      break;
    case 3:
      Serial.println("Now at Step 3:\n");
      all_Stop();
      //DDRB=0x00;
      delay(1000);
      count_Left = 0; // reset counter
      count_Right = 0;
      finished=true;
      break;

    }
  }

}

void rightTurn()
{
  stepNumber=1;
  boolean finished=false;
  while(!finished){
    switch (stepNumber)
    {
    case 1:
      Serial.println("Now at Step 1:\n");
      delay(1000);
      stepNumber = 2;
      break;
    case 2:
      Serial.println("Now at Step 2:\n");
      wheel_go(255, 'L', true); // Start wheel at max speed counterclockwise
      wheel_go(255,'R',false);
      if ((count_Left >= TURN_MAX)&&(count_Right>=TURN_MAX))stepNumber = 3; // Slow down when near target
      break;
    case 3:
      Serial.println("Now at Step 3:\n");
      all_Stop();
      //DDRB=0x00;
      delay(1000);
      count_Left = 0; // reset counter
      count_Right = 0;
      finished=true;
      break;

    }
  }

}



