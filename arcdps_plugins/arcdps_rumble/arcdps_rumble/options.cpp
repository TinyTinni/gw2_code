#include "options.h"

#include <fmt/format.h>

#include "imgui/imgui.h"

#include "rumble_devices.h" //for debug only

options_t g_opt;

void show_imgui_option_window()
{
    static bool show_options = false;
    if (ImGui::Button("Rumble Options"))
        show_options = !show_options;

    if (show_options)
    {
        ImGui::Begin("Rumble Options", &show_options);
        ImGui::Text("Todo");
        ImGui::SliderInt("max dps", &g_opt.max_dps, 0, 50000);
        ImGui::SliderInt("max hps", &g_opt.max_hps, 0, 20000);

        ImGui::Separator();

        ImGui::Text(fmt::format("Found Devices: {}", rumble_devices().number()).c_str());

        if (ImGui::Button("Stop All Rumble"))
            rumble_devices().disable_all();

        ImGui::End();
    }

   
}

const options_t & options()
{
    return g_opt;
}
