#include "CurrentState.h"

#define GPIO1 0
#define GPIO2 7

void CurrentState::InitPWM() {

    wiringPiSetup();
    
    pwmSetMode(PWM_MODE_MS);
    pwmSetClock(384);
    pwmSetRange(1000);

    pinMode(GPIO1, OUTPUT);
    digitalWrite(GPIO1, LOW);
    softPwmCreate(GPIO1, 0, 200);

    pinMode(GPIO2, OUTPUT);
    digitalWrite(GPIO2, LOW);
    softPwmCreate(GPIO2, 0, 200);
}

void CurrentState::LeftServoBeStop() {
    softPwmWrite(GPIO1, 0);
};

void CurrentState::LeftServoBeForward() {
    softPwmWrite(GPIO1, 5);
}

void CurrentState::LeftServoBeBack() {
    softPwmWrite(GPIO1, 20);
}

void CurrentState::RightServoBeStop(){
    softPwmWrite(GPIO2, 0);
}
void CurrentState::RightServoBeForward(){
    softPwmWrite(GPIO2, 20);
}
void CurrentState::RightServoBeBack(){
    softPwmWrite(GPIO2, 5);
}