cmake_minimum_required(VERSION 3.5)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

message("cmake for ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -O0 -g")

include_directories(game/include)
include_directories(gameViewer/include)

# List of source files
set(SOURCE_FILES_GAME
    game/src/Position.cpp
    game/src/BoardHelper.cpp
    game/src/Evaluator.cpp
    game/src/Solver.cpp
	game/game.cpp
)

set(SOURCE_FILES_GAMEVIEWER
)

set(localLibs
)
set(externLibs
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
endif()

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
