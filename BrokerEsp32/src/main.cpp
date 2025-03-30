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
uint8_t mac[6]; 

esp_now_peer_info_t peerInfo;

const char* ssid     = "ESP32-Access-Point-lesklights";
const char* password = "123456789";

bool stringToMac(const char* macStr, uint8_t* mac);

#define MAX_MAC_COUNT 20
#define MAC_LENGTH 6
uint8_t macAddresses[MAX_MAC_COUNT][MAC_LENGTH];
int indexForMacAddresses = 0;

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
  // Asking for the master
  char * strToCompareRUMSTR = "RUMSTR?";
  // Responding that one is the master
  char * strToCompareYIMSTR = "YIMSTR?";
  // Sending bac the mac address, several times
  char * strToCompareHIMMA = "HIMMA:"; // here is my mac address
  char * strToCompareInstructionEffects = "IE:"; // here is my mac address
  Serial.print("Mac address to send:");
  Serial.println(WiFi.macAddress());
  // Answering that the mac address is registered

  // When a master receive a demand if it master
  if (strcmp(myData.message, strToCompareRUMSTR) == 0 && isMaster) {
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

  // When a slave receive a message saying that one is a slave
  if (strcmp(myData.message, strToCompareYIMSTR) == 0 && !isMaster) {
    Serial.println("Oui je vais donc devenir un slave");
    foundAMaster = true;
    Serial.println("Et je vais envoyer mon address mac");
    String macAddress = WiFi.macAddress();
    String message = "HIMMA:" + macAddress;
    Serial.print("Mac address in a message to send:");
    Serial.println(macAddress);
    Serial.print("Whole message:");
    Serial.println(message);
    struct_message test;
    test.master = isMaster;
    strncpy(test.message, message.c_str(), sizeof(test.message));
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

  // When a master receive a message with a mac address to register
  if (strncmp(myData.message, strToCompareHIMMA, strlen(strToCompareHIMMA)) == 0) {
    Serial.println("Should be ready to add this mac address!");
    Serial.println(myData.message);
    
    // Find where the MAC address starts
    const char* macStart = strstr(myData.message, "HIMMA:");
    if (!macStart) {
        Serial.println("MAC address not found in message");
        return;
    }
    macStart += 6; // Move past "HIMMA:"
    
    // Extract the MAC address string
    char address[18] = {0}; // Initialize to zeros
    if (strlen(macStart) >= 17) {
        strncpy(address, macStart, 17);
    } else {
        Serial.println("Invalid MAC address length");
        return;
    }
    
    // Convert to byte array
    uint8_t mac[MAC_LENGTH];
    if (!stringToMac(address, mac)) {
        Serial.println("Failed to convert MAC address");
        return;
    }
    
    // Store in array if there's space
    if (indexForMacAddresses < MAX_MAC_COUNT) {
        memcpy(macAddresses[indexForMacAddresses], mac, MAC_LENGTH);
        indexForMacAddresses++;
        
        // Print all stored MACs for verification
        Serial.println("Stored MAC addresses:");
        for (int i = 0; i < indexForMacAddresses; i++) {
            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                    macAddresses[i][0], macAddresses[i][1], macAddresses[i][2],
                    macAddresses[i][3], macAddresses[i][4], macAddresses[i][5]);
            Serial.println(macStr);
        }
    } else {
        Serial.println("MAC address storage full");
    }
  }

  // Here is for the instruction and effects, just a test
  if (strncmp(myData.message, strToCompareInstructionEffects, strlen(strToCompareInstructionEffects)) == 0) {
    Serial.println("\n#####################\nReceived my first instruction:");
    Serial.println(myData.message);
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

  if (indexForMacAddresses > 0) {
    Serial.println("Trying to send something");
    struct_message test;
    test.master = isMaster;
    strncpy(test.message, "IE52?", sizeof(test.message)); // Here it's like "Instructuon effect number 52
    test.message[sizeof(test.message) - 1] = '\0';
    for (int i = 0; i < indexForMacAddresses; i++) {
      esp_err_t result1 = esp_now_send(
        macAddresses[i], 
        (uint8_t *) &test,
        sizeof(struct_message));
    }

    delay(2000);
  }
}

bool stringToMac(const char* macStr, uint8_t* mac) {
  if (strlen(macStr) != 17) return false; // Check length "XX:XX:XX:XX:XX:XX"
  
  int values[6];
  if (sscanf(macStr, "%x:%x:%x:%x:%x:%x", 
             &values[0], &values[1], &values[2],
             &values[3], &values[4], &values[5]) != 6) {
    return false;
  }

  for (int i = 0; i < 6; i++) {
    mac[i] = (uint8_t)values[i];
  }
  char macMorning[18];
  snprintf(macMorning, sizeof(macMorning), "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);
  Serial.println(macMorning);
  return true;
}