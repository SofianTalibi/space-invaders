#include <iostream>
#include "Game.hpp"

Game::Game() {
    width = 20;    // largeur de l'écran
    height = 10;   // hauteur de l'écran
    playerX = width / 2;
    running = true;
}


void Game::run() {
    char input;
    while (running) {
        render();
        std::cout << "Touche: ";
        std::cin >> input; // appuyer sur la touche + Entrée

        if (input == 'a' && playerX > 0) playerX--;
        if (input == 'd' && playerX < width-1) playerX++;
        if (input == 's') { // tirer
            bulletsX.push_back(playerX);
            bulletsY.push_back(1);
        }
        if (input == 'q') running = false;

        update(); // faire bouger les tirs
    }
}



void Game::processInput() {
    char input;
    std::cin >> input;

    if (input == 'q') running = false; // quitter le jeu
    if (input == 'a' && playerX > 0) playerX--; // gauche
    if (input == 'd' && playerX < width-1) playerX++; // droite
    if (input == ' ') {
    bulletsX.push_back(playerX);  // ajouter la position X du tir
    bulletsY.push_back(1);        // position Y initiale du tir (juste au-dessus du joueur)
}; // barre d’espace pour tirer
}

void Game::update() {
    // déplacer les tirs vers le haut
    for (int i = 0; i < bulletsY.size(); i++) {
        bulletsY[i]++;
    }

    // supprimer les tirs qui dépassent l'écran
    for (int i = bulletsY.size() - 1; i >= 0; i--) {
        if (bulletsY[i] >= height) {
            bulletsY.erase(bulletsY.begin() + i);
            bulletsX.erase(bulletsX.begin() + i);
        }
    }

}




void Game::render() {
    system("clear"); // efface l'écran

    for (int y = height-1; y >= 0; y--) {
    for (int x = 0; x < width; x++) {
        bool isBullet = false;
        for (int i = 0; i < bulletsX.size(); i++) {
            if (bulletsX[i] == x && bulletsY[i] == y) {
                std::cout << '|';
                isBullet = true;
                break;
            }
        }
        if (!isBullet) {
            if (y == 0 && x == playerX) std::cout << 'A'; // joueur
            else std::cout << '.';
        }
    }
    std::cout << std::endl;
}

}



void Game::shoot() {
    
    bulletsX.push_back(playerX); // le tir part de la position du joueur
    bulletsY.push_back(0);       // en haut de l’écran (ligne 0)
}
