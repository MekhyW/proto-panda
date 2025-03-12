#include "drawing/framerepository.hpp"
#include "drawing/animation.hpp"
#include "tools/logger.hpp"

#include "tools/oledscreen.hpp"


#include <FFat.h>

template<typename... arg> void FrameBufferCriticalError(File *bulkFile, const char *c, const arg&... a )
{
    char auxBuff[1200];
    bulkFile->close();   
    FFat.remove("/frames.bulk");
    sprintf(auxBuff, c, a...);
    OledScreen::CriticalFail(auxBuff);
    for(;;){}
}

bool FrameRepository::Begin(){
    m_mutex = xSemaphoreCreateMutex();
    Logger::Info("Starting FFAT");
    if (!FFat.begin(true)) {
        Logger::Info("FFat Mount Failed");
        Logger::Info("Formatting FFat");
        FFat.format();
        if (!FFat.begin(true)) {
            Logger::Info("FFat Mount Failed");
            return false;
        }
    }

    displayFFATInfo();
    bulkFile = FFat.open("/frames.bulk", FILE_READ);
    if (!bulkFile) {
        composeBulkFile();
    }

    if (!loadCachedData()){
        composeBulkFile();
    }

    return true;
}
// 1mb = 125000bytes
void FrameRepository::displayFFATInfo(){
    Logger::Info("FFAT totalBytes=%2.2f Mb", FFat.totalBytes()/1000000.0f);
    Logger::Info("FFAT usedBytes=%2.2f Mb", FFat.usedBytes()/1000000.0f);
    Logger::Info("FFAT Avaliable=%2.2f%%", (1.0f - (FFat.usedBytes()/(float)FFat.totalBytes()) ) * 100.0f );
}


void FrameRepository::extractModes(JsonVariant &element, bool &flip_left, int &color_scheme_left){
    
    if (element.containsKey("flip_left") && element["flip_left"].is<bool>()) {
        flip_left = element["flip_left"];
    }
                
    if (element.containsKey("color_scheme_left") && element["color_scheme_left"].is<const char*>()) {
        const char* schemestr = element["color_scheme_left"];
        std::string aux(schemestr);
        if (aux == "rgb") color_scheme_left = 0;
        else if (aux == "rbg") color_scheme_left = 1;
        else if (aux == "grb") color_scheme_left = 2;
        else if (aux == "gbr") color_scheme_left = 3;
        else if (aux == "brg") color_scheme_left = 4;
        else if (aux == "bgr") color_scheme_left = 5;
        else OledScreen::CriticalFail("Invalid 'color_scheme_left' mode.");
    }
}

bool FrameRepository::loadCachedData(){
    File file = SD.open( "/cache/cache.json", "r" );
    if( !file ) {
        Logger::Error("There is no cache build");
        return false;
    }

    SpiRamAllocator allocator;
    JsonDocument  json_doc(&allocator);
    auto err = deserializeJson( json_doc, file );
    file.close();
    if( err ) {
        Logger::Error("Cache seems to be corrupted");
        SD.remove("/cache/cache.json");
        return false;
    }

    if ( !json_doc.containsKey("frame_count") || !json_doc["frame_count"].is<int>() ) {
        json_doc.clear();   
        Logger::Error("Cache structure is corrupted");
        return false;
    }
    int frame_count = json_doc["frame_count"];
    if ( (bulkFile.size() % FILE_SIZE) != 0 || (bulkFile.size() / FILE_SIZE) != frame_count){
        json_doc.clear();   
        Logger::Error("Mismatched file bulk size and expected frame count: %d %d %d", (bulkFile.size() % FILE_SIZE), (bulkFile.size() / FILE_SIZE), frame_count);
        return false;
    }
    File conf = SD.open( "/config.json" );
    if( !conf ) {
        OledScreen::CriticalFail("Can't open config.json");
        for(;;){}
        return false;
    }

    JsonDocument  json_doc_aux(&allocator);
    auto err2 = deserializeJson( json_doc_aux, conf );
    conf.close();
    if( err2 ) {
       OledScreen::CriticalFail(err.c_str());
       for(;;){}
       return false;
    }

    JsonArray franesJson = json_doc_aux["frames"];
    int actualFrameCount = calculateMaxFrames(franesJson);
    json_doc_aux.clear();
    if (actualFrameCount != frame_count){
        Logger::Info("Mismatched cached frames and config frames, need rebuld. Stored: %d expected: %d", frame_count, actualFrameCount);
        return false;
    }

    if ((bulkFile.size() / FILE_SIZE) != actualFrameCount){
        Logger::Info("Mismatched stored frames and config frames, need rebuld. Stored: %d expected: %d", (bulkFile.size() / FILE_SIZE), actualFrameCount);
        return false;
    }

    JsonObject frame_alias = json_doc["frame_alias"];
    for (JsonPair alias : frame_alias) {
        m_offsets[alias.key().c_str()] = alias.value().as<int>();
    }

    return true;
}

