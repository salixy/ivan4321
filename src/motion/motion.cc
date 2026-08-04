#include "motion/motion.hxx"

#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>

c_motion g_motion;

float c_motion::interpolate( float const current, float const target, float const speed, float const delta_time ) const
{
    float const dt = std::clamp( delta_time, 0.0005f, 0.033f );
    return current + ( target - current ) * ( 1.0f - std::exp( -speed * dt ) );
}

ImVec2 c_motion::interpolate( ImVec2 const& current, ImVec2 const& target, float const speed, float const delta_time ) const
{
    return ImVec2(
        interpolate( current.x, target.x, speed, delta_time ),
        interpolate( current.y, target.y, speed, delta_time )
    );
}

ImU32 c_motion::calculate_gradient_color( float const u, float const time_offset ) const
{
    float phase = u - time_offset;
    phase = phase - std::floor( phase ); // Wrap to [0, 1)

    ImVec4 const c0 = ImVec4( 83.0f / 255.0f, 172.0f / 255.0f, 255.0f / 255.0f, 1.0f );
    ImVec4 const c1 = ImVec4( 204.0f / 255.0f, 118.0f / 255.0f, 193.0f / 255.0f, 1.0f );
    ImVec4 const c2 = ImVec4( 167.0f / 255.0f, 235.0f / 255.0f, 57.0f / 255.0f, 1.0f );

    ImVec4 result;
    if ( phase < 0.5f )
    {
        float const t = phase / 0.5f;
        result = ImVec4(
            c0.x + ( c1.x - c0.x ) * t,
            c0.y + ( c1.y - c0.y ) * t,
            c0.z + ( c1.z - c0.z ) * t,
            1.0f
        );
    }
    else
    {
        float const t = ( phase - 0.5f ) / 0.5f;
        result = ImVec4(
            c1.x + ( c2.x - c1.x ) * t,
            c1.y + ( c2.y - c1.y ) * t,
            c1.z + ( c2.z - c1.z ) * t,
            1.0f
        );
    }

    return IM_COL32( ( int )( result.x * 255.0f ), ( int )( result.y * 255.0f ), ( int )( result.z * 255.0f ), 255 );
}

void animation_t::update( float const delta_time )
{
    m_value = g_motion.interpolate( m_value, m_target, m_speed, delta_time );
}

void animation_t::set( float const target )
{
    m_target = target;
}

void animation_t::reset( float const val )
{
    m_value = val;
    m_target = val;
}

void animation_vec_t::update( float const delta_time, ImVec2 const& origin )
{
    if ( origin.x != 0.0f || origin.y != 0.0f )
    {
        if ( m_origin.x != 0.0f || m_origin.y != 0.0f )
        {
            ImVec2 const delta = ImVec2( origin.x - m_origin.x, origin.y - m_origin.y );
            m_value.x += delta.x;
            m_value.y += delta.y;
            m_target.x += delta.x;
            m_target.y += delta.y;
        }
        m_origin = origin;
    }

    m_value = g_motion.interpolate( m_value, m_target, m_speed, delta_time );
}

void animation_vec_t::set( ImVec2 const& target )
{
    m_target = target;
}

void animation_vec_t::reset( ImVec2 const& val )
{
    m_value = val;
    m_target = val;
    m_origin = ImVec2( 0.0f, 0.0f );
}



