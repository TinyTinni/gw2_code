#include <cstdint>
#include <cstdio>
#include <Windows.h>
#include <iostream>
#include <memory>

#include "io_console.h"
#include "imgui/imgui.h"
#include "cbt.h"

#include "player.h"
#include "options.h"

/* proto/globals */
uint32_t cbtcount = 0;
arcdps_exports arc_exports;
char* arcvers;
void dll_init(HANDLE hModule);
void dll_exit();
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9* id3dd9);
extern "C" __declspec(dllexport) void* get_release_addr();
arcdps_exports* mod_init();
uintptr_t mod_release();
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision);
uintptr_t mod_combat_track_state_changes(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision);

std::unique_ptr<player> g_player;

/* dll main -- winapi */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
    switch (ulReasonForCall) {
        case DLL_PROCESS_ATTACH: dll_init(hModule); break;
        case DLL_PROCESS_DETACH: dll_exit(); break;

        case DLL_THREAD_ATTACH:  break;
        case DLL_THREAD_DETACH:  break;
    }
    return 1;
}

/* dll attach -- from winapi */
void dll_init(HANDLE hModule) {
    return;
}

/* dll detach -- from winapi */
void dll_exit() {
    return;
}

/* export -- arcdps looks for this exported function and calls the address it returns on client load */
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9* id3dd9) {
    arcvers = arcversionstr;
    ImGui::SetCurrentContext((ImGuiContext*)imguicontext);
    return mod_init;
}

/* export -- arcdps looks for this exported function and calls the address it returns on client exit */
extern "C" __declspec(dllexport) void* get_release_addr() {
    arcvers = 0;
    return mod_release;
}

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init() {
    /* demo */
    //AllocConsole();
    RedirectIOToConsole();

    /* big buffer */
    //char buff[4096];
    //char* p = &buff[0];
    //p += _snprintf(p, 400, "==== mod_init ====\n");
    //p += _snprintf(p, 400, "arcdps: %s\n", arcvers);

    /* print */
    //DWORD written = 0;
    //HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
    //WriteConsoleA(hnd, &buff[0], (DWORD)(p - &buff[0]), &written, 0);
    std::cout << "==== mod_init ====\n" << "arcdps: " << arcvers << "\n";

    /* redirect to file*/
    //std::ofstream file;
    //file.open("log.txt");
    //std::streambuf* sbuf = std::cout.rdbuf();
    //std::cout.rdbuf(file.rdbuf());
    //cout is now pointing to a file

    /* for arcdps */
    memset(&arc_exports, 0, sizeof(arcdps_exports));
    arc_exports.sig = 0xFFFA;
    arc_exports.size = sizeof(arcdps_exports);
    arc_exports.out_name = const_cast<char*>("combatdemo");
    arc_exports.out_build = const_cast<char*>("0.1");
    arc_exports.wnd_nofilter = mod_wnd;
    arc_exports.combat = reinterpret_cast<void*>(mod_combat_track_state_changes);
    arc_exports.combat_local = reinterpret_cast<void*>(mod_combat);
    arc_exports.options_end = reinterpret_cast<void*>(show_imgui_option_window);
    //arc_exports.options_windows = reinterpret_cast<void*>(options_window);
    //arc_exports.size = (uintptr_t)"error message if you decide to not load, sig must be 0";
    return &arc_exports;
}

/* release mod -- return ignored */
uintptr_t mod_release() {
    FreeConsole();
    return 0;
}

/* window callback -- return is assigned to umsg (return zero to not be processed by arcdps or game) */
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    /* big buffer */
    //char buff[4096];
    //char* p = &buff[0];

    /* common */
    //p += _snprintf(p, 400, "==== wndproc %llx ====\n", (uintptr_t)hWnd);
    //p += _snprintf(p, 400, "umsg %u, wparam %lld, lparam %lld\n", uMsg, wParam, lParam);
    //std::cout << "==== wndproc %llx ====\n";

    /* print */
    //DWORD written = 0;
    //HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
    //WriteConsoleA(hnd, &buff[0], p - &buff[0], &written, 0);
    return uMsg;
}

