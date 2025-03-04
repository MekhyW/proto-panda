#include <map>
#include <string>
#include <memory>
#include "esp32-hal.h"


template <typename T>
class PSRAMAllocator {
public:
    using value_type = T;

    PSRAMAllocator() noexcept {}

    template <class U>
    PSRAMAllocator(const PSRAMAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        return static_cast<T*>(ps_malloc(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }

    bool operator!=(const PSRAMAllocator& other) const {
        return this != &other;
    }
};

using PSRAMString = std::basic_string<char, std::char_traits<char>, PSRAMAllocator<char>>;


class PersistentDictionary {
public:
    PersistentDictionary(){};

    void Save();
    void Load();
    void Format();

    void set(std::string key, const char* val);

    const char* get(std::string key);

    void del(std::string key);

private:
    std::map<PSRAMString,PSRAMString, std::less<PSRAMString>, PSRAMAllocator<char>>  store;    
};