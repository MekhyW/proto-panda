#include "bluetooth/characteristicshandler.hpp"
#ifdef ENABLE_BLE
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"

void BleCharacteristicsHandler::AddMessage(int cliID, int id, uint8_t* pData, size_t length, bool isNotify){
    if (xSemaphoreTake(queueMutex, portTICK_PERIOD_MS * 50) != pdTRUE){
        return;
    }
    if (dataQueue.size() > 100) {
        dataQueue.pop();
    }
    dataQueue.emplace(cliID, id, pData, length);
    xSemaphoreGive(queueMutex);
}

void BleCharacteristicsHandler::processMessageAndPop(){
    PSRAMVector<uint8_t> vec;
    int id = -1;
    int cliId = -1;
            
    if (xSemaphoreTake(queueMutex, portTICK_PERIOD_MS * 25) != pdTRUE){
        return;
    }

    vec = dataQueue.front().message;
    id = dataQueue.front().m_id;
    cliId = dataQueue.front().m_CliId;
    dataQueue.pop();
    xSemaphoreGive(queueMutex);
    #ifdef ENABLE_LUA
    if (luaCallback != nullptr){
        luaCallback->callLuaFunction(id, cliId, vec);
    }
    #endif
}
void BleCharacteristicsHandler::SendMessages(){
    if (m_stream){
        while (!dataQueue.empty()){
            processMessageAndPop();
        }
    }else{
        if (!dataQueue.empty()){
            processMessageAndPop();
        }
    }
}
#endif