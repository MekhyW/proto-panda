#pragma once 
#include "config.hpp"
#ifdef ENABLE_EDIT_MODE
void startWifiServer(int port);

class EditMode{
    public:
        EditMode():m_running(false){};
        void CheckBeginEditMode();
        bool IsOnEditMode();
        void LoopEditMode();
        void DoBegin(bool connectToWifi);
    private:
        bool m_running;
};
#endif