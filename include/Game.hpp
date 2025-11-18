#include <vector> // nécessaire pour utiliser std::vector

class Game {
public:
    Game();
    void run();
private:
    int playerX;
    int width;
    bool running;

    std::vector<int> bulletsX; // positions X des tirs
    std::vector<int> bulletsY; // positions Y des tirs

    void processInput();
    void update();
    void render();
    void shoot(); // fonction pour ajouter un tir
};
