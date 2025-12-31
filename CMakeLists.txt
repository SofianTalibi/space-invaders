cmake_minimum_required(VERSION 3.10)
project(space_invaders LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources communes
set(GAME_SOURCES
    src/Game.cpp
)

# ------------------
# Version console
# ------------------
add_executable(space_invaders_console
    src/main.cpp
    ${GAME_SOURCES}
)
target_include_directories(space_invaders_console PRIVATE include)

# ------------------
# Version SFML
# ------------------
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

add_executable(space_invaders_sfml
    src/main_sfml.cpp
    ${GAME_SOURCES}
)
target_include_directories(space_invaders_sfml PRIVATE include)
target_compile_definitions(space_invaders_sfml PRIVATE USE_SFML)
target_link_libraries(space_invaders_sfml
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# --- Copy assets next to executable (so relative paths work) ---
add_custom_command(TARGET space_invaders_sfml POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:space_invaders_sfml>/assets
)

# ------------------
# Tests (Catch2)
# ------------------
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(space_invaders_tests
    tests/test_game.cpp
    src/Game.cpp
)
target_include_directories(space_invaders_tests PRIVATE include)
target_link_libraries(space_invaders_tests PRIVATE Catch2::Catch2WithMain)

add_test(NAME space_invaders_tests COMMAND space_invaders_tests)
