#include <vector> // nécessaire pour utiliser std::vector

class Game {
    struct Enemy {
    int x;
    int y;};
public:
    Game();          // constructeur
    void run();
    void update();
    void render();
    void processInput();  
    void shoot();

private:
    int width;       // largeur de l'écran
    int height;      // hauteur de l'écran
    int playerX;     // position du joueur
    bool running;    // état du jeu
    std::vector<int> bulletsX; // positions X des tirs
    std::vector<int> bulletsY; // positions Y des tirs
    

    std::vector<Enemy> enemies;

};
