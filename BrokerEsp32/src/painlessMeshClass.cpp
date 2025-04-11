#include "painlessMeshClass.h"


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


PainLessMeshClass::PainLessMeshClass() {	

}

void PainLessMeshClass::instantiate(FastLedClass fastLedClass) {
	this->task.set(0, 0, [this]() { this->sendMessage(); });
	this->fastLedClass = fastLedClass;
}

void PainLessMeshClass::wifi_init() {
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
}

void PainLessMeshClass::run() {
	Serial.println("Sending message...");
	PainLessMeshClass::sendMessage();
	this->fastLedClass.setToPurple();
	mesh.update();
}

void PainLessMeshClass::sendMessage() {
	JsonDocument doc;
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
	this->fastLedClass.changeColor(colorHex.c_str());
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