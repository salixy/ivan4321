cmake_minimum_required(VERSION 3.20)
project(ImGuiApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

option(USE_SDL "Use SDL2 instead of GLFW" ON)

if(USE_SDL)
    find_package(SDL2 REQUIRED)
    add_compile_definitions(USE_SDL)
else()
    # Fetch GLFW for OpenGL windowing
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# Fetch FreeType
FetchContent_Declare(
    freetype
    URL https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-2.zip
)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(freetype)

# ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp
)

if(USE_SDL)
    list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp)
else()
    list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp)
endif()

set(APP_SOURCES
    src/main.cc
    src/render/render.cc
    src/menu/menu.cc
    src/motion/motion.cc
    src/button/button.cc
    src/input/input.cc
    src/fonts/fonts.cc
    src/texture/texture.cc
    src/blur/blur.cc
    src/tabs/tabs.cc
    src/settings/settings.cc
    src/widgets/widgets.cc
)

add_executable(ImGuiApp ${APP_SOURCES} ${IMGUI_SOURCES})

target_include_directories(ImGuiApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link GLFW/SDL2 and OpenGL
find_package(OpenGL REQUIRED)

if(USE_SDL)
    target_include_directories(ImGuiApp PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(ImGuiApp PRIVATE ${SDL2_LIBRARIES} OpenGL::GL freetype)
else()
    target_link_libraries(ImGuiApp PRIVATE glfw OpenGL::GL freetype)
endif()

target_compile_definitions(ImGuiApp PRIVATE IMGUI_ENABLE_FREETYPE)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

