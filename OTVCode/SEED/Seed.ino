#include "Enes100.h"
#include <math.h>
#include <Servo.h>

//drive constants
double kpRot = 10;           //5
double kpRotScale = 5;       //4
double kpTurn = 4;           //2.8
double kpLin = 700;          //255
double stopThreshold = .05;  //0.01
double angleThreshold = 10;  //5

//mission constants
double dx = 0.23;
double dy = 0.1;
double ds = 0.2;

// default speed
int default_speed = 50;

// motor a - Forward & Backward
int motorAF = 2;
int motorAB = 3;
int motorA_PWM = 5;

//motor b - Forward & Backward
int motorBF = 4;
int motorBB = 7;
int motorB_PWM = 6;

//servo setup
Servo myservo;
int servoPin = 10;
int servoNum;

// mission specific values
boolean dropped, TL, TR, BL, BR;

// //Motor Functions
// void motorStop();
// void turnLeft();
// void turnRight();
// void reverse();
// void forward();

// //function declarations
// double getDistance(double x, double y);
// void goStraightTo(double x, double y);
// double getAngle(double x, double y);
// void drive(int left, int right);
// double getErrorTheta(double x, double y);
// void mission(double x, double y, int dir);
// void goTo(double x, double y);
// void goToTheta(double theta);
// void alongX(double x);
// void alongY(double y);

// // mission functions
// void missionSetup();
// void runMission();
// void assignTL();
// void endMission();
// boolean checkCell();

void setup() {
  //Pins for H-Bridge / Motor Control
  pinMode(motorAF, OUTPUT);
  pinMode(motorAB, OUTPUT);
  pinMode(motorBF, OUTPUT);
  pinMode(motorBB, OUTPUT);
  myservo.attach(servoPin);  //attaches servo on pin 10 to the servo object
  delay(1000);
  beanReset();
  delay(1000);

  //9, 8
  Enes100.begin("BEAN", SEED, 49, 11, 12); 
  Enes100.println("successfully began WiFi Module");
  
  goTo(1.3, 0.64);

  runMission();
  endMission();
}

void justNav() { // 
  delay(1000);
  goTo(1.3, 0.64);
  delay(1000);
  goTo(1.78, 0.62);
  delay(1000);
  alongX(1.3);
  delay(1000);
  goTo(1.5, 0.64);
  delay(1000);
  goTo(1.68, 0.62);
  delay(1000);
  alongX(1.5);
  delay(1000);
  goTo(1.7, 0.3);
  Enes100.println("Here");
  delay(5000);
  alongY(0.6);
  delay(1000);
  alongY(0.85);
}

void loop() {
}

void runMissionNoML() { //
  // bottom right cell actions
  delay(1000);
  goTo(1.78, 0.62);
  delay(1000);
  TR = 0;  // check TR cell
  dropBean(true);
  delay(1000);
  //intermediate steps
  alongX(1.3);
  delay(250);
  goTo(1.5, 0.64);
  delay(250);
  // read top right cell actions
  goTo(1.68, 0.62);  // go to TR read position
  delay(1000);
  BR = 0;  // check TR cell
  delay(1000);
  //intermediate steps
  alongX(1.5);
  delay(1000);
  // perform bottom left cell  actions if applicable
  goTo(1.6, 0.4);
  delay(250);
  alongY(0.6);
  delay(250);
  alongY(0.85);
  delay(1000);
  BL = 1;
}

void runMission() {

  // bottom right cell actions
  delay(1000);
  goTo(1.78, 0.62);
  delay(1000);
  Enes100.begin("SeedSummer3", SEED, 49, 9, 8);
  delay(1000);
  TR = checkCell(true);  // check TR cell
  delay(1000);

  //intermediate steps
  Enes100.begin("BEAN", SEED, 49, 11, 12); 
  alongX(1.3);
  delay(250);
  goTo(1.5, 0.64);
  delay(250);

  // read top right cell actions
  goTo(1.68, 0.62);  // go to TR read position
  delay(1000);
  Enes100.begin("SeedSummer3", SEED, 49, 9, 8);
  delay(1000);
  BR = checkCell(true);  // check TR cell
  delay(1000);

  //intermediate steps
  Enes100.begin("BEAN", SEED, 49, 11, 12); 
  alongX(1.5);
  delay(1000);

  // perform bottom left cell  actions if applicable
  if (TR == BR) {
    if (dropped) {
      BL = 1;
    } else {
      goTo(1.6, 0.4);
      delay(250);
      alongY(0.6);
      delay(250);
      alongY(0.85);
      delay(1000);
      BL = 0;
      dropBean(false);
    }
  } else {
    goTo(1.6, 0.4);
    delay(250);
    alongY(0.6);
    delay(250);
    alongY(0.85);
    delay(1000);

    Enes100.begin("SeedSummer3", SEED, 49, 9, 8);
    delay(1000);
    BL = checkCell(false);

  }
}

