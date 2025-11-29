#include <vector> // nécessaire pour utiliser std::vector

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
    void update();
    void render();
    void processInput();  
    void shoot();
    void enemyShoot();
    void spawnWave();



private:
    int width;       // largeur de l'écran
    int height;      // hauteur de l'écran
    int playerX;     // position du joueur
    bool running;    // état du jeu
    
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
