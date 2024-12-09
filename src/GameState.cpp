#include "GameState.h"

#include <sstream>
#include <Corrade/Utility/DebugStl.h>
#include "Player.h"

namespace MagnumGame {
    GameState::GameState(Timeline& timeline) : _timeline(timeline) {
        _bSphereQueryObject.setCollisionShape(&_bSphereQueryShape);

    }

    void GameState::setupPlayer(RigidBody *rigidBody, Animator *animator) {
        _player = std::make_unique<Player>("Someone", rigidBody, animator);
    }

    // void GameState::doPlayerShoot(Player &player) {
    //     auto pos = player.hitPosition();
    //     _bSphereQueryObject.setWorldTransform(btTransform{btQuaternion::getIdentity(), pos});
    //
    //     auto& _bWorld = player.getWorld();
    //
    //     struct PlayerHitResultCallback : btCollisionWorld::ContactResultCallback {
    //         Player& _firingPlayer;
    //         std::vector<Player>& _players;
    //         btDynamicsWorld& _bWorld;
    //         bool _allowDeath;
    //
    //
    //         explicit PlayerHitResultCallback(Player& hitter, std::vector<Player> &players, btDynamicsWorld& world, bool allowDeath)
    //             : _firingPlayer(hitter), _players(players), _bWorld(world), _allowDeath(allowDeath) {
    //         }
    //
    //         btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int /*partId0*/,
    //                                  int /*index0*/, const btCollisionObjectWrapper *colObj1Wrap, int /*partId1*/, int /*index1*/) override {
    //             auto checkObj =[&](const btCollisionObjectWrapper* objWrap) {
    //                 auto userIndex1 = objWrap->m_collisionObject->getUserIndex();
    //                 if (userIndex1 == MagnumGameApp::World_UserIndex1_Player) {
    //                     auto playerIndex = objWrap->m_collisionObject->getUserIndex2();
    //                     auto& player = _players[playerIndex];
    //                     if (&player != &_firingPlayer) {
    //                         Debug{} << _firingPlayer.getName().c_str() << "hit player" << player.getName().c_str();
    //                         if (_allowDeath) {
    //                             player.die();
    //                         }
    //                     }
    //                 }
    //             };
    //             checkObj(colObj0Wrap);
    //             checkObj(colObj1Wrap);
    //             return cp.m_distance1;
    //         }
    //     } results(player, _players, _bWorld, _allowDeath);
    //     _bWorld.contactTest(&_bSphereQueryObject, results);
    // }

    void GameState::start() {
        _isStarted = true;
    }

    void GameState::update() {

        auto dt = _timeline.previousFrameDuration();


        _player->update(dt);
    }
}