void assignTL() {
  if (BR == TR) {   // right match
    if (BR == 0) {  // right are 0
      TL = 1;
    } else {  // right is 1
      TL = 0;
    }
  } else if (BR == BL) {  // bottom match
    if (BR == 0) {        // bottom are 0
      TL = 1;
    } else {
      TL = 0;
    }
  } else {  // mismatches
    TL = BR;
  }
}

void endMission() {
  assignTL();
  Enes100.println("Plot Layout:");
  Enes100.println(TL + " " + TR);
  Enes100.println(BL + " " + BR);
}

// checks if a cell is rocks or substrate. if it is rocks then return 1. if it is substrate then drop bean if it hasnt been dropped yet and return 0.
boolean checkCell(boolean rightSide) {
  if (Enes100.MLGetPrediction() == 0) {  // if cell is substrate approve drop
    if (!dropped) {                      // check if bean is already dropped
      dropBean(rightSide);
      dropped = true;
    }
    Enes100.println("substrate");
    return 0;
  } else {
    Enes100.println("rocks");
    return 1;
  }
}

void goTo(double x, double y) {
  double errorDistance = getDistance(x, y);
  double accumulate = 0;
  while (errorDistance > stopThreshold) {
    Enes100.updateLocation();
    if (errorDistance < 0.15) {
      accumulate += 0.001;
    }
    if (abs(getErrorTheta(x, y)) > angleThreshold) {
      drive(getErrorTheta(x, y) * kpTurn, getErrorTheta(x, y) * -kpTurn);
      accumulate = 0;
    } else {
      goStraightTo(x, y, getErrorTheta(x, y), errorDistance, accumulate);
    }
    errorDistance = getDistance(x, y);
  }
  drive(0, 0);
  Enes100.println(Enes100.location.x);
  Enes100.println(Enes100.location.y);
  Enes100.println("");
}

void goStraightTo(double x, double y, double errorTheta, double errorLinear, double integral) {
  //off left: +
  //off right: -
  drive(30 + kpLin * errorLinear + errorTheta * kpRot * (errorLinear * kpRotScale) + integral, 30 + kpLin * errorLinear - errorTheta * kpRot * (errorLinear * kpRotScale) + integral);
}

//returns distance to a point in meters
double getDistance(double x, double y) {
  double diffX = x - Enes100.location.x;
  double diffY = y - Enes100.location.y;
  return sqrt(pow(diffX, 2) + pow(diffY, 2));
}

double getErrorTheta(double x, double y) {
  double error = Enes100.location.theta * 180 / M_PI - getAngle(x, y);
  if (error < -180) {
    error += 360;
  } else if (error > 180) {
    error -= 360;
  }
  return error;
}

//returns angle to a point from 180 to -180 degrees
double getAngle(double x, double y) {
  double diffX = x - Enes100.location.x;
  double diffY = y - Enes100.location.y;
  double angle = atan(diffY / diffX) * 180 / M_PI;
  if (diffX < 0) {
    if (diffY >= 0) {
      angle += 180;
    } else if (diffY < 0) {
      angle -= 180;
    }
  }
  return angle;
}

//Motor Drive Functions
//modified drive - needs pwm
void drive(int right, int left) {
  //Enes100.println("Drive Called");  //left wheels
  if (left > 0) {
    if (left > 255) {
      left = 255;
    } else if (left < 80) {
      left = 80;
    }
    analogWrite(motorA_PWM, left);
    digitalWrite(motorAF, HIGH);
    digitalWrite(motorAB, LOW);
    //Enes100.println("left on");
  } else if (left < 0) {
    if (left < -255) {
      left = -255;
    } else if (left > -80) {
      left = -80;
    }
    analogWrite(motorA_PWM, (-1) * left);
    digitalWrite(motorAB, HIGH);
    digitalWrite(motorAF, LOW);
  } else {
    digitalWrite(motorAF, LOW);
    digitalWrite(motorAB, LOW);
  }  //right wheels
  if (right > 0) {
    if (right > 255) {
      right = 255;
    } else if (right < 80) {
      right = 80;
    }
    analogWrite(motorB_PWM, right);
    digitalWrite(motorBF, HIGH);
    digitalWrite(motorBB, LOW);
    //Enes100.println("right on");
  } else if (right < 0) {
    if (right < -255) {
      right = -255;
    } else if (right > -80) {
      right = -80;
    }
    analogWrite(motorB_PWM, (-1) * right);
    digitalWrite(motorBB, HIGH);
    digitalWrite(motorBF, LOW);
  } else {
    digitalWrite(motorBF, LOW);
    digitalWrite(motorBB, LOW);
  }
}

