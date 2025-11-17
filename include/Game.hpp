class Game {
public:
    Game();
    void run();
private:
    int playerX;
    int width;
    void processInput();
    void update();
    void render();
};
