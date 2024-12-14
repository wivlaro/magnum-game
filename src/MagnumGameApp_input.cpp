
#include "MagnumGameApp.h"
#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/Image.h>
#ifdef MAGNUM_SDL2APPLICATION_MAIN
#include <SDL_events.h>
#endif

#include "GameState.h"
#include "OnGroundQuery.h"
#include "Player.h"
#include "Tweakables.h"

namespace MagnumGame {
    static int getModifiersPower10Adjustment(const MagnumGameApp::KeyEvent &event) {
        auto powerAdjust = -1;
        if (event.modifiers() & MagnumGameApp::Modifier::Shift) {
            powerAdjust -= 1;
        }
        else if (event.modifiers() & (MagnumGameApp::Modifier::Ctrl|MagnumGameApp::Modifier::Alt)) {
            powerAdjust += 1;
        }
        return powerAdjust;
    }

    void MagnumGameApp::keyPressEvent(KeyEvent &event) {
        if (event.key() == Key::PageDown) {
            _tweakables->changeDebugModeIndex(1);
            event.setAccepted();
        } else if (event.key() == Key::PageUp) {
            _tweakables->changeDebugModeIndex(-1);
            event.setAccepted();
        // } else if (event.key() == Key::X) {
        //     _gameState->_allowDeath = !_gameState->_allowDeath;
        //     event.setAccepted();
        } else if (event.key() == Key::X) {
            _drawDebug = !_drawDebug;
            event.setAccepted();
        } else if (event.key() == Key::W) {
            controllerKeysHeld |= KEY_FORWARD;
            event.setAccepted();
        } else if (event.key() == Key::S) {
            controllerKeysHeld |= KEY_BACKWARD;
            event.setAccepted();
        } else if (event.key() == Key::A) {
            controllerKeysHeld |= KEY_LEFT;
            event.setAccepted();
        } else if (event.key() == Key::D) {
            controllerKeysHeld |= KEY_RIGHT;
            event.setAccepted();
        } else if (event.key() == Key::Space) {

            _gameState->getPlayer()->tryJump();

            event.setAccepted();
        } else {
            auto &debugMode = _tweakables->currentDebugMode();
            if (debugMode.getModeName() && debugMode.hasTweakableValues()) {
                if (event.key() == Key::Down) {
                    debugMode.changeCurrentTweaker(1);
                    event.setAccepted();
                } else if (event.key() == Key::Up) {
                    debugMode.changeCurrentTweaker(-1);
                    event.setAccepted();
                } else if (event.key() == Key::Left) {
                    debugMode.currentTweaker().tweakBy(-1, getModifiersPower10Adjustment(event));
                    event.setAccepted();
                } else if (event.key() == Key::Right) {
                    debugMode.currentTweaker().tweakBy(1, getModifiersPower10Adjustment(event));
                    event.setAccepted();
                }
            }
        }
    }

    void MagnumGameApp::keyReleaseEvent(KeyEvent &event) {
        if (event.key() == Key::W) {
            controllerKeysHeld &= ~KEY_FORWARD;
            event.setAccepted();
        } else if (event.key() == Key::S) {
            controllerKeysHeld &= ~KEY_BACKWARD;
            event.setAccepted();
        } else if (event.key() == Key::A) {
            controllerKeysHeld &= ~KEY_LEFT;
            event.setAccepted();
        } else if (event.key() == Key::D) {
            controllerKeysHeld &= ~KEY_RIGHT;
            event.setAccepted();
        }
    }

    void MagnumGameApp::pointerPressEvent(PointerEvent &event) {
        if (event.pointer() == Pointer::MouseLeft || event.pointer() == Pointer::Finger) {
            _pointerDrag = false;
        }
    }

    void MagnumGameApp::pointerMoveEvent(PointerMoveEvent &event) {
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
            return;
        }

        auto objectId = pickObjectIdAt(event.position());
        Debug{} << "Clicked objectId: " << objectId;

        event.setAccepted();
    }

#ifdef MAGNUM_SDL2APPLICATION_MAIN
    void MagnumGameApp::anyEvent(SDL_Event &event) {
        if (event.type == SDL_WINDOWEVENT_FOCUS_LOST) {
            controllerKeysHeld = 0;
        }
    }
#endif

}
