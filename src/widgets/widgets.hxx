#pragma once

#include "imgui.h"
#include "motion/motion.hxx"

#include <string>
#include <vector>
#include <unordered_map>

struct c_widgets
{
    // Persistent animation state cache for smooth hover and popup motion
    std::unordered_map<std::string, animation_t> m_anim_combos;
    std::unordered_map<std::string, bool>        m_combo_open_states;
    std::unordered_map<std::string, bool>        m_picker_open_states;

    bool color_picker_dot( char const* label, float col[ 4 ], float const alpha, float const delta_time, ImFont* font, char const* id = nullptr );
    bool combo( char const* label, int* current_item, std::vector<char const*> const& items, float const alpha, float const delta_time, ImFont* font, char const* id = nullptr );

    bool is_any_open( ) const;
};

extern c_widgets g_widgets;
