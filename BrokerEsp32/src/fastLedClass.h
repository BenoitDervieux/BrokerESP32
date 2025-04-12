#ifndef FASTLEDCLASS_H
#define FASTLEDCLASS_H
#include <FastLED.h>
#include <iostream> 
#include <string> // for string class 
using namespace std;

#define NUM_LEDS 10
#define DATA_PIN_1 23
#define BRIGHTNESS 100
#define BUTTON_PIN 15



class FastLedClass {

    public: 
        FastLedClass();
        void init();
        void changeColor(String colorToConvert);
        CRGB hexToCRGB(const String hex);
        String decimalStringToHex(String decimalString);
        void setToPurple();

    private:
        String readings;
        int buttonPin;
        int buttonState;
        String bufferMessage;

};

#endif