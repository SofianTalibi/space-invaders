#include <iostream>
#include <vector>
#include <unistd.h>     // pour usleep
#include <termios.h>    // pour lire les touches sans Entrée
#include <stdio.h>
#include "Game.hpp"

Game::Game() {
    width = 40;    // largeur de l'écran
    height = 10;   // hauteur de l'écran
    playerX = width / 2;
    // créer une ligne d'ennemis en haut
    enemyDirection = 1; // les ennemis commencent en allant vers la droite
    // créer la première vague d’ennemis
    spawnWave();

    running = true;
}

// fonction pour lire une touche sans Entrée
char getInput() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcgetattr()");
    old.c_lflag &= ~ICANON; // désactive le buffering
    old.c_lflag &= ~ECHO;   // ne pas afficher la touche
    old.c_cc[VMIN] = 0;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) {
        // pas de touche appuyée
    }
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
    return buf;
}


void Game::run() {
    while (running) {
        render();

        char input = getInput();

        if (input == 'a' && playerX > 0) playerX--;
        if (input == 'd' && playerX < width - 1) playerX++;
        if (input == 't') shoot(); // tirer
        if (input == 'q') running = false;

        update();       // bouger les tirs
        usleep(100000); // vitesse du mouvement 

    }
}



void Game::processInput() {
    char input;
    std::cin >> input;

    if (input == 'q') running = false; // quitter le jeu
    if (input == 'a' && playerX > 0) playerX--; // gauche
    if (input == 'd' && playerX < width-1) playerX++; // droite
    if (input == 't') {
    bulletsX.push_back(playerX);  // ajouter la position X du tir
    bulletsY.push_back(1);        // position Y initiale du tir (juste au-dessus du joueur)
    }; // t pour tirer
}

void Game::update() {

    // -------------------------------------------
    // Déplacement des ennemis (contrôlé par enemySpeed)
    // -------------------------------------------
    enemyMoveCounter++;
    if (enemyMoveCounter >= enemySpeed) {
        enemyMoveCounter = 0;

        bool mustGoDown = false;

        for (const auto& e : enemies) {
            if (e.x + enemyDirection < 0 || e.x + enemyDirection >= width) {
                mustGoDown = true;
                break;
            }
        }

        if (mustGoDown) {
            enemyDirection *= -1;
            for (auto& e : enemies) e.y -= 1;
        } else {
            for (auto& e : enemies) e.x += enemyDirection;
        }
    }


    // tirs ennemis aléatoires
    if (rand() % (20 - std::min(level, 15)) == 0) {   // 1 chance sur 20 à chaque frame
        enemyShoot();
    }



    // --- Mouvements des tirs ---
    for (int i = 0; i < bulletsY.size(); i++) {
        bulletsY[i]++;
    }

    // --- déplacement des tirs ennemis ---
    for (int i = 0; i < enemyBulletsY.size(); i++) {
        enemyBulletsY[i]--;
    }
    
    // --- Suppression des tirs ennemis ---
    for (int i = enemyBulletsY.size() - 1; i >= 0; i--) {
    if (enemyBulletsY[i] < 0) {
        enemyBulletsX.erase(enemyBulletsX.begin() + i);
        enemyBulletsY.erase(enemyBulletsY.begin() + i);
    }
    }

    // mise à jour des explosions
    for (int i = 0; i < explosions.size(); i++) {
        explosions[i].timer--;
        if (explosions[i].timer <= 0) {
            explosions.erase(explosions.begin() + i);
            i--;
        }
    }



    // --- Détection de collision des tirs enemis avec le joueur ---
    for (int i = enemyBulletsY.size() - 1; i >= 0; i--) {
    if (enemyBulletsX[i] == playerX && enemyBulletsY[i] == 0) {
        running = false;
        std::cout << "GAME OVER ! Vous avez été touché." << std::endl;
        usleep(500000);
        return;
    }
    }

    // --- Détection de collision des enemis avec le joueur ---
    for (const auto& e : enemies) {
    if (e.y == 0) {
        running = false;
        std::cout << "GAME OVER ! Les ennemis ont atteint votre position !" << std::endl;
        usleep(500000);
        return;
    }
    }


    // supprimer les tirs hors écran
    for (int i = bulletsY.size() - 1; i >= 0; i--) {
        if (bulletsY[i] >= height) {
            bulletsY.erase(bulletsY.begin() + i);
            bulletsX.erase(bulletsX.begin() + i);
        }
    }

    // vérifier collisions
    for (int i = bulletsY.size() - 1; i >= 0; i--) {
        for (int j = enemies.size() - 1; j >= 0; j--) {
            if (bulletsX[i] == enemies[j].x && bulletsY[i] == enemies[j].y) {

                // >>> AJOUT EXPLOSION ASCII
                Explosion exp;
                exp.x = enemies[j].x;
                exp.y = enemies[j].y;
                exp.timer = 5;   // explosion visible pendant 5 frames
                explosions.push_back(exp);
                

                // >>> AJOUT SCORE
                score += 10;
                

                enemies.erase(enemies.begin() + j);
                bulletsX.erase(bulletsX.begin() + i);
                bulletsY.erase(bulletsY.begin() + i);
                break;
            }

        }
    }

    // -------------------------------------------
    // Tous les ennemis sont morts → vague suivante
    // -------------------------------------------
    if (enemies.empty()) {
        level++;
        spawnWave();
    }

}

