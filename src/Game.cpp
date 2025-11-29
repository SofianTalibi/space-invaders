#include <iostream>
#include <vector>
#include <unistd.h>     // usleep : temporisation
#include <termios.h>    // lecture clavier non bloquante
#include <stdio.h>
#include "Game.hpp"
#include <algorithm>
#include <cstdlib>

// ======================================================
//                   CONSTRUCTEUR DU JEU
// ======================================================
Game::Game() {
    width = 40;             // largeur du terrain
    height = 10;            // hauteur du terrain
    playerX = width / 2;    // joueur centré
    lives = 3;              // nombre de vies
    score = 0;              // score initial
    level = 1;              // niveau initial

    enemyDirection = 1;     // direction initiale des ennemis
    enemyMoveCounter = 0;   // compteur pour la vitesse des ennemis

    spawnWave();            // création de la première vague
    running = true;         // la boucle du jeu est active
}

// ======================================================
//       Fonction utilitaire : lire une touche sans ENTER
// ======================================================
char getInput() {
    char buf = 0;
    struct termios old = {0};
    tcgetattr(0, &old);         // récupère les paramètres du terminal
    old.c_lflag &= ~ICANON;     // mode non canonique (lecture instantanée)
    old.c_lflag &= ~ECHO;       // ne pas afficher les touches
    old.c_cc[VMIN] = 0;         // lecture non bloquante
    old.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &old);

    read(0, &buf, 1);           // lire une touche

    old.c_lflag |= ICANON;      // restauration mode canonique
    old.c_lflag |= ECHO;        // réactivation affichage touches
    tcsetattr(0, TCSADRAIN, &old);

    return buf;                 // retourne la touche
}

// ======================================================
//                     BOUCLE PRINCIPALE
// ======================================================
void Game::run() {
    while (running) {
        render();              // afficher l’état du jeu
        char input = getInput(); // lire la touche pressée

        // ------------------------------ Gestion joueur ------------------------------
        if (input == 'a' && playerX > 0) playerX--;  // déplacement gauche
        if (input == 'd' && playerX < width - 1) playerX++; // déplacement droite
        if (input == 't') shoot();                  // tir
        if (input == 'q') running = false;          // quitter le jeu

        update();             // mettre à jour tout le jeu
        usleep(100000);       // rythme (~10 FPS)
    }
}

// ======================================================
//               MISE À JOUR DE L’ÉTAT DU JEU
// ======================================================
void Game::update() {
    // ---------------- Déplacement horizontal des ennemis ----------------
    enemyMoveCounter++;                   // incrémente le compteur
    if (enemyMoveCounter >= enemySpeed) { // si le compteur atteint la vitesse
        enemyMoveCounter = 0;             // réinitialisation

        bool mustGoDown = false;          // flag pour savoir si on doit descendre
        for (const auto& e : enemies) {
            if (e.x + enemyDirection < 0 || e.x + enemyDirection >= width) {
                mustGoDown = true;       // un ennemi touche le mur
                break;
            }
        }

        if (mustGoDown) {
            enemyDirection *= -1;        // inverse le sens
            for (auto& e : enemies) e.y -= 1; // descente vers le joueur
        } else {
            for (auto& e : enemies) e.x += enemyDirection; // déplacement normal
        }
    }

    // ---------------- Tirs ennemis aléatoires ----------------
    if (!enemies.empty() && rand() % (20 - std::min(level, 15)) == 0) {
        enemyShoot();
    }

    // ---------------- Déplacement des tirs joueurs ----------------
    for (int i = 0; i < bulletsY.size(); i++) bulletsY[i]++;

    // ---------------- Déplacement des tirs ennemis ----------------
    for (int i = 0; i < enemyBulletsY.size(); i++) enemyBulletsY[i]--;

    // ---------------- Suppression des tirs joueurs hors écran ----------------
    for (int i = bulletsY.size() - 1; i >= 0; i--) {
        if (bulletsY[i] >= height) {
            bulletsX.erase(bulletsX.begin() + i);
            bulletsY.erase(bulletsY.begin() + i);
        }
    }

    // ---------------- Suppression des tirs ennemis hors écran ----------------
    for (int i = enemyBulletsY.size() - 1; i >= 0; i--) {
        if (enemyBulletsY[i] < 0) {
            enemyBulletsX.erase(enemyBulletsX.begin() + i);
            enemyBulletsY.erase(enemyBulletsY.begin() + i);
        }
    }

    // ---------------- Mise à jour des explosions ----------------
    for (int i = 0; i < explosions.size(); i++) {
        explosions[i].timer--;                 // décrémente le timer
        if (explosions[i].timer <= 0) {       // si terminé
            explosions.erase(explosions.begin() + i);
            i--;
        }
    }

    // ---------------- Collision tir ennemi → joueur ----------------
    for (int i = enemyBulletsY.size() - 1; i >= 0; i--) {
        if (enemyBulletsX[i] == playerX && enemyBulletsY[i] == 0) {
            enemyBulletsX.erase(enemyBulletsX.begin() + i); // supprime tir
            enemyBulletsY.erase(enemyBulletsY.begin() + i);

            lives--;                    // enlève une vie
            if (lives <= 0) {           // si plus de vies
                running = false;
                std::cout << "GAME OVER ! Vies = 0\n";
                usleep(500000);
                return;
            } else {                     // sinon on continue
                std::cout << "Touché ! Vies restantes : " << lives << "\n";
                playerX = width / 2;     // repositionne le joueur
                enemyBulletsX.clear();   // vide les tirs ennemis
                enemyBulletsY.clear();
                usleep(300000);          // pause pour voir le message
            }
        }
    }

    // ---------------- Collision tir joueur → ennemis ----------------
    for (int i = bulletsY.size() - 1; i >= 0; i--) {
        for (int j = enemies.size() - 1; j >= 0; j--) {
            if (bulletsX[i] == enemies[j].x && bulletsY[i] == enemies[j].y) {
                explosions.push_back({enemies[j].x, enemies[j].y, 5}); // explosion
                score += 10;             // augmente le score
                enemies.erase(enemies.begin() + j); // supprime l'ennemi
                bulletsX.erase(bulletsX.begin() + i); // supprime le tir
                bulletsY.erase(bulletsY.begin() + i);
                break;
            }
        }
    }

    // ---------------- Collision ennemis → joueur ----------------
    for (const auto& e : enemies) {
        if (e.y == 0) {                   // ennemi atteint le bas
            running = false;
            std::cout << "GAME OVER ! Les ennemis ont atteint votre position !\n";
            usleep(500000);
            return;
        }
    }

    // ---------------- Passage à la vague suivante ----------------
    if (enemies.empty()) {
        level++;                          // augmente le niveau
        spawnWave();                      // nouvelle vague
    }
}

