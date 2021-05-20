#include "rumble_devices.h"

#include <Windows.h>
#include <Xinput.h>

#include <iostream>

#pragma comment (lib, "xinput.lib")

XINPUT_STATE g_controllers[XUSER_MAX_COUNT];


rumble_devices_impl::rumble_devices_impl() noexcept
{
    enumerate();
    std::cout << "found devices: " << number();
}

int rumble_devices_impl::number() const noexcept
{
    return n_num_devices;
}

void rumble_devices_impl::enumerate() noexcept
{
    n_num_devices = 0;
    ZeroMemory(g_controllers, XUSER_MAX_COUNT * sizeof(XINPUT_STATE));
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
    {

        // Simply get the state of the controller from XInput.
        const DWORD dwResult = XInputGetState(i, &g_controllers[i]);

        if (dwResult == ERROR_SUCCESS)
        {
            ++n_num_devices;
        }
        else
        {
            // Controller is not connected 
        }
    }
}

void rumble_devices_impl::disable_all() noexcept
{
    for (DWORD i = 0; i < n_num_devices; ++i)
        set_rumble(i, 0);
}

void rumble_devices_impl::set_rumble(size_t device_id, float level) noexcept
{
    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    vibration.wLeftMotorSpeed = 65535*min(level, 1.f); // use any value between 0-65535 here
    vibration.wRightMotorSpeed = 65535* min(level, 1.f); // use any value between 0-65535 here
    XInputSetState(device_id, &vibration);
}

rumble_devices_impl rumble_devices() noexcept
{
    static rumble_devices_impl rd;
    return rd;
}
