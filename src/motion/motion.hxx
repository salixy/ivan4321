#pragma once

#include "imgui.h"

struct animation_t
{
    float m_value = 0.0f;
    float m_target = 0.0f;
    float m_speed = 10.0f;

    void update( float const delta_time );
    void set( float const target );
    void reset( float const val );
};

struct animation_vec_t
{
    ImVec2 m_value = ImVec2( 0.0f, 0.0f );
    ImVec2 m_target = ImVec2( 0.0f, 0.0f );
    float  m_speed = 10.0f;
    ImVec2 m_origin = ImVec2( 0.0f, 0.0f );

    void update( float const delta_time, ImVec2 const& origin = ImVec2( 0.0f, 0.0f ) );
    void set( ImVec2 const& target );
    void reset( ImVec2 const& val );
};

struct c_motion
{
    float interpolate( float const current, float const target, float const speed, float const delta_time ) const;
    ImVec2 interpolate( ImVec2 const& current, ImVec2 const& target, float const speed, float const delta_time ) const;
    ImU32 calculate_gradient_color( float const u, float const time_offset ) const;
};

extern c_motion g_motion;
