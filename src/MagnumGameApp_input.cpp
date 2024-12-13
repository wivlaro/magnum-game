
#include "MagnumGameApp.h"
#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/Image.h>

#include "GameState.h"
#include "Tweakables.h"

namespace MagnumGame {

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
        } else if (event.key() == Key::D) {
            _drawDebug = !_drawDebug;
            event.setAccepted();
        } else if (event.key() == Key::Space) {
            _gameState->start();
            event.setAccepted();
        } else {
            auto &debugMode = _tweakables->currentDebugMode();
            if (debugMode.modeName && !debugMode.tweakableValues.empty()) {
                if (event.key() == Key::Down) {
                    debugMode.currentTweakIndex = (debugMode.currentTweakIndex + 1) % debugMode.tweakableValues.size();
                    event.setAccepted();
                } else if (event.key() == Key::Up) {
                    debugMode.currentTweakIndex =
                            (debugMode.currentTweakIndex - 1 + debugMode.tweakableValues.size()) % debugMode.
                            tweakableValues.size();
                    event.setAccepted();
                } else if (event.key() == Key::Left) {
                    auto tweakIndex = debugMode.currentTweakIndex % debugMode.tweakableValues.size();
                    auto p_value = debugMode.tweakableValues[tweakIndex].pValue;
                    *p_value -= getTweakAmount(event, *p_value);
                    event.setAccepted();
                } else if (event.key() == Key::Right) {
                    auto tweakIndex = debugMode.currentTweakIndex % debugMode.tweakableValues.size();
                    auto p_value = debugMode.tweakableValues[tweakIndex].pValue;
                    *p_value += getTweakAmount(event, *p_value);
                    event.setAccepted();
                }
            }
        }
    }

    void MagnumGameApp::mousePressEvent(MouseEvent &event) {
        if (event.button() == MouseEvent::Button::Left) {
            _pointerDrag = false;
        }
    }

    void MagnumGameApp::mouseMoveEvent(MouseMoveEvent &event) {
        if (event.buttons() & MouseMoveEvent::Button::Left) {
            if (event.relativePosition().dot() > 4) {
                _pointerDrag = true;
            }
            if (_pointerDrag) {
                _cameraObject->rotateX(-1.0_degf*event.relativePosition().y())
                        .rotateY(-1.0_degf*event.relativePosition().x());
            }
        }

    }

    UnsignedInt MagnumGameApp::pickObjectIdAt(Vector2i eventPosition) {
        const Vector2i position = eventPosition*Vector2{framebufferSize()}/Vector2{windowSize()};
        const Vector2i fbPosition{static_cast<int>(Math::round(position.x())), static_cast<int>(Math::round(GL::defaultFramebuffer.viewport().sizeY() - position.y() - 1))};

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

    void MagnumGameApp::mouseReleaseEvent(MouseEvent &event) {
        if (_pointerDrag) {
            _pointerDrag = false;
            return;
        }

        auto objectId = pickObjectIdAt(event.position());

        // _gameState->playerClicked(objectId - PlayerIdOffset);

        event.setAccepted();
    }

}
