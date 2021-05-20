#pragma once

#include <cstdint>

struct IDirect3DDevice9;

/* arcdps export table */
typedef struct arcdps_exports {
    uintptr_t size; /* size of exports table */
    uintptr_t sig; /* pick a number between 0 and uint64_t max that isn't used by other modules */
    char* out_name; /* name string */
    char* out_build; /* build string */
    void* wnd_nofilter; /* wndproc callback, fn(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) */
    void* combat; /* combat event callback, fn(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) */
    void* imgui; /* id3dd9::present callback, before imgui::render, fn(uint32_t not_charsel_or_loading) */
    void* options_end; /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
    void* combat_local;  /* combat event callback like area but from chat log, fn(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) */
    void* wnd_filter; /* wndproc callback like above, input filered using modifiers */
    void* options_windows; /* called once per 'window' option checkbox, with null at the end, non-zero return disables drawing that option, fn(char* windowname) */
} arcdps_exports;

/* combat event - see evtc docs for details, revision param in combat cb is equivalent of revision byte header */
typedef struct cbtevent {
    uint64_t time;
    uintptr_t src_agent;
    uintptr_t dst_agent;
    int32_t value;
    int32_t buff_dmg;
    uint32_t overstack_value;
    uint32_t skillid;
    uint16_t src_instid;
    uint16_t dst_instid;
    uint16_t src_master_instid;
    uint16_t dst_master_instid;
    uint8_t iff;
    uint8_t buff;
    uint8_t result;
    uint8_t is_activation;
    uint8_t is_buffremove;
    uint8_t is_ninety;
    uint8_t is_fifty;
    uint8_t is_moving;
    uint8_t is_statechange;
    uint8_t is_flanking;
    uint8_t is_shields;
    uint8_t is_offcycle;
    uint8_t pad61;
    uint8_t pad62;
    uint8_t pad63;
    uint8_t pad64;
} cbtevent;

enum cbtstatechange {
    CBTS_NONE, // not used - not this kind of event
    CBTS_ENTERCOMBAT, // src_agent entered combat, dst_agent is subgroup
    CBTS_EXITCOMBAT, // src_agent left combat
    CBTS_CHANGEUP, // src_agent is now alive
    CBTS_CHANGEDEAD, // src_agent is now dead
    CBTS_CHANGEDOWN, // src_agent is now downed
    CBTS_SPAWN, // src_agent is now in game tracking range
    CBTS_DESPAWN, // src_agent is no longer being tracked
    CBTS_HEALTHUPDATE, // src_agent has reached a health marker. dst_agent = percent * 10000 (eg. 99.5% will be 9950)
    CBTS_LOGSTART, // log start. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = 0x637261 (arcdps id)
    CBTS_LOGEND, // log end. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = 0x637261 (arcdps id)
    CBTS_WEAPSWAP, // src_agent swapped weapon set. dst_agent = current set id (0/1 water, 4/5 land)
    CBTS_MAXHEALTHUPDATE, // src_agent has had it's maximum health changed. dst_agent = new max health
    CBTS_POINTOFVIEW, // src_agent is agent of "recording" player
    CBTS_LANGUAGE, // src_agent is text language
    CBTS_GWBUILD, // src_agent is game build
    CBTS_SHARDID, // src_agent is sever shard id
    CBTS_REWARD, // src_agent is self, dst_agent is reward id, value is reward type. these are the wiggly boxes that you get
    CBTS_BUFFINITIAL, // combat event that will appear once per buff per agent on logging start (statechange==18, buff==18, normal cbtevent otherwise)
    CBTS_POSITION, // src_agent changed, cast float* p = (float*)&dst_agent, access as x/y/z (float[3])
    CBTS_VELOCITY, // src_agent changed, cast float* v = (float*)&dst_agent, access as x/y/z (float[3])
    CBTS_FACING, // src_agent changed, cast float* f = (float*)&dst_agent, access as x/y (float[2])
    CBTS_TEAMCHANGE, // src_agent change, dst_agent new team id
    CBTS_ATTACKTARGET, // src_agent is an attacktarget, dst_agent is the parent agent (gadget type), value is the current targetable state
    CBTS_TARGETABLE, // dst_agent is new target-able state (0 = no, 1 = yes. default yes)
    CBTS_MAPID, // src_agent is map id
    CBTS_REPLINFO, // internal use, won't see anywhere
    CBTS_STACKACTIVE, // src_agent is agent with buff, dst_agent is the stackid marked active
    CBTS_STACKRESET // src_agent is agent with buff, dst_agent is the stackid, value is the duration to reset to (also marks inactive)
};

//void dump_cbtevent(const cbtevent& ce)
//{
//    std::cout << "time			   " << ce.time << "\n";
//    std::cout << "src_agent		   " << ce.src_agent << "\n";
//    std::cout << "dst_agent		   " << ce.dst_agent << "\n";
//    std::cout << "value			   " << ce.value << "\n";
//    std::cout << "buff_dmg		   " << ce.buff_dmg << "\n";
//    std::cout << "overstack_value  " << ce.overstack_value << "\n";
//    std::cout << "skillid		   " << ce.skillid << "\n";
//    std::cout << "src_instid	   " << ce.src_instid << "\n";
//    std::cout << "dst_instid	   " << ce.dst_instid << "\n";
//    std::cout << "src_master_instid" << ce.src_master_instid << "\n";
//    std::cout << "dst_master_instid" << ce.dst_master_instid << "\n";
//    std::cout << "iff			   " << ce.iff << "\n";
//    std::cout << "buff			   " << ce.buff << "\n";
//    std::cout << "result		   " << ce.result << "\n";
//    std::cout << "is_activation	   " << ce.is_activation << "\n";
//    std::cout << "is_buffremove	   " << ce.is_buffremove << "\n";
//    std::cout << "is_ninety		   " << ce.is_ninety << "\n";
//    std::cout << "is_fifty		   " << ce.is_fifty << "\n";
//    std::cout << "is_moving		   " << ce.is_moving << "\n";
//    std::cout << "is_statechange   " << ce.is_statechange << "\n";
//    std::cout << "is_flanking	   " << ce.is_flanking << "\n";
//    std::cout << "is_shields       " << ce.is_shields << "\n";
//    std::cout << "is_offcycle	   " << ce.is_offcycle << "\n";
//    std::cout << "pad61			   " << ce.pad61 << "\n";
//    std::cout << "pad62			   " << ce.pad62 << "\n";
//    std::cout << "pad63			   " << ce.pad63 << "\n";
//    std::cout << "pad64			   " << ce.pad64 << "\n";
//}

/* agent short */
typedef struct ag {
    char* name; /* agent name. may be null. valid only at time of event. utf8 */
    uintptr_t id; /* agent unique identifier */
    uint32_t prof; /* profession at time of event. refer to evtc notes for identification */
    uint32_t elite; /* elite spec at time of event. refer to evtc notes for identification */
    uint32_t self; /* 1 if self, 0 if not */
    uint16_t team; /* sep21+ */
} ag;