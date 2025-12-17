#include <iostream>
#include <vector>
#include <stdio.h>
#include "Game.hpp"
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <ctime>

#ifndef USE_SFML
    #if defined(_WIN32)
        #include <conio.h>
    #else
        #include <termios.h>  // lecture clavier non bloquante
        #include <unistd.h> // usleep : temporisation
    #endif
#endif

#ifdef USE_SFML
    #include <SFML/Graphics.hpp>
    #include <SFML/Window.hpp>
#endif

static void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


// ======================================================
//                   CONSTRUCTEUR DU JEU
// ======================================================
Game::Game() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    width = 40;             // largeur du terrain
    height = 10;            // hauteur du terrain
    playerX = width / 2;    // joueur centré
    lives = 3;              // nombre de vies
    score = 0;              // score initial
    level = 1;              // niveau initial

    enemyDirection = 1;     // direction initiale des ennemis
    enemyMoveCounter = 0;   // compteur pour la vitesse des ennemis
    bossHealth = 0;
    bossMaxHealth = 0;
    spawnWave();            // création de la première vague
    running = true;         // la boucle du jeu est active
}

// ======================================================
//       Fonction utilitaire : lire une touche sans ENTER
// ======================================================
static char getInput() {
#if defined(_WIN32)
    if (_kbhit()) {
        return static_cast<char>(_getch());
    }
    return 0;
#else
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
#endif
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
#endif 

// ======================================================
//                  MISE À JOUR DU JEU
// ======================================================
void Game::update() {

    // -------------------------------
    // Compteur pour gérer vitesse ennemis
    // -------------------------------
    enemyMoveCounter++;

    if (enemyMoveCounter >= enemySpeed) { // seulement si on atteint la vitesse
        enemyMoveCounter = 0;             // reset compteur

        bool mustGoDown = false;

        // vérifier si un ennemi toucherait le mur
        for (const auto& e : enemies) {
            if (e.x + enemyDirection < 0 || e.x + enemyDirection >= width) {
                mustGoDown = true;
                break;
            }
        }

        // si un ennemi touche un mur → descente
        if (mustGoDown) {
            enemyDirection *= -1;            // inverser direction
            for (auto& e : enemies)
                e.y -= 1;                   // descendre d'une ligne
        } else {
            // déplacement horizontal normal
            for (auto& e : enemies)
                e.x += enemyDirection;
        }
    }

    // -------------------------------
    // Tirs ennemis aléatoires
    // -------------------------------
    if (rand() % (20 - std::min(level, 15)) == 0)
        enemyShoot(); // un ennemi tire

    // -------------------------------
    // Déplacement des tirs du joueur vers le haut
    // -------------------------------
    for (std::size_t i = 0; i < bulletsY.size(); i++)
        bulletsY[i]++;

    // -------------------------------
    // Déplacement des tirs ennemis vers le bas
    // -------------------------------
    for (std::size_t i = 0; i < enemyBulletsY.size(); i++)
        enemyBulletsY[i]--;

    // -------------------------------
    // Suppression des tirs ennemis hors écran
    // -------------------------------
    for (int i = static_cast<int>(enemyBulletsY.size()) - 1; i >= 0; i--) {
        if (enemyBulletsY[i] < 0) {
            enemyBulletsX.erase(enemyBulletsX.begin() + i);
            enemyBulletsY.erase(enemyBulletsY.begin() + i);
        }
    }

    // -------------------------------
    // Mise à jour des explosions
    // -------------------------------
    for (int i = 0; i < explosions.size(); i++) {
        explosions[i].timer--;              // réduire timer de l’explosion
        if (explosions[i].timer <= 0) {    // si fini
            explosions.erase(explosions.begin() + i); // supprimer explosion
            i--; // ajuster l’indice après suppression
        }
    }

    // -------------------------------
    // Collision tirs ennemis → joueur
    // -------------------------------
    for (int i = static_cast<int>(enemyBulletsY.size()) - 1; i >= 0; i--) {
        if (enemyBulletsX[i] == playerX && enemyBulletsY[i] == 0) {
            enemyBulletsX.erase(enemyBulletsX.begin() + i);
            enemyBulletsY.erase(enemyBulletsY.begin() + i);

            lives--; // retirer une vie

            if (lives <= 0) {  // plus de vies → GAME OVER
                running = false;
                std::cout << "GAME OVER ! Vies = 0\n";
                usleep(500000);
                return;
            } else {
                std::cout << "Touché ! Vies restantes : " << lives << "\n";
                playerX = width / 2;     // recentrer le joueur
                enemyBulletsX.clear();   // nettoyer tirs ennemis
                enemyBulletsY.clear();
                usleep(300000);
                break;
            }
        }
    }

    // -------------------------------
    // Collision tirs joueur → ennemis
    // -------------------------------
    for (int i =static_cast<int>(bulletsY.size()) - 1; i >= 0; i--) {
        for (int j = static_cast<int>(enemies.size()) - 1; j >= 0; j--) {

            // Si la balle touche un ennemi
            if (bulletsX[i] == enemies[j].x && bulletsY[i] == enemies[j].y) {

                // Ajouter explosion
                explosions.push_back({ enemies[j].x, enemies[j].y, 5 });

                // Gestion boss
                if (enemies[j].isBoss) {
                    bossHealth--;                 // retirer 1 point de vie
                    if (bossHealth <= 0) {        // si boss mort
                        enemies.erase(enemies.begin() + j); // supprimer boss
                        score += 50;              // score plus élevé
                    }
                } else {
                    enemies.erase(enemies.begin() + j);     // supprimer ennemi normal
                    score += 10;                             // ajouter points
                }

                // supprimer balle après collision
                bulletsX.erase(bulletsX.begin() + i);
                bulletsY.erase(bulletsY.begin() + i);

                break; // balle détruite, passer à la suivante
            }
        }
    }

    // -------------------------------
    // Suppression tirs joueur hors écran
    // -------------------------------
    for (int i = static_cast<int>(bulletsY.size()) - 1; i >= 0; i--) {
        if (bulletsY[i] >= height) {
            bulletsX.erase(bulletsX.begin() + i);
            bulletsY.erase(bulletsY.begin() + i);
        }
    }

    // -------------------------------
    // Vérifier si passage à la vague suivante
    // -------------------------------
    if (enemies.empty() && (level % 5 != 0 || bossHealth <= 0)) {
        level++;
        spawnWave(); // nouvelle vague
    }
}

// ======================================================
//                  AFFICHAGE ASCII
// ======================================================
#ifndef USE_SFML

void Game::render() {
    system("clear");

    // -------------------------------
    // Bordure supérieure
    // -------------------------------
    for (int x = 0; x < width + 2; x++) std::cout << "+";
    std::cout << "\n";

    // -------------------------------
    // Affichage HUD
    // -------------------------------
    std::cout << "SCORE : " << score
              << "    VIES : " << lives
              << "    NIVEAU : " << level
              << "\n\n";

    // -------------------------------
    // Grille principale
    // -------------------------------
    for (int y = height - 1; y >= 0; y--) {

        std::cout << "|"; // bordure gauche

        for (int x = 0; x < width; x++) {

            bool printed = false;

            // Tir joueur
            for (std::size_t i = 0; i < bulletsX.size(); i++) {
                if (bulletsX[i] == x && bulletsY[i] == y) {
                    std::cout << '|';
                    printed = true;
                    break;
                }
            }

            // Tir ennemi
            if (!printed) {
                for (std::size_t i = 0; i < enemyBulletsX.size(); i++) {
                    if (enemyBulletsX[i] == x && enemyBulletsY[i] == y) {
                        std::cout << '!';
                        printed = true;
                        break;
                    }
                }
            }

            // Ennemi / Boss
            if (!printed) {
                for (const auto& e : enemies) {
                    if (e.x == x && e.y == y) {
                        if (e.isBoss) std::cout << 'B'; // Boss
                        else std::cout << 'M';           // Ennemi normal
                        printed = true;
                        break;
                    }
                }
            }

            // Explosion
            if (!printed) {
                for (const auto& ex : explosions) {
                    if (ex.x == x && ex.y == y) {
                        std::cout << '*';
                        printed = true;
                        break;
                    }
                }
            }

            // Joueur
            if (!printed) {
                if (y == 0 && x == playerX)
                    std::cout << 'A';
                else
                    std::cout << '.';
            }
        }

        std::cout << "|\n"; // bordure droite
    }

    // -------------------------------
    // Bordure inférieure
    // -------------------------------
    for (int x = 0; x < width + 2; x++) std::cout << "+";
    std::cout << "\n";

    // -------------------------------
    // Affichage vie boss si présent
    // -------------------------------
    if (level % 5 == 0 && bossHealth > 0) {
        std::cout << "BOSS HP : ";
        for (int i = 0; i < bossMaxHealth; i++) {
            if (i < bossHealth) std::cout << "|"; // points de vie actuels
            else std::cout << ".";               // vie perdue
        }
        std::cout << "\n\n";
    }
}
#endif
// ======================================================
//                  Tir du joueur
// ======================================================
void Game::shoot() {
    bulletsX.push_back(playerX);
    bulletsY.push_back(0);
}

// ======================================================
//        Un ennemi tire un projectile vertical
// ======================================================
void Game::enemyShoot() {
    if (enemies.empty()) return;

    // choisir un ennemi au hasard
    int idx = rand() % static_cast<int>(enemies.size());

    enemyBulletsX.push_back(enemies[idx].x);
    enemyBulletsY.push_back(enemies[idx].y - 1);
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
        enemies.push_back(boss);
        bossHealth = 20 + level * 2;  // vie en fonction du niveau
        bossMaxHealth = bossHealth;
        std::cout << "\n--- BOSS LEVEL " << level << " ---\n";
        return ;
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
    #ifndef USE_SFML
        std::cout << "\n--- NIVEAU " << level << " ---\n";
        usleep(500000);
    #endif

    }
}
#ifdef USE_SFML
// ======================================================
//                  BOUCLE SFML (GRAPHique)
// ======================================================
void Game::runSFML(const std::string& fontPath)  {
    const float TICK_SECONDS = 0.10f;

    const int cell = 20;
    const int margin = 20;
    const int hudH = 60;

    const int winW = margin * 2 + width * cell;
    const int winH = margin * 2 + hudH + height * cell;

    sf::RenderWindow window(sf::VideoMode(winW, winH), "Space Invaders (SFML)");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontOk = font.loadFromFile(fontPath);

    sf::Text hud;
    if (fontOk) {
        hud.setFont(font);
        hud.setCharacterSize(18);
        hud.setPosition(static_cast<float>(margin), 10.f);
    }
    auto gridToPixel = [&](int gx, int gy) {
        const float px = static_cast<float>(margin + gx * cell);
        const float py = static_cast<float>(hudH + margin + (height - 1 - gy) * cell);
        return sf::Vector2f(px, py);
    };

}
#endif
