#include "Time.h"


LARGE_INTEGER Time::CpuFrequency = {};
LARGE_INTEGER Time::PrevFrequency = {};
LARGE_INTEGER Time::CurrentFrequency = {};
float Time::DeltaTimeValue = 0.0f;
bool Time::TimeStop;
void Time::Initialize()
{
    // Cpu 고유 진동수
    QueryPerformanceFrequency(&CpuFrequency);

    // 프로그램이 시작 했을 때 현재 진동수
    QueryPerformanceCounter(&PrevFrequency);
}

void Time::Update()
{
   
    QueryPerformanceCounter(&CurrentFrequency);

    float differenceFrequency
        = static_cast<float>(CurrentFrequency.QuadPart - PrevFrequency.QuadPart);
    DeltaTimeValue = differenceFrequency / static_cast<float>(CpuFrequency.QuadPart);
    PrevFrequency.QuadPart = CurrentFrequency.QuadPart;
    
   
}

void Time::SetTimeStop(bool a)
{
    TimeStop = a;
}
float Time::DeltaTime() { return TimeStop ? 0.0f : DeltaTimeValue; }
