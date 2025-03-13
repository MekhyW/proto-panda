#include <map>
#include <string>
#include <memory>
#include "esp32-hal.h"
#include "tools/logger.hpp"

template <typename T>
class PSRAMAllocator {
public:
    using value_type = T;

    PSRAMAllocator() noexcept {}

    template <class U>
    PSRAMAllocator(const PSRAMAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        void* ptr = ps_malloc(n * sizeof(T));
        if (!ptr) {
            throw std::bad_alloc();
        }
        auto Y = static_cast<T*>(ptr);;
        return Y;
    }

    void deallocate(T* p, std::size_t) noexcept {
        if (p) {
            heap_caps_free(p);
        }
    }

    bool operator!=(const PSRAMAllocator& other) const {
        return this != &other;
    }

    bool operator==(const PSRAMAllocator& other) const {
        return this == &other;
    }
};

using PSRAMString = std::basic_string<char, std::char_traits<char>, PSRAMAllocator<char>>;

struct PSRAMStringComparator {
    bool operator()(const PSRAMString& lhs, const PSRAMString& rhs) const {
        return lhs < rhs;
    }
};


using PSRAMMap = std::map<PSRAMString, PSRAMString, PSRAMStringComparator, PSRAMAllocator<std::pair<const PSRAMString, PSRAMString>>>;


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
    PSRAMMap store;    
};