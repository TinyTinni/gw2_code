#pragma once
#include <cstdint>
#include <chrono>

#include <Windows.h>

#include "cbt.h"

#include <boost/circular_buffer.hpp>


class player
{
    /*remove*/
    const uint32_t m_player_id;
    uint32_t m_dmg_done = 0;
    uint32_t m_healing_done = 0;
    std::chrono::time_point<std::chrono::system_clock> m_combat_start_time;
    uint64_t m_start_time;
    /**/


    // accumulated dmg/healing done during the current tick
    uint32_t m_dmg_tick = 0;
    uint32_t m_healing_tick = 0;

    // saved the accumulated dmg/healing per tick
    boost::circular_buffer<uint32_t> m_dmg_buffer;
    boost::circular_buffer<uint32_t> m_healing_buffer;

    // update a time tick
    void time_tick() noexcept;
    // update the rumble function depending on the average
    void update_rumble() noexcept;

    // returns the average dmg/healing done during the specified time window
    float average_healing() const noexcept;
    float average_dmg() const noexcept;

    HANDLE m_hTimer = NULL;
    HANDLE m_hTimerQueue = NULL;
    HANDLE m_eventDone = NULL;
    uint32_t m_update_interval_ms = 500;
    //bool timer_started() { return m_hTimer != NULL; }
    void start_timer()noexcept;
    static void CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);



public:
    player(uint32_t player_id)noexcept;
    ~player()noexcept;
    uint32_t get_id()const noexcept { return m_player_id; }

    void add_healing(uint32_t heal) noexcept;
    void add_damage(uint32_t dmg) noexcept;

    /*remove*/
    void begin_combat(uint64_t time);
    void end_combat(uint64_t time);
    void combat_visitor(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision);
    /**/
};
