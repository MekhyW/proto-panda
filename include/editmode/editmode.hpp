#pragma once 
#include "config.hpp"

void startWifiServer();

class EditMode{
    public:
        EditMode():m_running(false){};
        void CheckBeginEditMode();
        bool IsOnEditMode();
        void LoopEditMode();
        void DoBegin(bool useSSID);
    private:
        bool m_running;
};