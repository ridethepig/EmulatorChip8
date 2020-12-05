#pragma once
#include <windows.h>
#include <SDL.h>

class Timer
{
public:
    Timer() : mStartTicks(0), mPausedTicks(0), mPaused(false), mStarted(false) {}

    void start();
    void stop();
    void pause();
    void unpause();
    
    DWORD getTicks();

    bool isStarted() const { return mStarted; }
    bool isPaused() const { return mPaused; }

private:
    DWORD mStartTicks;

    DWORD mPausedTicks;

    bool mPaused;
    bool mStarted;
};