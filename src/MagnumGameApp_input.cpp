#include "MagnumGameApp.h"
#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/Image.h>
#ifdef MAGNUMGAME_SDL
#include <SDL_events.h>
#include "SdlGameController.h"
#endif

#include "GameState.h"
#include "Player.h"
#include "Tweakables.h"
#include "UserInterface.h"

namespace MagnumGame {
    MagnumGameApp::ControllerKeys MagnumGameApp::getKeyBit(Key key) {
        ControllerKeys keyBit;
        switch (key) {
            case Key::W:
                keyBit = KEY_FORWARD;
                break;
            case Key::S:
                keyBit = KEY_BACKWARD;
                break;
            case Key::A:
                keyBit = KEY_LEFT;
                break;
            case Key::D:
                keyBit = KEY_RIGHT;
                break;
            default:
                keyBit = KEY_NONE;
                break;
        }
        return keyBit;
    }


    Vector2 MagnumGameApp::getPlayerControlVector() const {
        Vector2 control{
            ((_controllerKeysHeld&KEY_RIGHT)?1.0f:0.0f) - ((_controllerKeysHeld&KEY_LEFT)?1.0f:0.0f),
            ((_controllerKeysHeld&KEY_FORWARD)?1.0f:0.0f) - ((_controllerKeysHeld&KEY_BACKWARD)?1.0f:0.0f),
        };

#ifdef MAGNUMGAME_SDL
        if (_gameController) {
            auto gameControllerVector = _gameController->getPlayerDirectionalControlVector();
            if (!gameControllerVector.isZero()) {
                control = gameControllerVector;
            }
        }
#endif

        return control;
    }


    void MagnumGameApp::keyPressEvent(KeyEvent &event) {
        auto key = event.key();
        auto modifiers = event.modifiers();

        if (handleKeyPress(key, modifiers)) {
            event.setAccepted();
        }
        else {
            auto keyBit = getKeyBit(key);
            if (keyBit) {
                _controllerKeysHeld |= keyBit;
                event.setAccepted();
            }
        }
    }

    void MagnumGameApp::keyReleaseEvent(KeyEvent &event) {
        auto keyBit = getKeyBit(event.key());
        if (keyBit) {
            _controllerKeysHeld &= ~keyBit;
            event.setAccepted();
        }
    }

    bool MagnumGameApp::handleKeyPress(Key key, Modifiers modifiers) {
        if (_debugScreen && _debugScreen->handleKeyPress(key, modifiers)) {
            return true;
        }
        if (_currentScreen && _currentScreen->handleKeyPress(key, modifiers)) {
            return true;
        }

        switch (key) {
            case Key::Esc:
                if (isPlaying()) {
                    toPauseScreen();
                }
                else {
                    startGame();
                }
                return true;
            case Key::Space:
                _gameState->getPlayer()->tryJump();
                return true;
            case Key::X:
                _drawDebug = !_drawDebug;
                return true;
            default: break;
        }
        return false;
    }

    void MagnumGameApp::pointerPressEvent(PointerEvent &event) {
        _pointerPressLocations[event.id()] = event.position();

        if (event.pointer() == Pointer::MouseLeft || event.pointer() == Pointer::Finger) {
            _pointerDrag = false;
        }
    }

    Vector2 MagnumGameApp::getUIPosition(Vector2 mousePosition) const {
        auto screenSize = windowSize();
        return Vector2(mousePosition.x(), screenSize.y() - mousePosition.y()) -
               Vector2(screenSize) * 0.5f;
    }

    void MagnumGameApp::pointerMoveEvent(PointerMoveEvent &event) {
        if (!event.pointers() && _currentScreen && _currentScreen->handleMouseMove(getUIPosition(event.position()))) {
            event.setAccepted();
            return;
        }

        if (event.pointers() & (Pointer::MouseLeft | Pointer::Finger)) {
            auto eventPosDelta = event.relativePosition();
            if (eventPosDelta.dot() > 4) {
                _pointerDrag = true;
            }

            _gameState->getCamera()->rotateFromPointer(eventPosDelta);
        }
    }

    void MagnumGameApp::scrollEvent(ScrollEvent &event) {
        if (_gameState) {
            _gameState->getCamera()->adjustZoom(event.offset().y());
        }
    }

    UnsignedInt MagnumGameApp::pickObjectIdAt(Vector2 eventPosition) {
        const Vector2i position = eventPosition * framebufferSize() / windowSize();
        const Vector2i fbPosition{position.x(), GL::defaultFramebuffer.viewport().sizeY() - position.y() - 1};

        /* Read object ID at given click position, and then switch to the color
           attachment again so drawEvent() blits correct buffer */
        _framebuffer.mapForRead(GL::Framebuffer::ColorAttachment{1});
        auto data = _framebuffer.read(
            Range2Di::fromSize(fbPosition, {1, 1}),
            {GL::PixelFormat::RedInteger, GL::PixelType::UnsignedInt});
        _framebuffer.mapForRead(GL::Framebuffer::ColorAttachment{0});

        // One pixel is returned as a 1x1 image
        return data.pixels<UnsignedInt>()[0][0];
    }

    void MagnumGameApp::pointerReleaseEvent(PointerEvent &event) {
        if (_pointerDrag) {
            _pointerDrag = false;
            event.setAccepted();
            return;
        }

        if (_currentScreen) {
            auto pressLocation = _pointerPressLocations.find(event.id());
            if (pressLocation != _pointerPressLocations.end()) {
                if (_currentScreen->handleClick(getUIPosition(pressLocation->second),
                                                getUIPosition(event.position()))) {
                    event.setAccepted();
                    return;
                }
            }
        }

        auto objectId = pickObjectIdAt(event.position());
        Debug{} << "Clicked objectId: " << objectId;

        event.setAccepted();
    }

#ifdef MAGNUMGAME_SDL
    void MagnumGameApp::anyEvent(SDL_Event &event) {
        if (event.type == SDL_WINDOWEVENT_FOCUS_LOST) {
            _controllerKeysHeld = 0;
            return ;
        }
        if (_gameController && _gameController->handleEvent(event, [this](Key key, Modifiers modifiers) { return handleKeyPress(key, modifiers); } )) {
            return;
        }
        // Debug{} << "Unhandled SDL event" << event.type;
    }
#endif
}
