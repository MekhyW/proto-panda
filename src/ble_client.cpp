#include "ble_client.hpp"
#include "tools/logger.hpp"
#include <Arduino.h>

void scanEndedCB(NimBLEScanResults results){
  Logger::Info("[BLE] Scan Ended");
}

static bool isScanning = false;


static ConnectTuple *toConnect = nullptr;


BleSensorHandlerData BleManager::remoteData[MAX_BLE_CLIENTS];


void ClientCallbacks::onConnect(NimBLEClient* pClient) {
  Logger::Info("[BLE] Device Connected");
};


void ClientCallbacks::onDisconnect(NimBLEClient* pClient) {
  Serial.print(pClient->getPeerAddress().toString().c_str());
  Logger::Info("[BLE] Device disconnected");
  auto aux = g_remoteControls.clients[pClient->getPeerAddress()];
  if (aux != nullptr){
    g_remoteControls.clients[pClient->getPeerAddress()]->connected = false;
  }
};

bool ClientCallbacks::onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params) {
    if(params->itvl_min < 24) { /** 1.25ms units */
        return false;
    } else if(params->itvl_max > 40) { /** 1.25ms units */
        return false;
    } else if(params->latency > 2) { /** Number of intervals allowed to skip */
        return false;
    } else if(params->supervision_timeout > 100) { /** 10ms units */
        return false;
    }
    return true;
};

uint32_t ClientCallbacks::onPassKeyRequest(){
    return 123456;
};

bool ClientCallbacks::onConfirmPIN(uint32_t pass_key){
    return true;
};

void ClientCallbacks::onAuthenticationComplete(ble_gap_conn_desc* desc){
    if(!desc->sec_state.encrypted) {
        NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
        return;
    }
};

void AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    Logger::Info("[BLE] Advertised Device found: %s", advertisedDevice->toString().c_str());

    auto vecOfTuples = bleObj->GetAcceptedUUIDS();
    for (size_t i = 0; i < vecOfTuples.size(); ++i) {
      const auto it = vecOfTuples[i];

      if(advertisedDevice->isAdvertisingService(std::get<0>(it)))
      {
          NimBLEDevice::getScan()->stop();
          isScanning = false;
          toConnect = new ConnectTuple(advertisedDevice, std::get<0>(it), std::get<1>(it));
          toConnect->clientMode = 0;
          return;
      }
    }
};



void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  BleSensorData* data = (BleSensorData*)pData;

  if (pRemoteCharacteristic != nullptr){
    int id = g_remoteControls.getElementIdByUUID(pRemoteCharacteristic->getUUID());
    
    if (id >= 0){
      BleManager::remoteData[id].copy(data);
    }
  }
}