void Game::enemyShoot() {
    if (enemies.empty()) return;

    // un ennemi choisi au hasard
    int idx = rand() % enemies.size();
    enemyBulletsX.push_back(enemies[idx].x);
    enemyBulletsY.push_back(enemies[idx].y - 1);
}




void Game::render() {
    system("clear"); // efface l'écran

    // >>> AFFICHAGE DES BORDURES ASCII
    for (int x = 0; x < width + 2; x++) std::cout << "#"; // bordure du haut
    std::cout << std::endl;


    //  affichage du score
    std::cout << "SCORE : " << score << "\n\n";
    

    for (int y = height - 1; y >= 0; y--) {      // parcourir chaque ligne
        std::cout << "#"; // bordure gauche
        for (int x = 0; x < width; x++) {        // parcourir chaque colonne
            bool printed = false;

            // afficher les tirs
            for (int i = 0; i < bulletsX.size(); i++) {
                if (bulletsX[i] == x && bulletsY[i] == y) {
                    std::cout << '|';
                    printed = true;
                    break;
                }
            }
            // afficher tirs ennemis
            for (int i = 0; i < enemyBulletsX.size(); i++) {
                if (enemyBulletsX[i] == x && enemyBulletsY[i] == y) {
                    std::cout << '!';
                    printed = true;
                    break;
                }
            }

            // afficher les ennemis
            if (!printed) {
                for (const auto& e : enemies) {
                    if (e.x == x && e.y == y) {
                        std::cout << 'M'; // caractère pour un ennemi
                        printed = true;
                        break;
                    }
                }
            }

            //  afficher explosion ASCII
            if (!printed) {
                for (const auto& ex : explosions) {
                    if (ex.x == x && ex.y == y) {
                        std::cout << '*'; // explosion
                        printed = true;
                        break;
                    }
                }
            }
            

            // afficher le joueur
            if (!printed) {
                if (y == 0 && x == playerX)
                    std::cout << 'A';
                else
                    std::cout << '.';
            }
        }
        std::cout << "#" << std::endl; // bordure droite
        }

        // bordure du bas
        for (int x = 0; x < width + 2; x++) std::cout << "#";
        std::cout << std::endl; // passer à la ligne suivante
    
}




void Game::shoot() {
    
    bulletsX.push_back(playerX); // le tir part de la position du joueur
    bulletsY.push_back(0);       // en haut de l’écran (ligne 0)
}

// -------------------------------------------
// Génère une nouvelle vague d’ennemis
// -------------------------------------------
void Game::spawnWave() {
    enemies.clear();

    for (int i = 0; i < 5 + level; i++) {   // plus le niveau monte, plus il y a d’ennemis
        Enemy e;
        e.x = 2 + (i % 8) * 2;              // placement automatique
        e.y = height - 1 - (i / 8);        // plusieurs lignes si beaucoup d’ennemis
        enemies.push_back(e);
    }

    enemyDirection = 1;

    // ennemis plus agressifs à chaque niveau
    enemySpeed = std::max(3, 10 - level);  // minimum = 3 (très rapide)

    std::cout << "\n--- NIVEAU " << level << " ---\n";
    usleep(500000);
}