// ======================================================
//        Un ennemi tire un projectile vertical
// ======================================================
void Game::enemyShoot() {
    if (enemies.empty()) return;

    int idx = rand() % enemies.size(); // tir aléatoire
    enemyBulletsX.push_back(enemies[idx].x);
    enemyBulletsY.push_back(enemies[idx].y - 1);
}

// ======================================================
//                  AFFICHAGE ASCII
// ======================================================
void Game::render() {
    system("clear");

    // bordure haut
    for (int x = 0; x < width + 2; x++) std::cout << "+";
    std::cout << "\n";

    // HUD
    std::cout << "SCORE : " << score
              << "    VIES : " << lives
              << "    NIVEAU : " << level
              << "\n\n";

    // grille principale
    for (int y = height - 1; y >= 0; y--) {
        std::cout << "|";

        for (int x = 0; x < width; x++) {
            bool printed = false;

            // tir joueur
            for (int i = 0; i < bulletsX.size(); i++) {
                if (bulletsX[i] == x && bulletsY[i] == y) { std::cout << '|'; printed = true; break; }
            }

            // tir ennemi
            if (!printed) {
                for (int i = 0; i < enemyBulletsX.size(); i++) {
                    if (enemyBulletsX[i] == x && enemyBulletsY[i] == y) { std::cout << '!'; printed = true; break; }
                }
            }

            // ennemis
            if (!printed) {
                for (const auto& e : enemies) {
                    if (e.x == x && e.y == y) {
                        if (e.isBoss) std::cout << 'B';  // boss 
                        else std::cout << 'M';  // ennemi normal
                        printed = true;
                        break; }
                }
            }

            // explosions
            if (!printed) {
                for (const auto& ex : explosions) {
                    if (ex.x == x && ex.y == y) { std::cout << '*'; printed = true; break; }
                }
            }

            // joueur
            if (!printed) {
                if (y == 0 && x == playerX) std::cout << 'A';
                else std::cout << '.';
            }
        }

        std::cout << "|\n";
    }

    // bordure bas
    for (int x = 0; x < width + 2; x++) std::cout << "+";
    std::cout << "\n";
}

// ======================================================
//                  Tir du joueur
// ======================================================
void Game::shoot() {
    bulletsX.push_back(playerX);
    bulletsY.push_back(0);
}

// ======================================================
//           Génération d'une vague ennemie
// ======================================================
void Game::spawnWave() {
    enemies.clear();

    if (level % 5 == 0) {
        // --- NIVEAU BOSS ---
        Enemy boss;
        boss.x = width / 2;  // centré
        boss.y = height - 1; // tout en haut
        boss.isBoss = true;
        boss.hp = 10;         // exemple : boss avec 10 PV
        enemies.push_back(boss);
        std::cout << "\n--- BOSS LEVEL " << level << " ---\n";
    }else{

    int enemiesPerLine = 8;             // nombre d'ennemis par ligne
    int lines = level;                  // le niveau = nombre de lignes
    int startY = height - 1;            // première ligne en haut

    for (int line = 0; line < lines; line++) {
        for (int i = 0; i < enemiesPerLine; i++) {
            Enemy e;
            e.x = 2 + i * 2;                 // colonnes espacées
            e.y = startY - line;             // ligne correspondante
            enemies.push_back(e);
        }
    }

    enemyDirection = 1;                     // direction initiale
    enemySpeed = std::max(3, 10 - level);   // vitesse ennemis

    std::cout << "\n--- NIVEAU " << level << " ---\n";
    usleep(500000);}
}