/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat_track_state_changes(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) {
    if (!(!g_player || g_player->get_id() == src->id))
        return 0;
    
    if (!ev)
    {
        /* notify tracking change */
        if (!src->elite)
        //    /* add */
            if (src->prof)
                if (!g_player)
                    g_player = std::make_unique<player>(src->id);
        //std::cout << "agent added: " << src->name << " " << src->id << "\n";
        return 0;
    }
    else if (ev->is_statechange) {
        if (ev->is_statechange == cbtstatechange::CBTS_ENTERCOMBAT)
        {
            if (g_player )
            {
                g_player->begin_combat(ev->time);
            }
        }
        if (ev->is_statechange == cbtstatechange::CBTS_EXITCOMBAT)
        {
            if (g_player)
            {
                g_player->end_combat(ev->time);
            }
        }
        //	p += _snprintf(p, 400, "is_statechange: %u\n", ev->is_statechange);
    }
    return 0;
}

uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) {
    /* big buffer */
    //char buff[4096];
    //char* p = &buff[0];

    if (!(!g_player || g_player->get_id() == src->id))
        return 0;

    /* ev is null. dst will only be valid on tracking add. skillname will also be null */
    if (!ev) {

        /* notify tracking change */
        //if (!src->elite) {

        //    /* add */
        //    if (src->prof) {
        //        if (g_player == nullptr)
        //            g_player = std::make_unique<player>(src->id);

        //        //std::cout << "==== cbtnotify ====\n";
        //        //p += _snprintf(p, 400, "==== cbtnotify ====\n");
        //        //p += _snprintf(p, 400, "agent added: %s:%s (%0llx), instid: %u, prof: %u, elite: %u, self: %u, team: %u, subgroup: %u\n", src->name, dst->name, src->id, dst->id, dst->prof, dst->elite, dst->self, src->team, dst->team);
        //        std::cout << "agent added: " << src->name << " "<< dst->name << "\n";
        //    }

        //    /* remove */
        //    else {
        //        //p += _snprintf(p, 400, "==== cbtnotify ====\n");
        //        //p += _snprintf(p, 400, "agent removed: %s (%0llx)\n", src->name, src->id);
        //        //std::cout << "agent removed: " << src->name << " " << src->id;
        //    }
        //}

        ///* notify target change */
        //else if (src->elite == 1) {
        //    //p += _snprintf(p, 400, "==== cbtnotify ====\n");
        //    //p += _snprintf(p, 400, "new target: %0llx\n", src->id);
        //}
    }

    /* combat event. skillname may be null. non-null skillname will remain static until module is unloaded. refer to evtc notes for complete detail */
    else if (g_player){

        ///* default names */
        if (!src->name || !strlen(src->name)) src->name = const_cast<char*>("(area)");
        if (!dst->name || !strlen(dst->name)) dst->name = const_cast<char*>("(area)");

        ///* common */
        //p += _snprintf(p, 400, "==== cbtevent %u at %llu ====\n", cbtcount, ev->time);
        //p += _snprintf(p, 400, "source agent: %s (%0llx:%u, %lx:%lx), master: %u\n", src->name, ev->src_agent, ev->src_instid, src->prof, src->elite, ev->src_master_instid);
        //if (ev->dst_agent) p += _snprintf(p, 400, "target agent: %s (%0llx:%u, %lx:%lx)\n", dst->name, ev->dst_agent, ev->dst_instid, dst->prof, dst->elite);
        //else p += _snprintf(p, 400, "target agent: n/a\n");



        ///* statechange */
        if (ev->is_statechange) {
            std::cout << "statchange: " << ev->is_statechange << "\n";
            if (ev->is_statechange == cbtstatechange::CBTS_ENTERCOMBAT)
            {
                // doesnt work
                //healing_done = 0;
                //dmg_done = 0;
            }
            if (ev->is_statechange == cbtstatechange::CBTS_EXITCOMBAT)
            {
                
                //std::cout << "sum healing: " << healing_done << "\n";
                //std::cout << "sum dmg: " << dmg_done << "\n";
            }
        //	p += _snprintf(p, 400, "is_statechange: %u\n", ev->is_statechange);
        }

        ///* activation */
        else if (ev->is_activation) {
        //	p += _snprintf(p, 400, "is_activation: %u\n", ev->is_activation);
        //	p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
        //	p += _snprintf(p, 400, "ms_expected: %d\n", ev->value);
        }

        ///* buff remove */
        else if (ev->is_buffremove) {
        //	p += _snprintf(p, 400, "is_buffremove: %u\n", ev->is_buffremove);
        //	p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
        //	p += _snprintf(p, 400, "ms_duration: %d\n", ev->value);
        //	p += _snprintf(p, 400, "ms_intensity: %d\n", ev->buff_dmg);
        }

        ///* buff */
        else if (ev->buff) {

            /* damage */
            if (ev->buff_dmg) {

        //		p += _snprintf(p, 400, "is_buff: %u\n", ev->buff);
        //		p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
        //		p += _snprintf(p, 400, "dmg: %d\n", ev->buff_dmg);
                //std::cout << "ev->buff" << ev->buff << "\n";
                //std::cout << "buff_dmg: " << ev->buff_dmg << "\n";
                //if (ev->buff_dmg > 0)
                //    dmg_done += ev->buff_dmg;
                //else if (ev->buff_dmg < 0)
                //    healing_done -= ev->buff_dmg;
        //		p += _snprintf(p, 400, "is_shields: %u\n", ev->is_shields);
                if (ev->value > 0)
                    g_player->add_damage(ev->buff_dmg);
                else if (ev->value < 0)
                    g_player->add_healing(-ev->buff_dmg);

                    //g_player->combat_visitor(ev, src, dst, skillname, id, revision);

            }

        //	/* application */
            else {
        //		p += _snprintf(p, 400, "is_buff: %u\n", ev->buff);
        //		p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
        //		p += _snprintf(p, 400, "raw ms: %d\n", ev->value);
        //		p += _snprintf(p, 400, "overstack ms: %u\n", ev->overstack_value);
                //std::cout << "healing?: " << ev->buff_dmg << "|" << ev->value << std::endl;
            }
        }

        ///* physical */
        else {
        //	p += _snprintf(p, 400, "is_buff: %u\n", ev->buff);
        //	p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
        //	p += _snprintf(p, 400, "dmg: %d\n", ev->value);
            //std::cout << "physical dmg: " << ev->value << "\n";
            //if (ev->value > 0)
            //    dmg_done += ev->value;
            //else if (ev->value < 0)
            //    healing_done -= ev->value;
            //std::cout << "shield: " << ev->is_shields << "\n";
        //	p += _snprintf(p, 400, "is_moving: %u\n", ev->is_moving);
        //	p += _snprintf(p, 400, "is_ninety: %u\n", ev->is_ninety);
        //	p += _snprintf(p, 400, "is_flanking: %u\n", ev->is_flanking);
        //	p += _snprintf(p, 400, "is_shields: %u\n", ev->is_shields);
            if (ev->value > 0)
                g_player->add_damage(ev->value);
            else if (ev->value < 0)
                g_player->add_healing(-ev->value);
            //g_player->combat_visitor(ev, src, dst, skillname, id, revision);
        }

        ///* common */
        //p += _snprintf(p, 400, "iff: %u\n", ev->iff);
        //p += _snprintf(p, 400, "result: %u\n", ev->result);
        cbtcount += 1;
    }

    /* print */
    //DWORD written = 0;
    //p[0] = 0;
    //wchar_t buffw[4096];
    //int32_t rc = MultiByteToWideChar(CP_UTF8, 0, &buff[0], 4096, &buffw[0], 4096);
    //buffw[rc] = 0;
    //WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &buffw[0], (DWORD)wcslen(&buffw[0]), &written, 0); causes a crash?
    
    std::cout << std::flush;
    return 0;
}
