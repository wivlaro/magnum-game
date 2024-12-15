#include <cassert>
#include <iomanip>
#include <sstream>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Path.h>
#include <Magnum/Math/Matrix3.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/Alignment.h>
#include <Magnum/Text/Renderer.h>
#include <Magnum/TextureTools/Atlas.h>
#include <Magnum/Math/Color.h>

#include "GameState.h"
#include "MagnumGameApp.h"
#include "Player.h"
#include "Tweakables.h"

namespace MagnumGame {

    static float fontSmoothness = 0.01f;
    static float fontOutlineStart = 0.45f;
    static float fontOutlineEnd = 0.4f;
    static float fontLargeSize = 40.0f;
    static float fontSmallSize = 20.0f;



    void MagnumGameApp::setupTextRenderer() {
        if (auto fontDir = findDirectory("font")) {
            using namespace Corrade::Utility;
            _font = _fontManager.loadAndInstantiate("StbTrueTypeFont");
            assert(_font);

            for (auto& file : *list(*fontDir, Path::ListFlag::SkipDirectories | Path::ListFlag::SkipDotAndDotDot)) {
                auto fontFileName = Path::join(*fontDir, file);
                if (_font->openFile(fontFileName, 80.0f)) {

                    Debug{} << "Loaded font from" <<fontFileName;
                    _fontGlyphCache = Text::DistanceFieldGlyphCacheGL{Vector2i{1024}, Vector2i{512}, 22};
                    _textShader = Shaders::DistanceFieldVectorGL2D{};
                    _font->fillGlyphCache(_fontGlyphCache, "abcdefghijklmnopqrstuvwxyz"
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "*<>0123456789?!:;,.-_ "
                                          "←→😊"
                                          );
                    Debug{} << "Filled glyph cache" << _fontGlyphCache.atlas().filledSize();
                    _debugTextRenderer.emplace(*_font, _fontGlyphCache, 20.0f, Text::Alignment::TopLeft);
                    _debugTextRenderer->reserve(1024, GL::BufferUsage::DynamicDraw, GL::BufferUsage::DynamicDraw);

                    _textIndexBuffer = GL::Buffer{GL::Buffer::TargetHint::ElementArray};
                    _textVertexBuffer = GL::Buffer{GL::Buffer::TargetHint::Array};
                    break;
                }
            }
        }

        _tweakables->addDebugMode("Font", 0, {
            Tweakables::TweakableValue{"Smoothness", &fontSmoothness},
            Tweakables::TweakableValue{"Outline start", &fontOutlineStart},
            Tweakables::TweakableValue{"Outline end", &fontOutlineEnd},
            Tweakables::TweakableValue{"Large size", &fontLargeSize},
            Tweakables::TweakableValue{"Small size", &fontSmallSize},
        });
    }

    inline std::ostream& operator<<(std::ostream& o, const btVector3& vec) {
        o << vec.x() << "," << vec.y() << "," << vec.z();
        return o;
    }
    template<typename T, size_t N>
    std::ostream& operator<<(std::ostream& o, const Magnum::Math::Vector<N,T>& vec) {
        for (size_t i = 0; i < N; ++i) {
            if (i) {
                o << ",";
            }
            o << vec[i];
        }
        return o;
    }


    Vector2 MagnumGameApp::getPlayerControlVector() {
        return {
            ((controllerKeysHeld&KEY_RIGHT)?1.0f:0.0f) - ((controllerKeysHeld&KEY_LEFT)?1.0f:0.0f),
            ((controllerKeysHeld&KEY_FORWARD)?1.0f:0.0f) - ((controllerKeysHeld&KEY_BACKWARD)?1.0f:0.0f),
        };
    }

    void MagnumGameApp::renderGameStatusText() {
        Matrix3 textMatrix{Math::IdentityInit};
        if (false) {
            std::tie(_textMesh, std::ignore) =
                    Text::Renderer2D::render(*_font, _fontGlyphCache, fontLargeSize, "Press Space to Start\nClick to deselect players", _textVertexBuffer, _textIndexBuffer, GL::BufferUsage::DynamicDraw, Text::Alignment::MiddleCenter);
            textMatrix = Matrix3::translation(Vector2{windowSize()} * Vector2{0.f, 0.f});
        }
        else {
            std::ostringstream oss;

            if (_gameState && _gameState->getPlayer()->getBody()) {
                auto rigidBody = _gameState->getPlayer()->getBody();
                oss << std::setprecision(5);
                oss << std::fixed;

                auto onGround = _gameState->getPlayer()->isOnGround();

                oss << "Pos " << rigidBody->transformationMatrix().translation();
                if (onGround) {
                    oss << " OnGround";
                }
                else {
                    oss << " OffGround";
                }
            }
            else {
                oss << "No player";
            }
            std::tie(_textMesh, std::ignore) =
                    Text::Renderer2D::render(*_font, _fontGlyphCache, fontSmallSize, oss.str(), _textVertexBuffer, _textIndexBuffer, GL::BufferUsage::DynamicDraw, Text::Alignment::TopLeft);
            textMatrix = Matrix3::translation(Vector2{windowSize()}*Vector2{-0.45f, 0.45f});
        }

        renderTextBuffer(textMatrix, 0x2f83cc_rgbf, 0xdcdcdc_rgbf, _textMesh);
    }



    void MagnumGameApp::renderDebugText() {
        auto debugText= _tweakables->getDebugText();
        if (_debugTextRenderer && !debugText.empty()) {

            std::tie(_textMesh, std::ignore) =
                Text::Renderer2D::render(*_font, _fontGlyphCache, fontSmallSize, debugText, _textVertexBuffer, _textIndexBuffer, GL::BufferUsage::DynamicDraw, Text::Alignment::BottomLeft);
            auto textMatrix = Matrix3::translation(Vector2{windowSize()}*Vector2{-0.45f, -0.45f});
            renderTextBuffer(textMatrix, 0xffffff_rgbf, 0x111111_rgbf, _textMesh);
        }
    }

    void MagnumGameApp::renderTextBuffer(const Matrix3 &textMatrix, const Color3 &color, const Color3 &outlineColour, GL::Mesh &mesh) {
        _textShader
                .setTransformationProjectionMatrix( Matrix3::projection(Vector2{windowSize()}) * textMatrix)
                .setColor(color)
                .setOutlineColor(outlineColour)
                .setOutlineRange(fontOutlineStart, fontOutlineEnd)
                .setSmoothness(fontSmoothness)
                .bindVectorTexture(_fontGlyphCache.texture())
                .draw(mesh);
    }
};