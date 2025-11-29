#include <SFML/Graphics.hpp>

int main() {
    // Création d'une fenêtre SFML
    sf::RenderWindow window(sf::VideoMode(800, 400), "Space Invaders");

    // Boucle principale
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close(); // fermer la fenêtre si on clique sur la croix
        }

        window.clear(); // effacer l'écran (fond noir par défaut)
        window.display(); // afficher tout ce qui est dessiné
    }

    return 0;
}
