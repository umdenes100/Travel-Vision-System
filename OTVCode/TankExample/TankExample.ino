#include <Enes100.h>
#include <Tank.h>

#define put(x) {Enes100.print(F(#x ": ")); Enes100.print(x);}
#define putn(x, n) {Enes100.print(F(#x ": ")); Enes100.print(x, n);}
#define putl(x) {Enes100.print(F(#x ": ")); Enes100.println(x);}
#define putnl(x, n) {Enes100.print(F(#x ": ")); Enes100.println(x, n);}
#define ps(x) Enes100.print(F(x));
#define psl(x) Enes100.println(F(x));
#define p(x) Enes100.print(x);
#define pl(x) Enes100.println(x);

#define STATE_DELAY .1

void setup() {
    Enes100.begin("TankExample", DATA, 14, 52, 50);
    Tank.begin();
    delay(1000);
    //    Tank.setLeftMotorPWM(-100);
    //    Tank.setRightMotorPWM(100);
    //    delay(1000);
    //    Tank.setLeftMotorPWM(100);
    //    Tank.setRightMotorPWM(-100);
    //    delay(1000);
}

void loop() {
    goTo(1.5, 0.6);
    goTo(0.5, 0.6);
    return;
//    goTo(0.5, 0.8);
//    goTo(0.5, 0.2);
//    goTo(1.5, 0.2);
//    goTo(1.5, 0.8);
}

const float kP = 400.0;
#define MAX_TURN_SPEED 255

float hyp(float x, float y) {
    return sqrt(x*x + y*y);
}

void goTo(double tx, double ty) {
    ps("Going to x:");
    p(tx);
    ps(" y:");
    pl(ty);
    double dis = 100;
    double speed = 0;
    const float alpha = 0.2;
    uint32_t last_time = millis();
    uint32_t current_time = millis();
    float lastTheta = Enes100.getTheta();

    while (dis > 0.15) { //Within 5 CM
        delay(200);
        last_time = current_time;
        current_time = millis();
//        putl(tx - Enes100.getX());
//        putl(tyEnes100.getY());
        float new_dis = hyp(tx - Enes100.getX(), ty - Enes100.getY());
        dis = new_dis;
        //We could go directly to a point, but we could also shoot for a point on a line.
        float target_angle = atan2(ty - Enes100.getY(), tx - Enes100.getX());
        //We are going to try to predict the robots location forward in time by extrapolating current rotational state.
        float delta_theta = Enes100.getTheta() - lastTheta;
        lastTheta = Enes100.getTheta();
        float current_angle = Enes100.getTheta() - HALF_PI + delta_theta * STATE_DELAY;
        if (current_angle > PI) current_angle -= 2 * PI;
        //Our current will stay the same. It is within the range -pi/2 to 3pi/2
        //Our target anle, however will change based on how we want to approach the thing.
        //For example, if (assume for a second our current is -pi to pi) we are targeting
        //.9pi and are currently at -.9 pi, we should instead try to target -1.1 pi, which is equivalent
        //We will try to find a potential new target.
        //Basically, we should be able to get the target within a pi of the current.
        float delta = abs(target_angle - current_angle);
        if (delta > PI) {
            if (target_angle > current_angle) {
                target_angle = target_angle - 2 * PI;
            }
            else {
                target_angle = target_angle + 2 * PI;
            }
        }
//        putl(toDeg(target_angle));
//        putl(toDeg(current_angle));
        speed = alpha * (10 / (abs(target_angle - current_angle))) + ((1 - alpha) * speed);
//        speed = 0;
        if (speed > 255) speed = 255;
        //        ps(" ts: ");
        //        p(speed);
        //        ps(" ms: ");
        double p = kP / 2 * (target_angle - current_angle);
        p = constrain(p, -MAX_TURN_SPEED, MAX_TURN_SPEED);
//        putl(dis);
        Tank.setLeftMotorPWM(-p + speed);
        Tank.setRightMotorPWM(p + speed);
    }
}

float toDeg(float rad) {
    return 180.0f * rad / PI;
}
