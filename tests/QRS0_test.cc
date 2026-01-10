#include "CoreState.h"
#include "Grid.h"
#include "PieceDefinition.h"
#include "QRS0.h"
#include "Settings.h"
#include "catch2/catch_test_macros.hpp"
#include "game_qs.h"
#include <array>
#include <iostream>
#include <memory>

namespace TestHelpers {
void game_deleter(game_t *game)
{
    game->quit(game);
    free(game);
    game = NULL;
}

struct GameEnvironment
{
    Shiro::Settings settings;
    CoreState coreState;
    std::unique_ptr<game_t, void (*)(game_t *)> game;
    qrsdata *data;

    GameEnvironment()
        : settings()
        , coreState(settings)
        , game(std::unique_ptr<game_t, void (*)(game_t *)>(qs_game_create(&coreState, 0, QRS_PRACTICE, -1), TestHelpers::game_deleter))
        , data((qrsdata *)game->data)
    {
        settings.sfxVolume = 0; // Disable sound effects for tests
        data->p1counters->init = 121;
        data->pracdata->paused = 0;
    }

    void setActivePlayerPiece(const int qrsID)
    {
        const auto &pieceDefinition = getPieceDefinition(qrsID);
        auto *player = data->p1;
        player->def = new Shiro::PieceDefinition(pieceDefinition);
    }

  private:
    const Shiro::PieceDefinition &getPieceDefinition(const int qrsID) const
    {
        for(const auto &piece : data->piecepool)
        {
            if(piece.qrsID == qrsID)
            {
                return piece;
            }
        }
        throw std::runtime_error("Piece definition not found in piecepool");
    }
};
} // namespace TestHelpers

SCENARIO("J-piece jumping obstacle")
{
    GIVEN("A QRS0 game with an obstacle on ground")
    {
        TestHelpers::GameEnvironment env{};
        env.game->field->setCell(6, QRS_FIELD_H - 1, -5); // Set a jumping obstacle

        AND_GIVEN("standing J-piece left of obstacle")
        {
            auto *player = env.data->p1;
            player->state = PSLOCK; // piece on ground but not locked

            env.setActivePlayerPiece(QRS_J4);
            player->x = 5;
            const int yAtGround = ROWTOY(QRS_FIELD_H - 2); // Y of J standing on ground
            player->y = yAtGround;
            player->orient = Shiro::CCW; // stood up

            WHEN("player rotates CW")
            {
                env.coreState.pressed.b = true; // Rotate CW
                env.game->update(true);

                THEN("J-piece should jump over the obstacle")
                {
                    REQUIRE(player->x == 6);         // wallkicks to the right
                    REQUIRE(player->y == yAtGround); // height remains unchanged
                    REQUIRE(player->orient == Shiro::FLAT);
                }
            }
        }
    }
}

SCENARIO("L-piece can't jump obstacle")
{
    GIVEN("A QRS0 game with an obstacle on ground")
    {
        TestHelpers::GameEnvironment env{};
        env.game->field->setCell(4, QRS_FIELD_H - 1, -5); // Set a jumping obstacle

        AND_GIVEN("standing L-piece left of obstacle")
        {
            auto *player = env.data->p1;
            player->state = PSLOCK; // piece on ground but not locked

            env.setActivePlayerPiece(QRS_L4);
            player->x = 5;
            const int yAtGround = ROWTOY(QRS_FIELD_H - 2); // Y of J standing on ground
            player->y = yAtGround;
            player->orient = Shiro::CW; // stood up

            WHEN("player rotates CW")
            {
                env.coreState.pressed.a = true; // Rotate CCW
                env.game->update(true);

                THEN("L-piece should not jump over the obstacle")
                {
                    REQUIRE(player->x == 6);         // wallkicked to the right
                    REQUIRE(player->y == yAtGround); // height remains unchanged
                    REQUIRE(player->orient == Shiro::FLAT);
                }
            }
        }
    }
}

SCENARIO("Auto-synchro S-piece rotation")
{
    // Tests S-piece rotation which requires synchro move.
    // The specific case tested, is visualized here:
    // https://tetris.wiki/File:Synchro_compil_1.gif
    // Or in Fumen format:
    // https://fumen.zui.jp/?v115@VgF8DeE8EeE8EeF8DeG8CeG8CeG8CeG8Men4I
    GIVEN("A QRS0 game with S-piece on a ledge")
    {
        TestHelpers::GameEnvironment env{};
        // setup field
        Shiro::Grid &field = *env.game->field;
        static const int GARBAGE_BLOCK = -5;
        field.fill(Shiro::GridRect{1, QRS_FIELD_H - 4, 7, 4}, GARBAGE_BLOCK);
        field.fill(Shiro::GridRect{1, QRS_FIELD_H - (4 + 4), 6, 4}, GARBAGE_BLOCK);
        field.setCell(6, QRS_FIELD_H - 6, 0);
        field.setCell(6, QRS_FIELD_H - 7, 0);

        // place S-piece on ledge
        env.setActivePlayerPiece(QRS_S4);
        auto *player = env.data->p1;
        player->state = PSLOCK;          // piece on ground but not locked
        player->speeds->grav = 20 * 256; // 20G
        player->x = 8;
        player->y = ROWTOY(QRS_FIELD_H - 6);
        player->orient = Shiro::FLAT;

        WHEN("rotating without DAS")
        {
            env.coreState.pressed.a = true;
            env.game->update(true);

            THEN("S-piece should fall down")
            {
                REQUIRE(player->y == ROWTOY(QRS_FIELD_H - 5));
                REQUIRE(player->x == 8);               // remains in the same column
                REQUIRE(player->orient == Shiro::CCW); // rotated to CW orientation
            }
        }

        WHEN("rotating with DAS (auto-synchro)")
        {
            env.coreState.pressed.a = true;
            env.coreState.keys.left = 1;
            env.coreState.hold_dir = Shiro::DASDirection::LEFT;
            env.coreState.hold_time = 20; // hold for a bit to ensure DAS is active
            env.game->update(true);

            THEN("S-piece should fill gap")
            {
                REQUIRE(player->y == ROWTOY(QRS_FIELD_H - 6)); // remains on the ledge
                REQUIRE(player->x == 7);                       // fills gap
                REQUIRE(player->orient == Shiro::CCW);
            }
        }
    }
}