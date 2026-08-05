#pragma once

#include "imgui.h"

struct c_menu;

namespace menu_content
{
    void render_dashboard_content(
        c_menu* menu,
        float const tabs_alpha,
        float const ease_t,
        ImFont* font_medium_32,
        float const delta_time
    );
}
