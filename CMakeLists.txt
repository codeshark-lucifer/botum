cmake_minimum_required(VERSION 4.2) 
project(Atlas LANGUAGES C CXX) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

set(FREETYPE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype)

if(NOT EXISTS ${FREETYPE_DIR}/CMakeLists.txt)
    message(STATUS "FreeType not found. Cloning from GitHub...")

    execute_process(
        COMMAND git clone https://github.com/freetype/freetype.git ${FREETYPE_DIR}
        RESULT_VARIABLE GIT_RESULT
    )

    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone FreeType repository")
    endif()
endif()

add_subdirectory(${FREETYPE_DIR})
add_subdirectory(vendor/glad) 

add_executable(app 
    src/main.cpp
    src/core/helper.cpp
    src/core/win32.cpp
    src/core/input.cpp
    src/core/renderer.cpp
    src/core/shader.cpp
)

target_include_directories(app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE 
    user32  # window creation, message loop, input handling
    gdi32   # basic graphics output(HDC, BitBlt, etc.)
    winmm   # multimedia timer for frame timing, sound output (PlaySound, etc.)
    glad    # OpenGL function loader
    dwmapi  # DWM for window composition, transparency, etc.
    freetype # FreeType for font rendering
)

# if(WIN32)
#     # Build as a GUI application (no console window) on Windows
#     set_target_properties(app PROPERTIES WIN32_EXECUTABLE TRUE)
# endif()
