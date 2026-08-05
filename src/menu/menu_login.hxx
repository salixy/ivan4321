#pragma once

#include "imgui.h"
#include "texture/texture.hxx"
#include "input/input.hxx"
#include "motion/motion.hxx"
#include <string>

struct c_menu;

namespace menu_login
{
    void render_left_panel(
        ImVec2 const& pos,
        ImVec2 const& size,
        float const ease_t,
        c_texture const& bg_texture,
        c_texture const& cici_texture,
        animation_t& anim_tg_hover,
        animation_t& anim_discord_hover,
        float const delta_time
    );

    void render_inputs(
        c_menu* menu,
        float const ease_t,
        ImFont* font_medium_32,
        float const delta_time
    );
}
