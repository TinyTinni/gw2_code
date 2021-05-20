#include <Windows.h>
#include <Xinput.h>

#include <iostream>

#pragma comment (lib, "xinput.lib")

XINPUT_STATE g_controllers[XUSER_MAX_COUNT];

void CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    std::cout << "tick" << std::endl;
}

int main()
{
    std::cout << "Enumerate all Xinput devices:\n";
    DWORD dwResult;
    ZeroMemory(g_controllers, XUSER_MAX_COUNT * sizeof(XINPUT_STATE));
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
    {

        // Simply get the state of the controller from XInput.
        dwResult = XInputGetState(i, &g_controllers[i]);

        if (dwResult == ERROR_SUCCESS)
        {
            std::cout << "Controller " << i << " is connected.\n";
            // Controller is connected 
            XINPUT_VIBRATION vibration;
            ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
            vibration.wLeftMotorSpeed = 32000; // use any value between 0-65535 here
            vibration.wRightMotorSpeed = 16000; // use any value between 0-65535 here
            XInputSetState(i, &vibration);
            Sleep(2000);
            ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
            vibration.wLeftMotorSpeed = 65535; // use any value between 0-65535 here
            //vibration.wRightMotorSpeed =  65535; // use any value between 0-65535 here
            XInputSetState(i, &vibration);
            Sleep(2000);
            ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
            vibration.wLeftMotorSpeed = 0; // use any value between 0-65535 here
            vibration.wRightMotorSpeed = 0; // use any value between 0-65535 here
            XInputSetState(i, &vibration);
        }
        else
        {
            // Controller is not connected 
        }
    }

        // Create the timer queue.
    //HANDLE m_hTimerQueue = CreateTimerQueue();
    //if (NULL == m_hTimerQueue)
    //{
    //    printf("CreateTimerQueue failed (%d)\n", GetLastError());
    //    return 1;
    //}

    //HANDLE m_hTimer;

    //// Set a timer to call the timer routine in 10 seconds.
    //if (!CreateTimerQueueTimer(&m_hTimer, m_hTimerQueue,
    //    (WAITORTIMERCALLBACK)TimerRoutine, NULL, 500, 500, 0))
    //{
    //    printf("CreateTimerQueueTimer failed (%d)\n", GetLastError());
    //    return 1;
    //}


    //for(;;){}

    return 0;
}
