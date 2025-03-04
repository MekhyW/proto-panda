#pragma once
#include "config.hpp"
#include <ArduinoJson.h>
#include <string>
#include <map>
#include <FS.h>

struct SpiRamAllocator : ArduinoJson::Allocator {
  void* allocate(size_t size) override {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) override {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) override {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};

class FrameRepository{
    public:
        FrameRepository(){};
        bool Begin();

        File *takeFile();
        void freeFile();
        void composeBulkFile();
        void displayFFATInfo();

        int getAlias(std::string str){
            return m_offsets[str];
        }
    private:
        void extractModes(JsonVariant &element, bool &flip, int &scheme);
        bool decodeFile(const char *pathName, bool flip, int scheme);
        bool loadCachedData();
        int calculateMaxFrames(JsonArray &franesJson);
        void generateCacheFile();
        int m_frameCount;
        std::map<std::string, int> m_offsets;
        SemaphoreHandle_t m_mutex;
        File bulkFile;
};      




