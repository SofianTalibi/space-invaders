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

    if (input == 'q') running = false; // quitter le jeu
    if (input == 'a' && playerX > 0) playerX--; // gauche
    if (input == 'd' && playerX < width-1) playerX++; // droite
    if (input == ' ') shoot(); // barre d’espace pour tirer
}


void Game::update() {
    for (int i = 0; i < bulletsY.size(); i++) {
        bulletsY[i]++; // chaque tir monte d’une ligne
    }

    // supprimer les tirs qui sortent de l’écran
    for (int i = bulletsY.size() - 1; i >= 0; i--) {
        if (bulletsY[i] >= width) { // si tir trop haut
            bulletsY.erase(bulletsY.begin() + i);
            bulletsX.erase(bulletsX.begin() + i);
        }
    }
}


void Game::render() {
    system("clear"); // efface l’écran

    // afficher une seule ligne pour l’instant 
    for (int i = 0; i < width; i++) {
        bool bulletHere = false;

        for (int j = 0; j < bulletsX.size(); j++) {
            if (bulletsX[j] == i && bulletsY[j] == 0) bulletHere = true;
        }

        if (bulletHere) std::cout << '|'; // tir
        else if (i == playerX) std::cout << 'A'; // joueur
        else std::cout << '.';
    }
    std::cout << std::endl;
}


void Game::shoot() {
    bulletsX.push_back(playerX); // le tir part de la position du joueur
    bulletsY.push_back(0);       // en haut de l’écran (ligne 0)
}