void reverse(void) {
  // Make A go back backwards
  analogWrite(motorA_PWM, -50);
  digitalWrite(motorAB, HIGH);
  digitalWrite(motorAF, LOW);
  // Make B go backwards
  analogWrite(motorB_PWM, -50);
  digitalWrite(motorBB, HIGH);
  digitalWrite(motorBF, LOW);
}

void forward(void) {
  // Make A go forward
  analogWrite(motorA_PWM, 50);
  digitalWrite(motorAF, HIGH);
  digitalWrite(motorAB, LOW);
  // Make B go forward
  analogWrite(motorB_PWM, 50);
  digitalWrite(motorBF, HIGH);
  digitalWrite(motorBB, LOW);
}

// A is right side of wheels
// B is left side of wheels
void goToTheta(double theta) {
  double dTheta = theta - Enes100.getTheta();
  delay(1000);
  //min and max acceptable angles, change to change tolerance
  double tolThetaMax = theta + 0.1;
  double tolThetaMin = theta - 0.1;
  if (abs(dTheta) > 3.1415) {
    dTheta = (dTheta / abs(dTheta)) * -6.283 + dTheta;
  }
  //Enes100.println("dTheta: " + String(dTheta) + " theta: " + String(theta));
  if (dTheta < 0) {
    turnRight();
  } else {
    turnLeft();
  }
  //turns until within tolerance.
  dTheta = Enes100.getTheta();
  while (dTheta < tolThetaMin || dTheta > tolThetaMax) {
    delay(100);
    dTheta = Enes100.getTheta();
    Enes100.println(dTheta);
  }
  //Enes100.println("ended turn");
  motorStop();
}

void turnRight() {
  int turnSpeed = 80;
  // Make right side of wheels go back
  analogWrite(motorA_PWM, -1 * turnSpeed);
  digitalWrite(motorAB, HIGH);
  digitalWrite(motorAF, LOW);
  // Make left side of wheels go forward
  analogWrite(motorB_PWM, turnSpeed);
  digitalWrite(motorBF, HIGH);
  digitalWrite(motorBB, LOW);
}

void turnLeft() {
  int turnSpeed = 100;
  // Make right side of wheels go forward
  analogWrite(motorA_PWM, turnSpeed);
  digitalWrite(motorAF, HIGH);
  digitalWrite(motorAB, LOW);
  // Make left side of wheels go backwards
  analogWrite(motorB_PWM, -1 * turnSpeed);
  digitalWrite(motorBB, HIGH);
  digitalWrite(motorBF, LOW);
}

void motorStop() {
  Enes100.println("Motor Stop Called");
  digitalWrite(motorAF, LOW);
  digitalWrite(motorAB, LOW);
  digitalWrite(motorBF, LOW);
  digitalWrite(motorBB, LOW);
  Enes100.println("Motor should be off");
}

// Place allowances
void alongX(double x) {
  goToTheta(0);
  double position = Enes100.getX();  // Position needs to decrease to x: reverse
  if (position > x) {
    while (position > x) {
      // Go back until destination is achieved
      reverse();
      // Update x position until while loop exits
      position = Enes100.getX();
      if (x > position) {
        motorStop();
        return;
      }
    }
  }  // Position needs to increase to x: forward
  else if (x > position) {
    while (x > position) {
      // Go back until destination is achieved
      forward();
      // Update x position until while loop exits
      position = Enes100.getX();
      if (position > x) {
        motorStop();
        return;
      }
    }
  }
}

void alongY(double y) {
  Enes100.println("alongy function called");

  double position = Enes100.getY();
  goToTheta(-1.517);
  // Position needs to decrease to y: reverse
  if (position < y) {
    while (position < y) {
      reverse();
      // Update y position until while loop exits
      position = Enes100.getY();
      Enes100.println(position);
      if (y <= position) {
        motorStop();
        return;
      }
    }
  } else if (y < position) {  // Position needs to increase to y: forward
    while (y < position) {
      forward();
      // Update y position until while loop exits
      position = Enes100.getY();
      if (y > position) {
        motorStop();
        return;
      }
    }
  }
}

//servo functions
void dropBean(boolean right) {
  //Enes100.println("drop bean!");
  if (right) {
    alongX(Enes100.getX() + 0.02);
  } else {
    alongY(Enes100.getY() - 0.02);
  }
  delay(1000);
  myservo.write(70);
  delay(1000);
  myservo.write(5);
  servoReset();
}

void servoReset() {
  delay(1000);
  myservo.write(90);
}

void beanReset() {
  delay(1000);
  myservo.write(175);
  delay(5000);
  myservo.write(150);
  delay(5000);
  myservo.write(120);
  delay(5000);
}
