/*********
  Sources: 
  Rui Santos & Sara Santos - Random Nerd Tutorial -Complete project details at https://RandomNerdTutorials.com/esp-now-one-to-many-esp32-esp8266/
*********/
#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure for the data
typedef struct struct_message {
  bool master;
  char message[32];
} struct_message;

struct_message myData; // Structure for the messages
bool isMaster = true; // Define if someone is the master
bool foundAMaster = false; // Define if has found a master 
uint8_t mac[6]; // Structure for holding the mac address

esp_now_peer_info_t peerInfo;

const char* ssid     = "ESP32-Access-Point-lesklights";
const char* password = "123456789";

// Function definition
bool stringToMac(const char* macStr, uint8_t* mac);
void areYouAMasterMsgRecepted();
void registerMacAddress(const char * macAddressString);

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

