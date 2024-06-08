#include <Enes100.h>

void setup() {
  // put your setup code here, to run once:
  delay(2000);
  Enes100.begin("CSS0", CRASH_SITE, 621, 4, 5);
  delay(2000);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop() {
  int dimensions_l[] = {135, 270, 180, 180, 135, 270};
  int dimensions_h[] = {180, 180, 135, 270, 270, 135};
  int possible_dimensions[] = {135, 270, 180};
  String colors[] = {"This Side Grey", "This Side Red"};
  //power on solenoid
  analogWrite(3, 250);
  delay(5000);
  Enes100.println("TESTING begin");
  if(getY() < 1){
    //Enes100.println("Start Position 1");
    driveTo(150, 0.4, 0.75);
    //Enes100.println("point 1 reached");
    //turns to face box head on
    turnTo(1.75);
    //more precise turn
    leftSpeed(50);
    while(getTheta() > 1.5707){
      delay(100);
    }
    stop();
    //first pause
    delay(3000);
    Enes100.begin("CSS", CRASH_SITE, 621, 8, 7);
    delay(3000);
    int result1 = Enes100.MLGetPrediction();
    Enes100.println(colors[result1]);
    delay(3000);
    Enes100.begin("CSS2", CRASH_SITE, 621, 8, 7);
    delay(3000);
    int result2 = Enes100.MLGetPrediction();
    delay(3000);
    Enes100.begin("CSS3", CRASH_SITE, 621, 4, 5);
    Enes100.mission(HEIGHT, dimensions_h[result2]);
    delay(3000);
    if(result1 == 1){
      analogWrite(3, 0);
      Enes100.mission(LENGTH, dimensions_l[result2]);
      Enes100.mission(DIRECTION, 1);
      delay(500);
      driveTo(150, 0.9, 1.5);
    }else{
      Enes100.mission(DIRECTION, NORMAL_X);
      for(int i = 0; i < 3; i++){
        if(possible_dimensions[i] == dimensions_l[result2] || possible_dimensions[i] == dimensions_h[result2]){
          delay(100);
        }else{
          Enes100.mission(LENGTH, possible_dimensions[i]);
        }
      }
      driveTo(150, 0.9, 1.5);
      delay(500);
      analogWrite(3, 0);
      delay(500);
    }
    //Enes100.println("point 2 reached");
  }else{
    //Enes100.println("Start Position 2");
    driveTo(150, 0.5, 1.15);
    //Enes100.println("point 1 reached");
    //turns to face box head on
    turnTo(-1.5707);
    //first pause
    delay(3000);
    Enes100.begin("CSS", CRASH_SITE, 621, 8, 7);
    delay(3000);
    int result1 = Enes100.MLGetPrediction();
    Enes100.println(colors[result1]);
    delay(3000);
    Enes100.begin("CSS2", CRASH_SITE, 621, 8, 7);
    delay(3000);
    int result2 = Enes100.MLGetPrediction();
    delay(3000);
    Enes100.begin("CSS3", CRASH_SITE, 621, 4, 5);
    delay(3000);
    Enes100.mission(HEIGHT, dimensions_h[result2]);
    if(result1 == 1){
      analogWrite(3, 0);
      Enes100.mission(LENGTH, dimensions_l[result2]);
      Enes100.mission(DIRECTION, 1);
      delay(500);
    }else{
      Enes100.mission(DIRECTION, NORMAL_X);
      for(int i = 0; i < 3; i++){
        if(possible_dimensions[i] == dimensions_l[result2] || possible_dimensions[i] == dimensions_h[result2]){
          delay(100);
        }else{
          Enes100.mission(LENGTH, possible_dimensions[i]);
        }
      }
      driveTo(150, 0.9, 0.4);
      delay(500);
      analogWrite(3, 0);
      delay(500);
    }
    //Enes100.println("point 2 reached");
  }
  driveTo(150, 0.8, 1.7);
  driveTo(150, 0.5, 1.8);
  driveTo(150, 1.0, 2.0);
  //Enes100.println("point 4 reached");
  driveStraight(150, 0.0, 1.7, true);
  //Enes100.println("point 5 reached");
  driveTo(150, 2.1, 2.02);
  //Enes100.println("point 6 reached");
  driveStraight(150, 0.0, 2.8, true);
  leftSpeed(150);
  rightSpeed(110);
  while(getTheta() > -0.5){
    delay(250);
  }
  //Enes100.println("turn 5 done");
  driveStraight(150, -0.5, 3.7, true);
  Enes100.println("end reached");
  driveTo(150, 0.5, 0.5);
}
//getters to avoid errors caused by the vision system returning -1 when not visible
void rightSpeed(int velocity){
  if (velocity > 0){
    analogWrite(10, 0);
    analogWrite(9, abs(velocity));
  }else{
    analogWrite(9, 0);
    analogWrite(10, abs(velocity));
  }
}
void leftSpeed(int velocity){
  if (velocity > 0){
    analogWrite(6, 0);
    analogWrite(11, abs(velocity));
  }else{
    analogWrite(11, 0);
    analogWrite(6, abs(velocity));
  }
}

//stops
void stop(){
  analogWrite(9, 0);
  analogWrite(10, 0);
  analogWrite(11, 0);
  analogWrite(6, 0);
}

void turn(int velocity, char direction){
  //l = left, r = right
  int dir = 0;
  if(direction == 'r'){
    dir = -1;
    Enes100.println("turning right");
  }else if(direction == 'l'){
    dir = 1;
    Enes100.println("turning left");
  }
  rightSpeed(dir * velocity);
  leftSpeed(dir * -velocity);
}

double getX() {
    while(!Enes100.isVisible())
    {
      Enes100.println("not visible");
      delay(180);
    }
    delay(120);
    return Enes100.getX();
} 
double getY() {
    while(!Enes100.isVisible())
    {
      Enes100.println("not visible");
      delay(180);
    }
    delay(120);
    return Enes100.getY();
} 
double getTheta() {
    while(!Enes100.isVisible())
    {
      Enes100.println("not visible");
      delay(180);
    }
    delay(120);
    return Enes100.getTheta();
} 

//wizard magic to drive towards given coordinates and course correct
void driveTo(int velocity, double targetX, double targetY){
  Enes100.println("Aiming for: " + String(targetX) + ", " + String(targetY));
  double tTheta = atan((targetY-getY())/(targetX-getX()));
  delay(250);
  if(tTheta < 0 && getY() < targetY){
    //if calc angle is in quadrant 4 but true angle is in quadrant 2
    tTheta = (tTheta + 3.142);
  }else if(tTheta > 0 && getY() > targetY){
    //if calc angle is in quadrant 1 but true angle is in quadrant 3
    tTheta = (tTheta - 3.142);
  }
  turnTo(tTheta);
  //temporary vars for X and Y to not constantly spam the getters
  double X = getX();
  double Y = getY();
  double dTheta = 0;
  while(X < (targetX - 0.1) || X > (targetX + 0.1) || Y < (targetY - 0.1) || Y > (targetY + 0.1)){
    X = getX();
    delay(150);
    Y = getY();
    delay(150);
    tTheta = atan((targetY-Y)/(targetX-X));
    if(tTheta < 0 && Y < targetY){
      //if calc angle is in quadrant 4 but true angle is in quadrant 2
      tTheta = (tTheta + 3.142);
    }else if(tTheta > 0 && Y > targetY){
      //if calc angle is in quadrant 1 but true angle is in quadrant 3
      tTheta = (tTheta - 3.142);
    }
    dTheta =  getTheta() - tTheta;
    if(abs(dTheta) > 0.75){
      Enes100.println(" tTheta:" + String(tTheta) + " dTheta: " + String(dTheta) + " dX:" + String(targetX-X) + " dY: " + String(targetY-Y));
    }
    rightSpeed(velocity-(int)(dTheta*300));
    leftSpeed(velocity+(int)(dTheta*300));
    //Enes100.println("RS: " + String(velocity-(dTheta*300)) + " LS:" + String(velocity+(dTheta*300)) + " TAng: " + String(tTheta));
    //Enes100.println("X: " + String(X) + " Y:" + String(Y) + " DAng: " + String(dTheta));
    delay(150);
  }
  stop();
}
//does not work if target angle is across the pi / -pi line
void turnTo(double tTheta){ 
  delay(150);
  double dTheta = tTheta - getTheta();
  delay(150);
  //min and max acceptable angles, change to change tolerance
  double tolThetaMax = tTheta + 0.15;
  double tolThetaMin = tTheta - 0.15;
  if(abs(dTheta) > 3.1415){
    dTheta = (dTheta/abs(dTheta))*-6.283 + dTheta;
  }
  //Enes100.println("dTheta: " + String(dTheta) + " tTheta: " + String(tTheta));
  if(dTheta < 0){
    turn(50, 'r');
  }else{
    turn(50, 'l');
  }
  //turns until within tolerance.
  dTheta = getTheta();
  while(dTheta < tolThetaMin || dTheta > tolThetaMax){
    delay(200);
    dTheta = getTheta();
  }
  //Enes100.println("ended turn");
  stop();
}
//drives at a set angle, only works for targets not in 3rd quadrant
void driveStraight(int velocity, double heading, double cutCoor, bool xOrY){
  double dTheta = 0;
  //true = x, false = y
  if(xOrY){
    while(getX() < cutCoor){
      dTheta = getTheta()-heading;
      rightSpeed(velocity-(dTheta*300));
      leftSpeed(velocity+(dTheta*300));
      delay(150);
    }
  }else{
    while(getY() < cutCoor){
      dTheta = getTheta()-heading;
      rightSpeed(velocity-(dTheta*300));
      leftSpeed(velocity+(dTheta*300));
      delay(150);
    }
  }
  stop();
}
