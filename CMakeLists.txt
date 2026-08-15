cmake_minimum_required(VERSION 3.5)
project(Othello LANGUAGES CXX)

set(PROJECT_NAME Othello)
message("cmake for ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -O0 -g")

include_directories(game/include)
include_directories(gameViewer/include)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# List of source files
set(SOURCE_FILES_GAME
    game/src/Position.cpp
    game/src/BoardHelper.cpp
    game/src/Evaluator.cpp
    game/src/Solver.cpp
    game/game.cpp
)

set(SOURCE_FILES_GAMEVIEWER
    gameViewer/src/GameViewer.cpp
)

set(localLibs
)
set(externLibs
    sfml-graphics
    sfml-window
    sfml-system
)


if(TARGET ${PROJECT_NAME})
    message("${PROJECT_NAME} is already defined")
else()
    FOREACH (localLib ${localLibs})
        add_subdirectory (lib/${localLib} ${localLib})
    ENDFOREACH(localLib)

    add_executable (
        ${PROJECT_NAME}
        ${SOURCE_FILES_GAME}
        ${SOURCE_FILES_GAMEVIEWER}
    )

    FOREACH (localLib ${localLibs})
        include_directories (lib/${localLib}/src)
    ENDFOREACH(localLib)

    set(LIB_LINK
        ${localLibs}
        ${externLibs}
    )

    target_link_libraries(${PROJECT_NAME}
        ${LIB_LINK}
    )

    # Copy fonts next to the binary so the viewer can find them at runtime.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/fonts
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/gameViewer/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying gameViewer assets to build directory"
    )
endif()

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
