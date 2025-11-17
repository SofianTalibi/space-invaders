#include <iostream>
#include "Game.hpp"

Game::Game() {
    width = 20;
    playerX = width / 2;
}

void Game::run() {
    bool running = true;
    while (running) {
        render();
        processInput();
        update();
    }
}

void Game::processInput() {
    char input;
    std::cin >> input;
    if (input == 'q') exit(0);
    if (input == 'a' && playerX > 0) playerX--;
    if (input == 'd' && playerX < width-1) playerX++;
}

void Game::update() {
    // futur code
}

void Game::render() {
    system("clear");
    for (int i = 0; i < width; i++) {
        if (i == playerX) std::cout << 'A';
        else std::cout << '.';
    }
    std::cout << std::endl;
}
