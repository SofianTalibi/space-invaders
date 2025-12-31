#include <iostream>
#include <vector>
#include <stdio.h>
#include "Game.hpp"
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <ctime>
#include <unistd.h> // usleep : temporisation

#ifndef USE_SFML
    #if defined(_WIN32)
        #include <conio.h>
        #include <windows.h>

    #else
        #include <termios.h>  // lecture clavier non bloquante
        #include <fcntl.h> 
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
//      Fonction auxiliaire :configurer le mode du terminal
// ======================================================

#ifndef USE_SFML
static struct termios orig_termios; 

static void enableRawMode() {
#if !defined(_WIN32)
    tcgetattr(0, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &raw);
    std::cout << "\033[?25l";
#endif
}

static void disableRawMode() {
#if !defined(_WIN32)
    tcsetattr(0, TCSANOW, &orig_termios);
    std::cout << "\033[?25h"; 
#endif
}
#endif
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
//                     BOUCLE PRINCIPALE
// ======================================================
#ifndef USE_SFML  
void Game::run() {
    enableRawMode();
    while (running) {
        render();              // afficher l’état du jeu
    #if defined(_WIN32)
        if (_kbhit()) {
            char c = _getch();
            if (c == 'a' && playerX > 0) playerX--;
            if (c == 'd' && playerX < width - 1) playerX++;
            if (c == 't') shoot();
            if (c == 'q') running = false;
        }
    #else
        char buf;
        while (read(0, &buf, 1) > 0) {
            if (buf == 'a' && playerX > 0) playerX--;
            if (buf == 'd' && playerX < width - 1) playerX++;
            if (buf == 't') shoot();
            if (buf == 'q') running = false;
        }
    #endif

        update();             
        sleep_ms(30);       
    }
    
    disableRawMode(); 
    system("clear");          
    std::cout << "GAME OVER / QUIT\nFinal Score: " << score << "\n"; 
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
    if (rand() % (40 - std::min(level*2, 30)) == 0)
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
     std::cout << "\033[H";

    // -------------------------------
    // Bordure supérieure
    // -------------------------------
    for (int x = 0; x < width + 2; x++)  std::cout << "+";
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
        std::cout << std::flush;
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
    enemySpeed = std::max(2, 15 - level);   // vitesse ennemis

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

    // -------------------------------
    // Gestion de la vue (mise à l'échelle / letterboxing)
    // -------------------------------
    sf::View view(sf::FloatRect(0.f, 0.f, static_cast<float>(winW), static_cast<float>(winH)));
    window.setView(view);

    auto updateView = [&](unsigned int w, unsigned int h) {
        const float windowRatio = static_cast<float>(w) / static_cast<float>(h);
        const float viewRatio   = static_cast<float>(winW) / static_cast<float>(winH);

        float sizeX = 1.f, sizeY = 1.f;
        float posX  = 0.f, posY  = 0.f;

        // Letterboxing pour conserver les proportions du "monde" logique
        if (windowRatio > viewRatio) {
            sizeX = viewRatio / windowRatio;
            posX  = (1.f - sizeX) * 0.5f;
        } else {
            sizeY = windowRatio / viewRatio;
            posY  = (1.f - sizeY) * 0.5f;
        }

        view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
        window.setView(view);
    };

    // Valeur initiale
    updateView(static_cast<unsigned int>(winW), static_cast<unsigned int>(winH));

    // -------------------------------
    // Fond étoilé (sans assets)
    // -------------------------------
    struct Star {
        sf::Vector2f pos;
        float speed;      // pixels par seconde
        float radius;
        sf::Color color;
    };

    std::vector<Star> stars;
    stars.reserve(200);

    for (int i = 0; i < 200; ++i) {
        Star s;
        s.pos = sf::Vector2f(
            static_cast<float>(std::rand() % winW),
            static_cast<float>(std::rand() % winH)
        );
        s.speed  = 20.f + static_cast<float>(std::rand() % 80);
        s.radius = 1.f + static_cast<float>(std::rand() % 2);
        const sf::Uint8 c = static_cast<sf::Uint8>(180 + (std::rand() % 75));
        s.color = sf::Color(c, c, c);
        stars.push_back(s);
    }

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
    sf::Clock clock;
    float accumulator = 0.f;

    bool requestShoot = false;
    float shootCooldown = 0.f;
     while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == sf::Keyboard::Space) {
                    requestShoot = true;
                }
            }
            // -------------------------------
            // Redimensionnement : on met à jour la vue pour garder les proportions
            // (sinon l'image est étirée)
            // -------------------------------
            if (event.type == sf::Event::Resized) {
                updateView(event.size.width, event.size.height);
            }
        }

        const float dt = clock.restart().asSeconds();
        accumulator += dt;
        shootCooldown = std::max(0.f, shootCooldown - dt);
        // -------------------------------
        // Mise à jour du fond étoilé
        // -------------------------------
        // Les étoiles "descendent" (effet de mouvement), puis réapparaissent en haut
        // quand elles sortent de l'écran.
        for (auto& s : stars) {
            s.pos.y += s.speed * dt;
            if (s.pos.y > static_cast<float>(winH)) {
                s.pos.y = 0.f;
                s.pos.x = static_cast<float>(std::rand() % winW);
            }
        }

        // Inputs temps-réel (déplacement continu)
        const bool leftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        const bool rightHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);

        while (accumulator >= TICK_SECONDS && running && window.isOpen()) {
            accumulator -= TICK_SECONDS;

            if (leftHeld && playerX > 0) playerX--;
            if (rightHeld && playerX < width - 1) playerX++;

            if (requestShoot && shootCooldown <= 0.f) {
                shoot();
                shootCooldown = 0.25f;
            }
            requestShoot = false;

            update();
        }
         window.clear(sf::Color(10, 10, 18));
        // -------------------------------
        // Dessin du fond étoilé
        // -------------------------------
        // On dessine des petits cercles blancs/gris en arrière-plan.
        for (const auto& s : stars) {
            sf::CircleShape star(s.radius);
            star.setFillColor(s.color);
            star.setPosition(s.pos);
            window.draw(star);
        }

        sf::RectangleShape border(sf::Vector2f(static_cast<float>(width * cell), static_cast<float>(height * cell)));
        border.setPosition(static_cast<float>(margin), static_cast<float>(hudH + margin));
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(2.f);
        border.setOutlineColor(sf::Color(120, 120, 160));
        window.draw(border);

        {
            sf::RectangleShape player(sf::Vector2f(cell * 0.9f, cell * 0.6f));
            player.setFillColor(sf::Color(80, 220, 140));
            const auto p = gridToPixel(playerX, 0);
            player.setPosition(p.x + cell * 0.05f, p.y + cell * 0.25f);
            window.draw(player);
        }
           for (const auto& e : enemies) {
            sf::Vector2f size(cell * 0.85f, cell * 0.65f);
            if (e.isBoss) size = sf::Vector2f(cell * 1.3f, cell * 0.9f);

            sf::RectangleShape alien(size);
            alien.setFillColor(e.isBoss ? sf::Color(240, 140, 80) : sf::Color(200, 80, 220));

            const auto p = gridToPixel(e.x, e.y);
            alien.setPosition(p.x + (cell - size.x) * 0.5f, p.y + (cell - size.y) * 0.5f);
            window.draw(alien);
        }

        for (std::size_t i = 0; i < bulletsX.size(); i++) {
            sf::RectangleShape b(sf::Vector2f(cell * 0.15f, cell * 0.6f));
            b.setFillColor(sf::Color(240, 240, 240));
            const auto p = gridToPixel(bulletsX[i], bulletsY[i]);
            b.setPosition(p.x + cell * 0.425f, p.y + cell * 0.2f);
            window.draw(b);
        }

        for (std::size_t i = 0; i < enemyBulletsX.size(); i++) {
            sf::RectangleShape b(sf::Vector2f(cell * 0.15f, cell * 0.6f));
            b.setFillColor(sf::Color(255, 120, 120));
            const auto p = gridToPixel(enemyBulletsX[i], enemyBulletsY[i]);
            b.setPosition(p.x + cell * 0.425f, p.y + cell * 0.2f);
            window.draw(b);
        }

        for (const auto& ex : explosions) {
            sf::CircleShape c(cell * 0.35f);
            c.setFillColor(sf::Color(255, 210, 60, static_cast<sf::Uint8>(40 + ex.timer * 40)));
            const auto p = gridToPixel(ex.x, ex.y);
            c.setPosition(p.x + cell * 0.15f, p.y + cell * 0.15f);
            window.draw(c);
        }

        if (fontOk) {
            hud.setString(
                "Score: " + std::to_string(score) +
                "   Lives: " + std::to_string(lives) +
                "   Level: " + std::to_string(level) +
                "   (Move: A/D or < / >, Shoot: Space, Quit: Esc)"
            );
            window.draw(hud);

            if (level % 5 == 0 && bossHealth > 0) {
                sf::Text bossTxt;
                bossTxt.setFont(font);
                bossTxt.setCharacterSize(18);
                bossTxt.setPosition(static_cast<float>(margin), 32.f);
                bossTxt.setString("Boss HP: " + std::to_string(bossHealth) + "/" + std::to_string(bossMaxHealth));
                window.draw(bossTxt);
            }
        }

        window.display();

        if (!running) {
            window.close();

        }
    }
}
#endif