int FrameRepository::calculateMaxFrames(JsonArray &franesJson){
    
    int frameCount = 0;
    int jsonElement;
    for (JsonVariant element : franesJson) {
        if (element.is<JsonObject>()) {
            if ( element.containsKey("file")){
                frameCount++;
            }else{
                if (!element.containsKey("pattern") || !element["pattern"].is<const char*>() ) {
                    FrameBufferCriticalError(&bulkFile, "Missing 'pattern' on element %d", jsonElement);
                }
                if (!element.containsKey("from") || !element["from"].is<int>() ) {
                    FrameBufferCriticalError(&bulkFile, "Missing 'from' on element %d", jsonElement);
                }
                if (!element.containsKey("to") || !element["to"].is<int>()) {
                    FrameBufferCriticalError(&bulkFile, "Missing 'to' on element %d", jsonElement);
                }
                int from = element["from"];
                int to = element["to"];
                frameCount += (to-from+1);
            }
        }else{
            frameCount++;
        }
        jsonElement++;
    }
    return frameCount;
}

void FrameRepository::composeBulkFile(){
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    uint64_t start = micros();
    
    File file = SD.open( "/config.json" );
    if( !file ) {
        OledScreen::CriticalFail("Can't open config.json");
        for(;;){}
    }

    m_offsets.clear();

    SpiRamAllocator allocator;
    JsonDocument  json_doc(&allocator);
    auto err = deserializeJson( json_doc, file );
    file.close();
    if( err ) {
        OledScreen::CriticalFail(err.c_str() );
    }

    File fdesc = SD.open( "/frame_description.txt",  FILE_WRITE);

    FFat.remove("/frames.bulk");
    bulkFile = FFat.open("/frames.bulk", FILE_WRITE);

    if (!json_doc.containsKey("frames")){
        OledScreen::CriticalFail("Missing 'frames' in json!");
    }

    JsonArray franesJson = json_doc["frames"];
    char headerFileName[1024];
    char miniHBuffer[1100];
    
    int maxFrames = calculateMaxFrames(franesJson);

    if (maxFrames <= 0){
        OledScreen::CriticalFail("Maybe you forgot to add frames?!");
    }

    int fileIdx = 1;
    int jsonElement = 0;
    for (JsonVariant element : franesJson) {
        if (element.is<JsonObject>()) {
            if ( element.containsKey("alias") &&  element["alias"].is<const char*>() ) {
                const char *content = element["alias"];
                m_offsets[content] = fileIdx-1;
            }
            if ( element.containsKey("file") &&  element["file"].is<const char*>() ) {
                const char *filePath = element["file"].as<const char*>();
                if (strlen(filePath) >= 1024){
                    FrameBufferCriticalError(&bulkFile, "Path name is too big at element %d", filePath, jsonElement);
                }
                bool flipe_left = false; 
                int color_scheme_left = 0;
                extractModes(element, flipe_left, color_scheme_left);

                sprintf(miniHBuffer, "Copy frame %d\n%s", fileIdx+1, filePath);
                OledScreen::DrawProgressBar(fileIdx+1, maxFrames, miniHBuffer);
                if (!decodeFile(filePath, flipe_left, color_scheme_left)){
                    FrameBufferCriticalError(&bulkFile, "Failed to decode %s at element %d", filePath, jsonElement);
                }
                
                fdesc.printf("%d = %s\n", fileIdx, filePath);
                fileIdx++;
            }else{
               
                const char* pattern = element["pattern"];
                if (strlen(pattern) >= 800){
                    FrameBufferCriticalError(&bulkFile, "Pattern name too big on element %d", jsonElement);
                }
                int from = element["from"];
                int to = element["to"];
                bool flipe_left = false; 
                int color_scheme_left = 0;
                extractModes(element, flipe_left, color_scheme_left);
                
                for (int i=from;i<=to;i++){
                    sprintf(headerFileName, pattern, i);
                    sprintf(miniHBuffer, "Copy frame %d\n%s", fileIdx+1, headerFileName);
                    OledScreen::DrawProgressBar(fileIdx+1, maxFrames, miniHBuffer);
                    if (!decodeFile(headerFileName, flipe_left, color_scheme_left)){
                        FrameBufferCriticalError(&bulkFile, "Failed decode %s at element %d", headerFileName, jsonElement);
                    }
                    fdesc.printf("%d = %s\n", fileIdx, headerFileName);
                    fileIdx++;
                }
            }
        } else if (element.is<const char*>()) {
            const char *filePath = element.as<const char*>();
            if (strlen(filePath) >= 1024){
                FrameBufferCriticalError(&bulkFile, "Path name is too big at element %d", filePath, jsonElement);
            }
            sprintf(miniHBuffer, "Copy frame %d\n%s", fileIdx+1, filePath);
            OledScreen::DrawProgressBar(fileIdx+1, maxFrames, miniHBuffer);
            if (!decodeFile(filePath, false, 0)){
                FrameBufferCriticalError(&bulkFile, "Failed to decode %s at element %d", filePath, jsonElement);
            }
            fdesc.printf("%d = %s\n", fileIdx, filePath);
            fileIdx++;
        }else{
            OledScreen::CriticalFail("Error on config.json. Expected in folders the element be an object or string.");
        }
        jsonElement++;
    }
    m_frameCount = fileIdx-1;
    json_doc.clear();
    fdesc.close();
    bulkFile.close();
    bulkFile = FFat.open("/frames.bulk", FILE_READ);
    generateCacheFile();
    Logger::Info("Finished writing data to internal storage. in %d uS", micros()-start);



    xSemaphoreGive(m_mutex);    
}

