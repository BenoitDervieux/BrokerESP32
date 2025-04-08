/*********
  Sources: 
  Rui Santos & Sara Santos - Random Nerd Tutorial -Complete project details at https://RandomNerdTutorials.com/esp-now-one-to-many-esp32-esp8266/
*********/
#include <esp_now.h>
#include <WiFi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <utility>

#include "painlessMeshClass.h"

PainLessMeshClass painLessMeshClass;

uint8_t broadcastAddress1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure for the data
typedef struct struct_message {
  bool master;
  char message[32];
} struct_message;

struct_message myData; // Structure for the messages
bool isMaster = true; // Define if someone is the master
bool foundAMaster = false; // Define if has found a master
bool connectedToWifi = false;
bool connectedToPMesh = false;
uint8_t mac[6]; // Structure for holding the mac address

esp_now_peer_info_t peerInfo;

const char* ssid     = "ESP32-Access-Point-lesklights";
const char* password = "123456789";

const char * normal_ssid = getenv("normal_ssid");
const char * normal_password = getenv("normal_password");

// Function definition
bool stringToMac(const char* macStr, uint8_t* mac);
void areYouAMasterMsgRecepted();
void registerMacAddress(const char * macAddressString);
void load_env_file(const char * filename);
void masterConfirmationMsgRecepted();
void displayByteReceived(struct_message myData, int len);

