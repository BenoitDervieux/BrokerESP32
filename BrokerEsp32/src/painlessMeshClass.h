#ifndef PAINLESSMESHCLASS_H
#define PAINLESSMESHCLASS_H
#include <FastLED.h>
#include <iostream> 
#include <string> // for string class 
using namespace std;
#include <SPI.h>
#include "painlessMesh.h"

#define NUM_LEDS 10
#define DATA_PIN_1 23
#define BRIGHTNESS 100
#define BUTTON_PIN 15


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

class PainLessMeshClass {

    public: 
        PainLessMeshClass();
        void instantiate();
        void wifi_init();
        void run();
        void sendMessage();
        String getReadings();
        void receivedCallback( uint32_t from, String &msg );
        void newConnectionCallback(uint32_t nodeId);
        void changedConnectionCallback();
        void nodeTimeAdjustedCallback(int32_t offset);
        CRGB hexToCRGB(const String& hex);
        String decimalStringToHex(String decimalString);

    private:
        Task task;
        String readings;
        int buttonPin;
        int buttonState;
        String bufferMessage;

};

#endif