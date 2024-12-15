#include <cassert>
#include <iomanip>
#include <sstream>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Path.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/Alignment.h>
#include <Magnum/TextureTools/Atlas.h>

#include "GameState.h"
#include "MagnumGameApp.h"
#include "Player.h"
#include "Tweakables.h"
#include "UserInterface.h"

namespace MagnumGame {


    void MagnumGameApp::setupUserInterface() {
        using namespace Corrade::Utility;
        auto font = _fontManager.loadAndInstantiate("StbTrueTypeFont");
        assert(font);

        auto fontsDir = _assets->getFontsDir();
        for (auto& file : *list(fontsDir, Path::ListFlag::SkipDirectories | Path::ListFlag::SkipDotAndDotDot)) {
            auto fontFileName = Path::join(fontsDir, file);
            if (font->openFile(fontFileName, 80.0f)) {

                _ui.emplace(std::move(font));
                break;
            }
        }

        _hudScreen.emplace();
        _statusText = _hudScreen->addItem<UIText>(_ui->getTextAsset(), Vector2{windowSize()}*Vector2{-0.45f, 0.45f}, fontSmallSize, Text::Alignment::TopLeft);

        _menuScreen.emplace();
        _menuScreen->addItem<UIText>(_ui->getTextAsset(), Vector2{0,fontLargeSize}, fontLargeSize, Text::Alignment::MiddleCenter, "Magnum Game");

        _menuScreen->addItem<UITextButton>(_ui->getTextAsset(), Vector2{0,-fontLargeSize}, fontLargeSize, Text::Alignment::MiddleCenter, "Play", [this] { startGame(); });
        _menuScreen->addItem<UITextButton>(_ui->getTextAsset(), Vector2{0,-fontLargeSize*2.2f}, fontLargeSize, Text::Alignment::MiddleCenter, "Quit", [this] { exit(); });

        _debugScreen.emplace();
        _debugText = _debugScreen->addItem<TweakableUIText>(_ui->getTextAsset(), Vector2{windowSize()}*Vector2{-0.45,-0.45f}, fontLargeSize, Text::Alignment::BottomLeft, *_tweakables);
        _debugScreen->setSelectedIndex(0);

        _currentScreen = _menuScreen.get();

        _tweakables->addDebugMode("Font", 0, {
            Tweakables::TweakableValue{"Smoothness", &fontSmoothness},
            Tweakables::TweakableValue{"Outline start", &fontOutlineStart},
            Tweakables::TweakableValue{"Outline end", &fontOutlineEnd},
            Tweakables::TweakableValue{"Large size", &fontLargeSize},
            Tweakables::TweakableValue{"Small size", &fontSmallSize},
        });
    }

    Vector2 MagnumGameApp::getPlayerControlVector() const {
        return {
            ((_controllerKeysHeld&KEY_RIGHT)?1.0f:0.0f) - ((_controllerKeysHeld&KEY_LEFT)?1.0f:0.0f),
            ((_controllerKeysHeld&KEY_FORWARD)?1.0f:0.0f) - ((_controllerKeysHeld&KEY_BACKWARD)?1.0f:0.0f),
        };
    }

    void MagnumGameApp::updateStatusText() {
        std::ostringstream oss;
        if (_gameState && _gameState->getPlayer()->getBody()) {
            auto rigidBody = _gameState->getPlayer()->getBody();
            oss << std::setprecision(3);
            oss << std::fixed;

            Debug debug{&oss};
            debug << "Pos " << rigidBody->transformationMatrix().translation();
            if (_gameState->getPlayer()->isOnGround()) {
                debug << " On Ground";
            }
            else {
                debug << " Off Ground";
            }
        }
        else {
            oss << "No player";
        }
        _statusText->setText(oss.str());
    }
};