#pragma once 

#include <vector>
#include <stack>
#include <stdint.h>
#include <FS.h>
#include "config.hpp"

enum AnimationFrameAction{
    ANIMATION_NO_CHANGE,
    ANIMATION_FRAME_CHANGED,
    ANIMATION_FINISHED,
};

class AnimationSequence{
    public:
        AnimationSequence():m_duration(2500),m_frame(0),m_counter(0),m_repeat(-1),m_updateMode(0),m_storageId(-1){}
        std::vector<int> m_frames;
        AnimationFrameAction Update();
        inline int GetFrameId();
        int m_duration;
        int m_frame;
        int m_counter;
        int m_repeat;
        int m_updateMode;
        int m_storageId;
    private:
        AnimationFrameAction ChangeFrame();
        AnimationFrameAction SpeakFrame();
    
};

class Animation{
    public:
        Animation():m_animations(),m_shader(0),m_lastFace(0),m_runningFFT(false),m_runningTetris(false),m_runningPong(false),m_runningShooter(false),m_runningHexagon(false),m_isOnText(false),m_needFlip(false),m_isManaged(true),m_frameDrawDuration(0),m_frameLoadDuration(0),m_cycleDuration(0),m_mutex(xSemaphoreCreateMutex()){};

        void Update(File *file);

        void SetAnimation(int duration, std::vector<int> frames, int repeatTimes, bool dropAll, int externalStorageId=-1);
        void SetSpeakAnimation(int duration, std::vector<int> frames);
        void DrawFrame(File *file, int i);
        void DrawCurrentFrame(File *file){
            DrawFrame(file, m_lastFace);
        }

        bool PopAnimation();
        void MakeFlip();
        void SetShader(int id);

        bool getTextMode(){
            return m_isOnText;
        };
        bool needFlipScreen(){
            return m_needFlip;
        };
        void setManaged(bool v);
        bool isManaged(){
            return m_isManaged;
        }
        int getCurrentFace(){
            return m_lastFace;
        }

        int getCurrentAnimationStorage();

        float getFps(){
            return 1000000.0f/(float)m_cycleDuration;
        }

        int getAnimationStackSize(){
            return m_animations.size();
        }

        void setRainbowShader(bool enabled){
            if (enabled){
                m_shader = true;
            }else{
                m_shader = false;
            }
        }

        static unsigned char buffer[FILE_SIZE];

        uint32_t getDrawDuration() { return m_frameDrawDuration;};
        uint32_t getLoadDuration() { return m_frameLoadDuration;};
    private:
        std::stack<AnimationSequence> m_animations;

        bool internalUpdate(File *file, AnimationSequence &seq);
        int m_shader;
        int m_lastFace;
        bool m_runningFFT;
        bool m_runningTetris;
        bool m_runningPong;
        bool m_runningShooter;
        bool m_runningHexagon;
        bool m_isOnText;
        bool m_needFlip;
        bool m_isManaged;
        
        uint64_t m_frameDrawDuration;
        uint64_t m_frameLoadDuration;
        uint64_t m_cycleDuration;
        SemaphoreHandle_t m_mutex;
        
};


extern Animation g_animation;