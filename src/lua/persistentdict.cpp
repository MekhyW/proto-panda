#include "lua/persistentdict.hpp"
#include <FFat.h>
#include <stdio.h>
#include <string.h>
#include "tools/logger.hpp"

void PersistentDictionary::Format(){
    store.clear();
    FFat.remove("/dict.bin");
}

void PersistentDictionary::Save(){
    File outfile = FFat.open("/dict.bin", FILE_WRITE);
    if (outfile) {
        for (auto& kv : store) {
            size_t key_size = kv.first.size();
            outfile.write((uint8_t*)&key_size, sizeof(size_t));
            outfile.write((uint8_t*)kv.first.c_str(), sizeof(char) * key_size);
            size_t val_size = kv.second.size();
            outfile.write((uint8_t*)&val_size, sizeof(val_size));
            outfile.write((uint8_t*)kv.second.c_str(), sizeof(char) * val_size);
            //Logger::Info("Saved: [%s]=[%s]!", kv.first.c_str(), kv.second.c_str());
        }
        outfile.close();
    }
}
void PersistentDictionary::Load(){
    Logger::Info("Loading dict!");
    File outfile = FFat.open("/dict.bin", FILE_READ);
        if (outfile) {
            while (true) {
                size_t key_size=1;

                if (outfile.read((uint8_t*)&key_size, sizeof(key_size)) != sizeof(size_t)){
                    break;
                }

                char *key_content = (char *)ps_malloc(key_size+1);
                if (outfile.read((uint8_t*)key_content, sizeof(char) * key_size) != (sizeof(char) * key_size) ){
                    break;
                }

                key_content[key_size] = '\0';
                
                if (strlen(key_content) != key_size){
                    return;
                }

                size_t val_size=1;
                if (outfile.read((uint8_t*)&val_size, sizeof(val_size)) != sizeof(size_t)){
                    break;
                }

                char *value_content = (char *)ps_malloc(val_size+1);

                if (outfile.read((uint8_t*)value_content, sizeof(char) * val_size) != (sizeof(char) * val_size) ){
                    break;
                }

                value_content[val_size] = '\0';

                if (strlen(value_content) != val_size){
                    Logger::Info("Corruption on the persistent dictionary!");
                    return;
                }

                //Logger::Info("Loaded: [%s]=[%s]!", key_content, value_content);
                store[key_content] = value_content;
            }
            outfile.close();
        }else{
            Logger::Info("There is no dict!");
        }
}

void PersistentDictionary::set(std::string key, const char* val){
    store[key.c_str()] = val;
    //Force it to reallocate in psram, since the starting is 16 bytes on default heap
    store[key.c_str()].reserve(32);
}

const char* PersistentDictionary::get(std::string key){
    if (store.find(key.c_str()) != store.end()) {
        return store[key.c_str()].c_str();
    } else {
        return ""; // or handle the case where the key doesn't exist
    }
}

void PersistentDictionary::del(std::string key){
    store.erase(key.c_str());
}