// Variables to hold the mac addresses
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
  displayByteReceived(myData, len);
  
  char * strToCompareRUMSTR = "RUMSTR?"; // Asking for the master
  char * strToCompareYIMSTR = "YIMSTR?"; // Responding that one is the master
  char * strToCompareHIMMA = "HIMMA:"; // here is my mac address
  char * strToCompareInstructionEffects = "IE"; // Instruction and Effects

  // When a master receive a demand if it master
  if (strcmp(myData.message, strToCompareRUMSTR) == 0 && isMaster) {
    areYouAMasterMsgRecepted();
  }

  // When a slave receive a message saying that one is a slave
  if (strcmp(myData.message, strToCompareYIMSTR) == 0 && !isMaster) {
    masterConfirmationMsgRecepted();
  }

  // When a master receive a message with a mac address to register
  if (strncmp(myData.message, strToCompareHIMMA, strlen(strToCompareHIMMA)) == 0) {
    registerMacAddress(myData.message);
  }

  // Here is for the instruction and effects, just a test
  if (strncmp(myData.message, strToCompareInstructionEffects, strlen(strToCompareInstructionEffects)) == 0) {
    Serial.println("\n#####################\nReceived my first instruction:");
    Serial.println(myData.message);
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
void displayByteReceived(struct_message myData, int len) {
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("IsMaster: ");
  Serial.println(myData.master);
  Serial.print("Message: ");
  Serial.println(myData.message);
  Serial.println();
}
void areYouAMasterMsgRecepted() {
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
void masterConfirmationMsgRecepted() {
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
void registerMacAddress(const char * macAddressString) {
    Serial.println("Should be ready to add this mac address!");
    Serial.println(macAddressString);
    
    // Find where the MAC address starts
    const char* macStart = strstr(macAddressString, "HIMMA:");
    if (!macStart) {
        Serial.println("MAC address not found in message");
        return;
    }
    macStart += 6; // Move past "HIMMA:"
    
    // Extract the MAC address string
    char address[18] = {0};
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
    
    // Check for duplicates before storing
    bool exists = false;
    for (int i = 0; i < indexForMacAddresses; i++) {
        if (memcmp(macAddresses[i], mac, MAC_LENGTH) == 0) {
            exists = true;
            break;
        }
    }
    
    if (!exists) {
        if (indexForMacAddresses < MAX_MAC_COUNT) {
            memcpy(macAddresses[indexForMacAddresses], mac, MAC_LENGTH);
            indexForMacAddresses++;
            
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
    } else {
        Serial.println("MAC address already exists");
    }
}

void setup() {

  // Logic is first try to connect to the main network (hard code first) [TODO]
  // Then try to connect to know networks (hard code here) [TODO]
  // Then try to find a master --> logic handled 
  // Then becomes a master --> logic handled
 
  Serial.begin(9600);

  // Load environment variables from .env file
  // load_env_file(".env");
  if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
  }

  fs::File envVariables = SPIFFS.open("/networks.json", FILE_READ);
  if (!envVariables) {
      Serial.println("Failed to open file");
      // return;
  }
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, envVariables);
  envVariables.close();
  if (error) {
      Serial.println("Failed to parse JSON file");
      return;
  }

  // Retrieve environment variables
  const char *normal_ssid = doc["my_wifi"]["ssid"];
  const char *normal_password = doc["my_wifi"]["password"];
  Serial.print("Normal SSID: ");
  Serial.println(normal_ssid);
  Serial.print("Normal PWD: ");
  Serial.println(normal_password);

  // We try to connect to the normal wifi
  WiFi.begin(normal_ssid, normal_password);
  int attemptMain = 0;
    // Here we try to connect
  while (WiFi.status() != WL_CONNECTED && attemptMain < 5) {
      delay(1000);
      Serial.println("Connecting to WiFi in main credentials...");
      Serial.print("Pt 5475: Attempts --> ");
      Serial.println(attemptMain);
      attemptMain++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi in the main");
    WiFi.mode(WIFI_STA);
    isMaster = false;
    Serial.print("Is master? ");
    Serial.println(isMaster);
    connectedToWifi = 1;
    Serial.println("Now you're connected to Wifi...");
    painLessMeshClass.instantiate();
    painLessMeshClass.wifi_init();
    connectedToPMesh = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Now we gonna go through all the networks we have in the json file
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    int networksFound = WiFi.scanNetworks();
    Serial.print("PT 44444: Number of networks found --> ");
    Serial.println(networksFound);
    std::vector<std::pair<std::string, int>> network_map;
    for (int i = 0; i < networksFound; ++i) {
        // std::string ssid_temp = WiFi.SSID(i).c_str();
        network_map.push_back(std::make_pair(WiFi.SSID(i).c_str(), WiFi.RSSI(i)));
    }
    auto cmp = [](std::pair<std::string, int> a, std::pair<std::string, int>b) { return a.second > b.second; };
    std::sort(network_map.begin(), network_map.end(), cmp);
    std::vector<std::pair<std::string, std::string>> networks_available;
    for (int i = 0; i < network_map.size(); i++) {
      std::string network_ssid = network_map[i].first;
      // Iterate through all the networks in memory
      for (int j = 0; j < doc["networks"].size(); j++) {
          std::string in_memory_ssid = doc["networks"]["networks" + String(j)]["ssid"];
          std::string in_memory_pwd = doc["networks"]["networks" + String(j)]["password"];
          Serial.println(in_memory_ssid.c_str());
          if (in_memory_ssid.compare(network_ssid) == 0) {
            std::pair<std::string, std::string> pairToAdd;
            pairToAdd.first = in_memory_ssid;
            pairToAdd.second = in_memory_pwd;
            networks_available.push_back(pairToAdd);
          }
      }
    }
    // Check if there were networks available around that we could connect to
    if (networks_available.size() > 0) {
      // Iterate through all the networks available
      for (int i = 0; i < networks_available.size(); i++) {
          // Set the SSID and password locally in the class
          std::string ssidToTry = networks_available[i].first.c_str();
          std::string pwdToTry = networks_available[i].second.c_str();
          // Try to start the connexion
          WiFi.begin(ssidToTry.c_str(), pwdToTry.c_str());
          int attemptSecond = 0;
          // 10 seconds or 10 attempts
          while (WiFi.status() != WL_CONNECTED && attemptSecond < 5) {
              delay(100);
              Serial.println("Connecting to WiFi in secondary networks ...");
              attemptSecond++;
          }
          // If connected, the set as a client and then break the loop
          if (WiFi.status() == WL_CONNECTED) {
              Serial.println("Connected to Wifi with a stocked password");
              connectedToWifi = 1;
              painLessMeshClass.instantiate();
              painLessMeshClass.wifi_init();
              connectedToPMesh = true;
              break;
          }
          
      }
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Didn't managed to connect to Wifi");
      }
    }



    if (WiFi.status() != WL_CONNECTED) {
        // Under is the fact of scanning the network
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
    }
}
 
void loop() {
  if (!connectedToWifi && !isMaster && !foundAMaster) {
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

  if (connectedToWifi == 1) {
    Serial.println("Im just a chill ESP 32 connected to wifi");
    delay(3000);
  }

  if (connectedToPMesh) {
    Serial.println("Im just a chill ESP 32 mesh connected to wifi");
    painLessMeshClass.run();
  }

  if (isMaster && indexForMacAddresses > 0) {
    static unsigned long lastSendTime = 0;
    const unsigned long sendInterval = 5000; // 5 seconds
    
    if (millis() - lastSendTime >= sendInterval) {
      lastSendTime = millis();
      
      Serial.println("Attempting to send message to all registered devices");
      
      struct_message test;
      test.master = isMaster;
      strncpy(test.message, "IE52", sizeof(test.message) - 1);
      test.message[sizeof(test.message) - 1] = '\0';
      for (int i = 0; i < indexForMacAddresses; i++) {
          Serial.printf("Sending to MAC %02X:%02X:%02X:%02X:%02X:%02X... ",
                       macAddresses[i][0], macAddresses[i][1], macAddresses[i][2],
                       macAddresses[i][3], macAddresses[i][4], macAddresses[i][5]);

          // Add peer
          esp_now_peer_info_t peerInfo;
          memcpy(peerInfo.peer_addr, macAddresses[i], 6);
          peerInfo.channel = 0;
          peerInfo.encrypt = false;
          
          esp_err_t addResult = esp_now_add_peer(&peerInfo);
          if (addResult != ESP_OK) {
              Serial.printf("Failed to add peer (Error: %d)\n", addResult);
              continue;
          }
          
          esp_err_t sendResult = esp_now_send(
              macAddresses[i], 
              (uint8_t *)&test,
              sizeof(struct_message));
          
          // Remove peer after sending to free up space
          esp_now_del_peer(macAddresses[i]);
          
          if (sendResult == ESP_OK) {
              Serial.println("Success");
          } else {
              Serial.printf("Failed (Error: %d)\n", sendResult);
          }
          
          delay(10); // Short delay between sends
      }
    }
  }
}

void load_env_file(const char * filename) {
  FILE *file = fopen(filename, "r");
  if(file == NULL) {
    perror("Failed to open .env file");
    return;
  }
  char line[256];
  while(fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = 0;
    char * key = strtok(line, "=");
    char * value = strtok(NULL, "=");
    if (key && value) {
      if (setenv(key, value, 1) != 0) {
        perror("setenv");
      }
    }
  }

  fclose(file);
}