void FrameRepository::generateCacheFile() {
    File cache = SD.open( "/cache/cache.json", "w" );
    if( !cache ) {
        OledScreen::CriticalFail("Can't open /cache/cache.json, cant write!");
        for(;;){}
    }

    SpiRamAllocator allocator;
    JsonDocument  json_doc(&allocator);

    json_doc["frame_count"] = m_frameCount;

    JsonObject frames = json_doc["frame_alias"].to<JsonObject>();

    for (const auto& pair : m_offsets) {
        frames[pair.first.c_str()] = pair.second;
    }

    // Serialize JSON to file
    if (serializeJson(json_doc, cache) == 0) {
        Serial.println("Failed to write to file.");
    }

    json_doc.clear();

    cache.close();
}

std::string PngErrorToString(int error) {
    switch (error) {
        case 0: return "PNG_SUCCESS";
        case 1: return "PNG_INVALID_PARAMETER";
        case 2: return "PNG_DECODE_ERROR";
        case 3: return "PNG_MEM_ERROR";
        case 4: return "PNG_NO_BUFFER";
        case 5: return "PNG_UNSUPPORTED_FEATURE";
        case 6: return "PNG_INVALID_FILE";
        case 7: return "PNG_TOO_BIG";
        case 998: return "FILE_NOT_FOUND";
        case 999: return "SIZE MISMATCH";
        default: return "UNKNOWN_ERROR";
    }
}

bool FrameRepository::decodeFile(const char *pathName, bool flip, int color_mode){
    int lastRcError = 0;
    uint16_t* res = Storage::DecodePNG(pathName, lastRcError);
    if (res) {
        uint8_t meme = flip ? 1 : 0;
        bulkFile.write(&meme, sizeof(uint8_t));
        meme = color_mode;
        bulkFile.write(&meme, sizeof(uint8_t));
        int rd = bulkFile.write((const uint8_t*)res, FILE_SIZE_BULK_SIZE);
        if (rd != FILE_SIZE_BULK_SIZE){
            Logger::Info("Error writing data in the FFAT: exptected %d got %d\n",FILE_SIZE, rd);
        }
        Storage::FreeBuffer(res);
        return true;
    } else {
        FrameBufferCriticalError(&bulkFile, "Failed to decode %s with error %s", pathName, PngErrorToString(lastRcError).c_str());        
        return false;
    }
}

File *FrameRepository::takeFile(){
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    return &bulkFile;
};

void FrameRepository::freeFile(){
    xSemaphoreGive(m_mutex);
};