bool BleManager::connectToServer(ConnectTuple *tlp){
  NimBLEClient* pClient = nullptr;
  NimBLEAdvertisedDevice* advDevice = tlp->m_device;
  if(NimBLEDevice::getClientListSize()) {
    pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
    if(pClient){
        if(!pClient->connect(advDevice, false)) {
            tlp->connected = false;
            return false;
        }
    }else {
        pClient = NimBLEDevice::getDisconnectedClient();
    }
  }
  if(!pClient) {
    if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
        tlp->connected = false;
        return false;
    }

    pClient = NimBLEDevice::createClient();
    tlp->callbacks = new ClientCallbacks();
    tlp->callbacks->bleObj = this;
    pClient->setClientCallbacks(tlp->callbacks, false);
    /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
      *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
     *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
     *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
     */
    pClient->setConnectionParams(12,12,0,51);
    /** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
    pClient->setConnectTimeout(5);

    if (!pClient->connect(advDevice)) {
        NimBLEDevice::deleteClient(pClient);
        tlp->connected = false;
        return false;
    }
  }
  if(!pClient->isConnected()) {
    if (!pClient->connect(advDevice)) {
      Logger::Info("[BLE] Failed to connect");
      tlp->connected = false;
      return false;
    }
  }

  delay(300);

  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;

  pSvc = pClient->getService(tlp->m_service);
  if(pSvc) {
     
    pChr = pSvc->getCharacteristic( (tlp->m_characteristic) );
    if(pChr) {
      if(pChr->canNotify()) {
          if(!pChr->subscribe(true, notifyCB)) {
            Logger::Error("[BLE] The notification is a sussy baka");
            tlp->connected = false;
            NimBLEDevice::deleteClient(pClient);
            pClient->disconnect();
            return false;
          }
      }else{
        Logger::Error("[BLE] ye, there is not there wtf man");
        pClient->disconnect();
        NimBLEDevice::deleteClient(pClient);
        tlp->connected = false;
        return false;
      }
    }
  }else{
    Logger::Error("[BLE] The service is a sussy baka");
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    tlp->connected = false;
    return false;
  }

  tlp->m_client = pClient;
  clients[pClient->getPeerAddress()] = tlp;

  Logger::Info("[BLE] Done with this device!");
  return true;
}

int BleManager::getElementIdByUUID(NimBLEUUID id){
  auto vecOfTuples = GetAcceptedUUIDS();
  for (int i = 0; i < vecOfTuples.size(); ++i) {
    const auto it = vecOfTuples[i];
    if (id == std::get<1>(it)){
      return i;
    }
  }
  return -1;
}

bool BleManager::begin(){
  NimBLEDevice::init("");
  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

#ifdef ESP_PLATFORM
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
#else
    NimBLEDevice::setPower(9); /** +9db */
#endif

  NimBLEScan* pScan = NimBLEDevice::getScan();
  if (!pScan){
    return false;
  }
  for (int i = 0; i<MAX_BLE_CLIENTS;i++){
    remoteData[i] = BleSensorHandlerData();
  }

  AdvertisedDeviceCallbacks *cb = new AdvertisedDeviceCallbacks();
  if (!cb){
    return false;
  }
  cb->bleObj = this;

  pScan->setAdvertisedDeviceCallbacks(cb);

    /** Set scan interval (how often) and window (how long) in milliseconds */
  pScan->setInterval(45);
  pScan->setWindow(15);

  pScan->setActiveScan(true);
  /** Start scanning for advertisers for the scan time specified (in seconds) 0 = forever
  *  Optional callback for when scanning stops.
  */  
  m_started = true;
  NimBLEDevice::getScan()->start(0, scanEndedCB);
  return true;
}


void BleSensorHandlerData::updateButtons(){
  for (int i=0;i<MAX_BLE_BUTTONS;i++){
    if (real_inputButtonsStatus[i] == BUTTON_JUST_PRESSED){
      real_inputButtonsStatus[i] = BUTTON_PRESSED;
    }
    if (real_inputButtonsStatus[i] == BUTTON_JUST_RELEASED){
      real_inputButtonsStatus[i] = BUTTON_RELEASED;
    }
    if (buttons[i] != last_inputButtonsStatus[i]){
      if (buttons[i] == 1){
        real_inputButtonsStatus[i] = BUTTON_JUST_PRESSED;
      }else{
        real_inputButtonsStatus[i] = BUTTON_JUST_RELEASED;
      }
      last_inputButtonsStatus[i] = buttons[i];
    }
  }
}

void BleManager::updateButtons(){
  for (int i = 0; i<MAX_BLE_CLIENTS;i++){
    remoteData[i].updateButtons();
  }
}

void BleManager::beginScanning(){
  NimBLEDevice::getScan()->start(0, scanEndedCB);
  isScanning = true;
  m_canScan = true;
}

void BleManager::update(){
  if (!m_started){
    return;
  }
  if (toConnect != nullptr) {
    connectToServer(toConnect);
    toConnect = nullptr;
  }else{
    if (m_canScan){
      if (clients.size() < maxClients){
        if (!isScanning){
          isScanning = true;
          NimBLEDevice::getScan()->start(0, scanEndedCB);
        }
      }
     
      bool hasErase = false;
      NimBLEAddress toErase;
      for (auto &aux : clients){
        if (!aux.second->connected){
            toErase = aux.first;
            hasErase = true;
        }
      }
      
      if (hasErase){
        NimBLEDevice::deleteClient(clients[toErase]->m_client);
        delete clients[toErase]->callbacks;
        delete clients[toErase];
        clients.erase(toErase);
      }

    }
  }
}



int BleManager::acceptTypes(std::string service, std::string characteristic){
  m_acceptedUUIDs.emplace_back(std::make_tuple( NimBLEUUID(service), NimBLEUUID(characteristic)  ));
  return m_acceptedUUIDs.size() -1;
}

bool BleManager::hasChangedClients(){
  if (clientCount != clients.size()){
    clientCount = clients.size();
    return true;
  }
  return false;
}
bool BleManager::isElementIdConnected(int id){
  for (auto &obj : clients){
    if (obj.second->clientMode == id){
      return true;
    }
  }
  return false;
}


