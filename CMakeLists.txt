cmake_minimum_required(VERSION 3.21)
project(Othello LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR
        "MinGW (${CMAKE_CXX_COMPILER}) cannot use the MSVC SFML package vcpkg installs "
        "(triplet x64-windows). Run .\\run.cmd — it loads the Visual Studio environment — "
        "or open a Developer Command Prompt for VS and configure with a clean build/ folder.")
endif()

include_directories(game/include)
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

set(SOURCE_FILES
        game/src/Position.cpp
        game/src/BoardHelper.cpp
        game/src/Evaluator.cpp
        game/src/Solver.cpp
        game/src/GameViewer.cpp
        game/game.cpp
)
set(externLibs
        SFML::Graphics
        SFML::Window
        SFML::System
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${externLibs})

# So a packaged release can find its bundled shared libs next to the executable
# without needing LD_LIBRARY_PATH/DYLD_LIBRARY_PATH set.
if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
                BUILD_RPATH "$ORIGIN"
                INSTALL_RPATH "$ORIGIN"
        )
elseif(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
                BUILD_RPATH "@executable_path"
                INSTALL_RPATH "@executable_path"
        )
endif()

# Copy fonts next to the binary so the viewer can find them at runtime.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/fonts
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/game/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying gameViewer assets to build directory"
)

# SFML ships as shared libraries; copy the DLLs next to the binary so it can run.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SFML::Graphics>
        $<TARGET_FILE:SFML::Window>
        $<TARGET_FILE:SFML::System>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying SFML runtime DLLs to build directory"
)

# Packaging for distributable releases: `cmake --install build --prefix dist`
# stages the executable, its resolved runtime dependencies (SFML + transitive
# libs like FreeType/zlib/libpng), and game assets into a self-contained folder.
#
# CMP0207 normalizes GET_RUNTIME_DEPENDENCIES paths to forward slashes before
# matching; without it, Windows paths like "C:\WINDOWS\system32/kernel32.dll"
# (mixed slashes) don't match the system32 exclude pattern below and get bundled.
if(POLICY CMP0207)
        cmake_policy(SET CMP0207 NEW)
endif()

install(TARGETS ${PROJECT_NAME}
        RUNTIME_DEPENDENCY_SET ${PROJECT_NAME}_deps
        RUNTIME DESTINATION .
)
install(RUNTIME_DEPENDENCY_SET ${PROJECT_NAME}_deps
        DESTINATION .
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
        POST_EXCLUDE_REGEXES
        ".*/system32/.*\\.dll"
        "^/lib/.*\\.so.*"
        "^/lib64/.*\\.so.*"
        "^/usr/lib/.*\\.so.*"
        "^/usr/lib64/.*\\.so.*"
        DIRECTORIES $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/game/assets DESTINATION .)
