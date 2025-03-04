#pragma once
#include "lua/LuaWrapper.h"
#include "lua/persistentdict.hpp"

class LuaInterface{
    public:
        bool Start();

        bool CallFunction(const char *functionName);
        template<typename... Args> bool CallFunctionT(const char *functionName, Args&&... args){
            return m_lua->callLuaFunction(functionName, args...);
        };            

        const char *getLastError(){
            return lastError.c_str();
        }
        bool DoString(const char *script);
        bool LoadFile(const char *functionName);
        static void luaCallbackError(const char *errMsg);
        static bool HaltIfError;
    private:
        static std::string lastError;
        void RegisterMethods();
        void RegisterConstants();
        LuaWrapper *m_lua;
};
