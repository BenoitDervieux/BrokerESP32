#include "painlessMeshClass.h"

CRGB leds[NUM_LEDS];
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
int buttonState = 0;


PainLessMeshClass::PainLessMeshClass() {
	buttonPin = BUTTON_PIN;
	buttonState = 0;
}

void PainLessMeshClass::instantiate() {
	this->task.set(0, 0, [this]() { this->sendMessage(); });
	FastLED.addLeds<WS2812B, DATA_PIN_1, GRB>(leds, NUM_LEDS);
	FastLED.setBrightness(BRIGHTNESS);
	fill_solid(leds, NUM_LEDS, CRGB::Black);
	FastLED.show();
	pinMode(buttonPin, INPUT);
	
}

void PainLessMeshClass::wifi_init() {
	// Serial.println("Init WiFi");
	//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
	mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
	Serial.println("After set Debug");
	mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
	Serial.println("After init");
	mesh.onReceive([this](uint32_t from, String &msg) { this->receivedCallback(from, msg); });
	Serial.println("After onReceive");
	mesh.onNewConnection([this](uint32_t nodeId) { this->newConnectionCallback(nodeId); });
	Serial.println("After new connection");
	mesh.onChangedConnections([this]() { this->changedConnectionCallback(); });
	Serial.println("After on change");
	mesh.onNodeTimeAdjusted([this](int32_t offset) { this->nodeTimeAdjustedCallback(offset); });
	Serial.println("After on node");
	userScheduler.addTask( this->task );
	Serial.println("After add task");
	this->task.enable();
	Serial.println("after enable");

	fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
}

void PainLessMeshClass::run() {
	buttonState = digitalRead(buttonPin);
	if (buttonState == HIGH) {
		Serial.println("Sending message...");
		PainLessMeshClass::sendMessage();
		fill_solid(leds, NUM_LEDS, CRGB::Purple);
		// fill_solid(leds, NUM_LEDS, CRGB::Chartreuse);
		// fill_solid(leds, NUM_LEDS, CRGB::Red);
		FastLED.show();
	}
	mesh.update();
}

void PainLessMeshClass::sendMessage() {
	JsonDocument doc;
	// doc["color"] = 0xDFFF00; // Chartreuse
	// doc["color"] = 0xFF0000;
	doc["color"] = 0xA020F0; // purple
	String output;
	serializeJson(doc, output);
	Serial.println(output);
	mesh.sendBroadcast(output);
}


// Needed for painless library
void PainLessMeshClass::receivedCallback( uint32_t from, String &msg ) {
//   if (!strcmp(bufferMessage.c_str(), msg.c_str())) {
	Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
	bufferMessage = msg.c_str();
	JsonDocument doc;
	deserializeJson(doc, msg);
	String colorHex = doc["color"].as<String>();
	Serial.printf("color: %s\n", colorHex.c_str());
	CRGB color = hexToCRGB(this->decimalStringToHex(colorHex));
	fill_solid(leds, NUM_LEDS, color);
	FastLED.show();
//   }
}

void PainLessMeshClass::newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void PainLessMeshClass::changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void PainLessMeshClass::nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

CRGB PainLessMeshClass::hexToCRGB(const String& hex) {
    // Ensure the hex string is valid and properly formatted
    String cleanedHex = hex;
    if (cleanedHex.startsWith("#")) {
        cleanedHex.remove(0, 1); // Remove '#'
    }
    uint32_t color = strtoul(cleanedHex.c_str(), nullptr, 16);
    return CRGB(
        (color >> 16) & 0xFF,  // Red
        (color >> 8) & 0xFF,   // Green
        color & 0xFF           // Blue
    );
}

String PainLessMeshClass::decimalStringToHex(String decimalString) {
    // Convert the decimal string to an integer
    int decimalColor = decimalString.toInt();

    // Use sprintf to format the integer as a hexadecimal string
    char hexColor[8];  // Enough space for a 6-digit hex number and the null terminator
    sprintf(hexColor, "#%06X", decimalColor);
    
    // Return the hexadecimal string
    return String(hexColor);
}