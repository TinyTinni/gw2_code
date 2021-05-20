#include "player.h"

#include <thread>
#include <numeric>

#include <iostream>

#include "options.h"
#include "rumble_devices.h"

void player::TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (!lpParam)
        return; //error
    player* p = reinterpret_cast<player*>(lpParam);
    p->time_tick(); 
}

void player::time_tick() noexcept
{

    m_dmg_buffer.push_back(m_dmg_tick);
    m_healing_buffer.push_back(m_healing_tick);

    std::cout << "tick: " << m_healing_tick << " healing (" << average_healing() << ")\t" << "rumble: " << average_healing()*1.f / options().max_hps << "\n";

    m_dmg_tick = 0;
    m_healing_tick = 0;
    update_rumble();
}

float player::average_healing() const noexcept
{
    return std::reduce(std::begin(m_healing_buffer), std::end(m_healing_buffer))* 1.f / m_healing_buffer.size();
}

float player::average_dmg() const noexcept
{
    return std::reduce(std::begin(m_dmg_buffer), std::end(m_dmg_buffer))* 1.f / m_dmg_buffer.size();
}

void player::start_timer() noexcept
{
    // Create the timer queue.
    m_hTimerQueue = CreateTimerQueue();
    if (NULL == m_hTimerQueue)
    {
        printf("CreateTimerQueue failed (%d)\n", GetLastError());
        return;
    }

    // Set a timer to call the timer routine in 10 seconds.
    if (!CreateTimerQueueTimer(&m_hTimer, m_hTimerQueue,
        (WAITORTIMERCALLBACK)TimerRoutine, this, m_update_interval_ms, m_update_interval_ms, 0))
    {
        printf("CreateTimerQueueTimer failed (%d)\n", GetLastError());
        return;
    }


}

void player::update_rumble() noexcept
{
    rumble_devices().set_rumble(options().selected_device, average_healing()*1.f/options().max_hps);
}

player::player(uint32_t player_id) noexcept
    :m_player_id(player_id),
    m_dmg_buffer(10),
    m_healing_buffer(20)
{
    rumble_devices();
    start_timer();
}

player::~player() noexcept
{
    if (m_hTimerQueue)
        DeleteTimerQueue(m_hTimerQueue);
}

void player::add_healing(uint32_t heal) noexcept
{
    std::cout << "add healing: " << heal << "\n";
    m_healing_tick += heal;
}

void player::add_damage(uint32_t dmg) noexcept
{
    std::cout << "add dmg: " << dmg << "\n";
    m_dmg_tick += dmg;
}

void player::begin_combat(uint64_t time)
{
    //std::cout << "start combat" << std::endl;
    //m_combat_start_time = std::chrono::system_clock::now();
    //
    //m_start_time = time;
}

void player::end_combat(uint64_t time)
{
    //std::cout << "end combat\n";
    //auto combat_duration = std::chrono::system_clock::now() - m_combat_start_time;
    //auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(combat_duration).count();

    //auto time_dura = time - m_start_time;
    //std::cout << "hps: " << m_healing_done *1000.f * 1.f / time_dura << "\n";
    //std::cout << "dps: " << m_dmg_done *1000.f * 1.f / time_dura << "\n";
    //std::cout << "time: " << time_dura << std::endl;
    //m_healing_done = m_dmg_done = 0;
    //std::cout << std::flush;
}

void player::combat_visitor(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision)
{
    if (src->id != m_player_id)
        return;
    if (ev->buff)
    {
        if (ev->buff_dmg > 0)
            m_dmg_done += ev->buff_dmg;
        else if (ev->buff_dmg < 0)
            m_healing_done -= ev->buff_dmg;
        // buff_dmg == 0 -> nothing
    }

    if (!ev->buff && !ev->is_activation && !ev->is_buffremove)
    {
        if (ev->value > 0)
            m_dmg_done += ev->value;
        else if (ev->value < 0)
            m_healing_done -= ev->value;
    }

    add_healing(m_healing_done);
    add_damage(m_dmg_done);
    m_dmg_done = m_healing_done = 0;
}
