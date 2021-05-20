#pragma once

#include <cstdint>

class rumble_devices_impl
{
    int n_num_devices;
    rumble_devices_impl() noexcept;

    friend rumble_devices_impl rumble_devices() noexcept;
public:
    
    
    /// returns the number of connected devices
    int number()const noexcept;
    /// try to connect to all devices. This will be done once at startup
    void enumerate() noexcept;

    /// disable all rumbles
    void disable_all() noexcept;

    /// set the rumble level, from 0.f (no rumble) to 1.f (100% rumble)
    void set_rumble(size_t device_id, float level) noexcept;
};


rumble_devices_impl rumble_devices() noexcept;