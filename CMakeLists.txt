# CMake minimum
cmake_minimum_required(VERSION 3.10)

# Projet
project(SpaceInvaders VERSION 1.0 LANGUAGES CXX)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Dossiers include du projet
include_directories(include)

# Tous les fichiers sources
file(GLOB SOURCES "src/*.cpp")

# ---------------------------
# Version console (toujours buildée)
# ---------------------------
# On retire main_sfml.cpp pour la version console
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main_sfml.cpp")
add_executable(space_invaders_console ${SOURCES})

# ---------------------------
# Version SFML (si disponible)
# ---------------------------
set(SFML_ROOT "${CMAKE_SOURCE_DIR}/external/SFML-2.5.1")
set(SFML_INCLUDE_DIR "${SFML_ROOT}/include")
set(SFML_LIBRARY_DIR "${SFML_ROOT}/lib")

if(EXISTS "${SFML_INCLUDE_DIR}" AND EXISTS "${SFML_LIBRARY_DIR}")
    message(STATUS "Utilisation de SFML locale : ${SFML_ROOT}")

    # Inclure et linker SFML
    include_directories(${SFML_INCLUDE_DIR})
    link_directories(${SFML_LIBRARY_DIR})
    
    # On retire main.cpp pour la version SFML
    file(GLOB SFML_SOURCES "src/*.cpp")
    list(REMOVE_ITEM SFML_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

    add_executable(space_invaders_sfml ${SFML_SOURCES})

    # Bibliothèques SFML à lier
    target_link_libraries(space_invaders_sfml sfml-graphics sfml-window sfml-system)
else()
    message(STATUS "Pas de SFML locale trouvée sous ${SFML_ROOT} (build console uniquement)")
endif()

