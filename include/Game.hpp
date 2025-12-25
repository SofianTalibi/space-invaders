#include <vector> // nécessaire pour utiliser std::vector
#include <string>

/**
 * \brief Classe principale du jeu Space Invaders.
 *
 * \details Cette classe contient la logique du jeu (deplacements, collisions,
 *          score, niveaux) et propose deux modes d execution :
 *          - console (ASCII)
 *          - graphique (SFML)
 */

class Game {
    struct Enemy {
    int x;
    int y;
    bool isBoss = false;  // vrai si c’est un boss
    int hp = 1;           // points de vie (1 pour un ennemi normal, plus pour un boss)
    };
    // Structure pour les explosions ASCII
    struct Explosion {
        int x;
        int y;
        int timer;
    };


public:
    Game();          // constructeur
    void run();
#ifdef USE_SFML
    // Version graphique (SFML)
    void runSFML(const std::string& fontPath = "assets/fonts/DejaVuSans.ttf");
#endif
    void update();
    void render();
    void processInput();  
    void shoot();
    void enemyShoot();
    void spawnWave();



    // ------------------
    // Accesseurs (tests / debug)
    // ------------------
    int getScore() const { return score; }
    int getLives() const { return lives; }
    int getLevel() const { return level; }
    int getPlayerX() const { return playerX; }
    std::size_t getEnemyCount() const { return enemies.size(); }
    std::size_t getPlayerBulletCount() const { return bulletsX.size(); }

private:
    int width;       // largeur de l'écran
    int height;      // hauteur de l'écran
    int playerX;     // position du joueur
    bool running;    // état du jeu
    int bossHealth; // vie actuelle du boss
    int bossMaxHealth; // vie maximale du boss

    
    std::vector<int> bulletsX; // positions X des tirs
    std::vector<int> bulletsY; // positions Y des tirs
    
    // stockage des explosions
    std::vector<Explosion> explosions;

    // score du joueur
    int score = 0;
    int level = 1;          // Niveau actuel
    int lives = 3;          // <<< Nombre de vies du joueur
    int enemySpeed = 10;    // Vitesse des ennemis (frames entre mouvements)
    int enemyMoveCounter = 0; // Compteur interne pour ralentir/accélérer les ennemis


    std::vector<Enemy> enemies;

    int enemyDirection; // +1 = droite, -1 = gauche

    std::vector<int> enemyBulletsX;
    std::vector<int> enemyBulletsY;


};
