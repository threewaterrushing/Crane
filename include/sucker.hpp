#ifndef sucker_hpp
#define sucker_hpp 
#include <Arduino.h>

#define sucker 4

//夹取
void pick_up(){
    digitalWrite(sucker,HIGH);
}


void release(){
    digitalWrite(sucker,LOW);
}



#endif