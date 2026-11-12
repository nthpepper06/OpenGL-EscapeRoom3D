cmake_minimum_required(VERSION 3.20)
project(EscapeRoom3D LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Handle SSL certificate verification for MinGW curl
set(CMAKE_TLS_VERIFY 0)
cmake_policy(SET CMP0135 NEW)
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

include(FetchContent)

# 1. GLFW
message(STATUS "Configuring GLFW...")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

# 2. GLAD (OpenGL 3.3 Core loader)
message(STATUS "Configuring GLAD...")
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/libigl/libigl-glad.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# 3. GLM (OpenGL Mathematics)
message(STATUS "Configuring GLM...")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

# 4. STB (stb_image for texture loading)
message(STATUS "Configuring STB...")
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# 5. Dear ImGui with GLFW + OpenGL3 backends
message(STATUS "Configuring Dear ImGui...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
    GIT_SHALLOW TRUE
)

# 6. nlohmann/json for Scene serialization (room_scene.json)
message(STATUS "Configuring nlohmann_json...")
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

# Populate dependencies with CMakeLists
FetchContent_MakeAvailable(glfw glad glm nlohmann_json)

# Populate header-only / custom libraries without CMakeLists
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# Setup STB interface library
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# Setup ImGui static library with backends
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw glad)

# Explicit list of 3D Escape Room project sources
set(ESCAPE_ROOM_SOURCES
    src/main.cpp
    src/core/Common.hpp
    src/core/Application.hpp
    src/core/Application.cpp
    src/camera/Camera.hpp
    src/camera/Camera.cpp
    src/renderer/Shader.hpp
    src/renderer/Shader.cpp
    src/renderer/Mesh.hpp
    src/renderer/Mesh.cpp
    src/renderer/PrimitiveGenerator.hpp
    src/renderer/PrimitiveGenerator.cpp
    src/renderer/Material.hpp
    src/renderer/Light.hpp
    src/renderer/Texture.hpp
    src/renderer/Texture.cpp
    src/renderer/IblPipeline.hpp
    src/renderer/IblPipeline.cpp
    src/renderer/ShadowPipeline.hpp
    src/renderer/ShadowPipeline.cpp
    src/renderer/ParticleSystem.hpp
    src/renderer/ParticleSystem.cpp
    src/renderer/SsaoPipeline.hpp
    src/renderer/SsaoPipeline.cpp
    src/renderer/PlanarReflection.hpp
    src/renderer/PlanarReflection.cpp
    src/renderer/MeshBatcher.hpp
    src/renderer/MeshBatcher.cpp
    src/renderer/PostProcessor.hpp
    src/renderer/PostProcessor.cpp
    src/rooms/BaseRoom.hpp
    src/rooms/OccultRitualRoom.hpp
    src/rooms/OccultRitualRoom.cpp
    src/rooms/AbandonedAsylumRoom.hpp
    src/rooms/AbandonedAsylumRoom.cpp
    src/rooms/MirrorRoom.hpp
    src/rooms/MirrorRoom.cpp
    src/scene/SceneObject.hpp
    src/scene/SceneObject.cpp
    src/scene/EscapeRoomBuilder.hpp
    src/scene/EscapeRoomBuilder.cpp
    src/scene/SceneSerializer.hpp
    src/scene/SceneSerializer.cpp
    src/interaction/Raycaster.hpp
    src/interaction/Raycaster.cpp
    src/interaction/InteractionManager.hpp
    src/interaction/InteractionManager.cpp
    src/ui/GuiLayer.hpp
    src/ui/GuiLayer.cpp
)

add_executable(EscapeRoom ${ESCAPE_ROOM_SOURCES})

target_include_directories(EscapeRoom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${stb_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(EscapeRoom PRIVATE
    glfw
    glad
    glm::glm
    stb
    imgui
    nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(EscapeRoom PRIVATE opengl32)
endif()

# Copy assets directory to build output directory
add_custom_command(TARGET EscapeRoom POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:EscapeRoom>/assets
    COMMENT "Copying assets to build directory"
)

# ==============================================================================
# BÀI THỰC HÀNH LAB 04 - 3D VIEWING TRANSFORMATIONS & PROJECTIONS
# ==============================================================================
add_executable(Lab4
    src/lab4_main.cpp
    src/glad.c
)
target_include_directories(Lab4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(Lab4 PRIVATE glfw glad)
if(WIN32)
    target_link_libraries(Lab4 PRIVATE opengl32 gdi32 winmm)
endif()

# ==============================================================================
# TỔNG HỢP CÁC BÀI THỰC HÀNH LAB 01, 02, 03, 04
# ==============================================================================
add_executable(ComputerGraphicsLabs
    src/old_labs_main.cpp
    src/glad.c
)
target_include_directories(ComputerGraphicsLabs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ComputerGraphicsLabs PRIVATE glfw glad)
if(WIN32)
    target_link_libraries(ComputerGraphicsLabs PRIVATE opengl32 gdi32 winmm)
endif()

