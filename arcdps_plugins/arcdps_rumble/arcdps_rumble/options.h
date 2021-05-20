#pragma once

#include <cstdint>

struct options_t
{
    int max_dps = 10;//000;
    int max_hps = 4000;
    size_t selected_device = 0;
};

void show_imgui_option_window();

const options_t& options();
