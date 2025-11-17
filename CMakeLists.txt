# Nom minimum de CMake
cmake_minimum_required(VERSION 3.10)

# Nom du projet et version
project(SpaceInvaders VERSION 1.0)

# Utiliser C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Inclure les headers
include_directories(include)

# Ajouter les fichiers sources
file(GLOB SOURCES "src/*.cpp")

# Créer l’exécutable
add_executable(space_invaders ${SOURCES})
