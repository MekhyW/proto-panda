#pragma once
#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <map>
#include <vector>
#include <stack>
#include <cstring> 

#define BUTTON_RELEASED 0
#define BUTTON_JUST_PRESSED 1
#define BUTTON_PRESSED 2
#define BUTTON_JUST_RELEASED 3

class BleManager;

class BleSensorData{
  public:
    BleSensorData():x(0),y(0),z(0),ax(0),ay(0),az(0),temp(0){
      for (int i=0;i<MAX_BLE_BUTTONS;i++){
        buttons[i] = 0;
      }
    }
    int16_t x,y,z;
    int16_t ax,ay,az;
    int16_t temp;
    uint8_t buttons[MAX_BLE_BUTTONS+1];
};

class BleSensorHandlerData : public BleSensorData{
  public:
    BleSensorHandlerData():BleSensorData(){}

    void copy(BleSensorData* aux){
      this->x = aux->x;
      this->y = aux->y;
      this->z = aux->z;
      this->ax = aux->ax;
      this->ay = aux->ay;
      this->az = aux->az;
      this->temp = aux->temp;
      for (int i = 0; i < MAX_BLE_BUTTONS; ++i) {
          this->buttons[i] = aux->buttons[i];
      }
    }

    void updateButtons();

    uint32_t last_inputButtonsStatus[MAX_BLE_BUTTONS];  
    uint32_t real_inputButtonsStatus[MAX_BLE_BUTTONS];  
};

class AccelerometerData{
  public:
    int x,y,z;
};

class ClientCallbacks : public NimBLEClientCallbacks {
    
    void onConnect(NimBLEClient* pClient);
    void onDisconnect(NimBLEClient* pClient);
    bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params);
    uint32_t onPassKeyRequest();
    bool onConfirmPIN(uint32_t pass_key);
    void onAuthenticationComplete(ble_gap_conn_desc* desc);
  public:
    ~ClientCallbacks() {};
    ClientCallbacks():bleObj(nullptr){};
    BleManager *bleObj;


};

class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice);
  public:
    BleManager *bleObj;
};

class ConnectTuple{
  public:
    ConnectTuple(NimBLEAdvertisedDevice* device, NimBLEUUID service, NimBLEUUID characteristic):m_device(device),m_service(service),m_characteristic(characteristic),callbacks(nullptr),m_client(nullptr),clientMode(0xff),connected(true){};
    NimBLEAdvertisedDevice* m_device;
    NimBLEUUID m_service;
    NimBLEUUID m_characteristic;
    ClientCallbacks * callbacks;
    NimBLEClient* m_client;
    u_int16_t clientMode;
    bool connected;

    
};


class BleManager{
  public:
    BleManager():clientCount(0),lastScan(0), scanInterval(1), maxClients(1),m_started(false),m_canScan(false){}
    bool begin();
    void update();
    void updateButtons();
    void beginScanning();

    int acceptTypes(std::string service, std::string characteristic);
    //void waitForControls(int waitSize);
    void setMaximumControls(int n){maxClients = n;};

    int getElementIdByUUID(NimBLEUUID id);
    //bool hasServiceAlready(BLEUUID uuid);

    int getConnectedClients(){
      return clients.size();
    }

    bool isElementIdConnected(int id);

    bool hasChangedClients();


    static BleSensorHandlerData remoteData[MAX_BLE_CLIENTS];


    std::vector<std::tuple<NimBLEUUID,NimBLEUUID>> GetAcceptedUUIDS(){
      return m_acceptedUUIDs;
    }
  private:
    std::map<NimBLEAddress,ConnectTuple*> clients;
    bool connectToServer(ConnectTuple *tpl);
    uint16_t clientCount;
    //bool connectToDevice(ConnectTuple tpl);
    //void showWaitDisplay();
    //void showScanningDisplay();
    uint32_t lastScan, scanInterval, maxClients;
    bool m_started, m_canScan;

    //std::stack<ConnectTuple> toConnectDevices;

    //std::map<uint16_t,BleConnectedClient> clients;
    //bool changedClient;
    //std::vector<DescriptorUUID> m_acceptedUUID;

    std::vector<std::tuple<NimBLEUUID,NimBLEUUID>> m_acceptedUUIDs;


    //friend class MyAdvertisedDeviceCallbacks;
    friend class ClientCallbacks;
};

extern BleManager g_remoteControls;
