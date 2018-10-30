
#ifndef CURRENTSTATE_H
#define CURRENTSTATE_H

#include <wiringPi.h>
#include <softPwm.h>
#include <thread>

class CurrentState {
public:
    
    static CurrentState& get() {
        static CurrentState instance;
        return instance;
    }
    
    void InitPWM();
    void LeftServoBeStop();
    void LeftServoBeForward();
    void LeftServoBeBack();
    
    void RightServoBeStop();
    void RightServoBeForward();
    void RightServoBeBack();

private:
    
    bool isRunProcessing;

};

#endif /* CURRENTSTATE_H */

