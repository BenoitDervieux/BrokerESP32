/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-one-to-many-esp32-esp8266/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t broadcastAddress1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  bool master;
  char message[32];
} struct_message;

struct_message myData;
bool isMaster = true;
bool foundAMaster = false;

esp_now_peer_info_t peerInfo;

const char* ssid     = "ESP32-Access-Point-lesklights";
const char* password = "123456789";

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("IsMaster: ");
  Serial.println(myData.master);
  Serial.print("Message: ");
  Serial.println(myData.message);
  Serial.println();
  char * strToCompareRUMSTR = "RUMSTR?";
  char * strToCompareYIMSTR = "YIMSTR?";
  if (strcmp(myData.message, strToCompareRUMSTR) == 0) {
    Serial.println("Oui je suis ton Master michel");
    struct_message test;
    test.master = isMaster;
    strncpy(test.message, "YIMSTR?", sizeof(test.message));
    test.message[sizeof(test.message) - 1] = '\0';
    esp_err_t result1 = esp_now_send(
        broadcastAddress1, 
        (uint8_t *) &test,
        sizeof(struct_message));
       
      if (result1 == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
  }
  if (strcmp(myData.message, strToCompareYIMSTR) == 0) {
    Serial.println("Oui je vais donc devenir un slave");
    foundAMaster = true;
    Serial.println("On a changé le found a master");
  }
}
 
void setup() {
 
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
  int networks = WiFi.scanNetworks();
  char sent[80];
  char * wordy = "lesklights";
  for (int i = 0; i < networks; i++) {
    Serial.println(WiFi.SSID(i));
    strcpy(sent, WiFi.SSID(i).c_str()); 
    if (strstr(sent, wordy)) {
      isMaster = false;
    }
  }
  Serial.print("Is master? ");
  Serial.println(isMaster);
  if (isMaster) {
    WiFi.softAP(ssid, password);
    Serial.println("Became access point!");
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  esp_now_register_send_cb(OnDataSent);

  // register peer
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
    
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  if (!isMaster && !foundAMaster) {
    Serial.print("Is master:");
    Serial.println(isMaster);
    Serial.print("Found a master:");
    Serial.println(foundAMaster);
      struct_message test;
      test.master = isMaster;
      strncpy(test.message, "RUMSTR?", sizeof(test.message));
      test.message[sizeof(test.message) - 1] = '\0';
     
      esp_err_t result1 = esp_now_send(
        broadcastAddress1, 
        (uint8_t *) &test,
        sizeof(struct_message));
       
      if (result1 == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
      delay(500);
      delay(2000);
  }
}