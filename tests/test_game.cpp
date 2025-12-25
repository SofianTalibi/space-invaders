#include <catch2/catch_test_macros.hpp>
#include "Game.hpp"

/**
 * \brief Tests unitaires de base sur la logique du jeu.
 *
 * \details On teste ici des invariants simples et reproductibles.
 *          L’objectif est d’avoir des tests stables qui valident la logique,
 *          sans dépendre de l’interface SFML.
 */

TEST_CASE("Etat initial du jeu", "[game]") {
    Game g;

    CHECK(g.getScore() == 0);
    CHECK(g.getLives() == 3);
    CHECK(g.getLevel() == 1);

    // Au niveau 1, une vague d'ennemis doit être présente.
    CHECK(g.getEnemyCount() > 0);
}

TEST_CASE("Tir du joueur : un projectile est cree", "[game]") {
    Game g;

    const auto before = g.getPlayerBulletCount();
    g.shoot();
    const auto after = g.getPlayerBulletCount();

    CHECK(after == before + 1);
}
