cmake_minimum_required(VERSION 4.2)
project(agent_model CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# compiling flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -flto)
endif()

# ==========================================
# 1. Downloading external dependelsncies
# ==========================================

include(FetchContent)

# GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Dear ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.4
)

# ImPlot
FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG v0.16
)

FetchContent_MakeAvailable(glfw imgui implot)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# ==========================================
# 2. ui_core
# ==========================================

add_library(ui_core STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
        ${implot_SOURCE_DIR}/implot_demo.cpp
)

target_include_directories(ui_core PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${implot_SOURCE_DIR}
)
target_link_libraries(ui_core PUBLIC glfw OpenGL::GL)

# ==========================================
# 3. actual project
# ==========================================

add_library(ring_buffer INTERFACE
        include/AgentModel/RingBuffer.hpp
        include/AgentModel/SimulationBridge.hpp
)

add_library(model_core STATIC
        src/Engine/Model.cpp

        include/AgentModel/ModelStructures.hpp
        include/AgentModel/Model.hpp
)
target_include_directories(model_core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(ui STATIC
        src/UI/UiRenderer.cpp

        include/AgentModel/UIRenderer.hpp
)

target_include_directories(ui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ui PRIVATE ui_core ring_buffer)

# font
set(FONT_DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/assets/JetBrainsMono-Regular.ttf")
if(NOT EXISTS ${FONT_DESTINATION})
    message(STATUS "Downloading font...")
    file(DOWNLOAD
            "https://raw.githubusercontent.com/JetBrains/JetBrainsMono/master/fonts/ttf/JetBrainsMono-Regular.ttf"
            ${FONT_DESTINATION}
            SHOW_PROGRESS
    )
endif()

# ==========================================
# 4. executable
# ==========================================
add_executable(agent_model apps/main.cpp)

target_link_libraries(agent_model PRIVATE
        ring_buffer
        model_core
        ui
        ui_core
        glfw
        OpenGL::GL
        Threads::Threads
)

add_custom_command(TARGET agent_model POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:agent_model>/assets"
        COMMENT "Copying assets..."
)
