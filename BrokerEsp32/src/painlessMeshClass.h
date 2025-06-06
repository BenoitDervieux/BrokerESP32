#ifndef PAINLESSMESHCLASS_H
#define PAINLESSMESHCLASS_H
#include <FastLED.h>
#include <iostream> 
#include <string> // for string class 
using namespace std;
#include <SPI.h>
#include "painlessMesh.h"
#include "fastLedClass.h"

#define NUM_LEDS 10
#define DATA_PIN_1 23
#define BRIGHTNESS 100



#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

class PainLessMeshClass {

    public: 
        PainLessMeshClass();
        void instantiate(FastLedClass fastLedClass);
        void wifi_init();
        void run();
        void sendMessage();
        String getReadings();
        void receivedCallback( uint32_t from, String &msg );
        void newConnectionCallback(uint32_t nodeId);
        void changedConnectionCallback();
        void nodeTimeAdjustedCallback(int32_t offset);

    private:
        Task task;
        String readings;
        String bufferMessage;
        FastLedClass fastLedClass;

};

#endif