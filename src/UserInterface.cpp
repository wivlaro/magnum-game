#include "UserInterface.h"

#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/Math/Matrix3.h>
#include <Magnum/Math/Color.h>

namespace MagnumGame {
    using namespace Magnum::Math::Literals;

    TextAsset::TextAsset(Containers::Pointer<Text::AbstractFont> &&fontIn)
        : font{std::move(fontIn)},
          fontGlyphCache{Vector2i{1024}, Vector2i{512}, 22},
          textShader{} {
        font->fillGlyphCache(fontGlyphCache, "abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "*<>0123456789?!:;,.-_ "
                             "←→😊");
    }

    UIItem::UIItem(Vector2 position)
        : _matrix(Matrix3::translation(position)) {
    }

    Range2D UIItem::getArea() {
        auto pos = _matrix.translation();
        return Range2D(pos, pos);
    }

    UIText::UIText(TextAsset &font, const Vector2 &position, float size, Text::Alignment alignment,
                   Containers::StringView text)
        : UIItem(position)
          , _font(font)
          , _textRenderer{*font.font, font.fontGlyphCache, size, alignment}
          , _text(text) {
        _textRenderer.reserve(1024, GL::BufferUsage::DynamicDraw, GL::BufferUsage::DynamicDraw);
        if (!_text.isEmpty()) {
            _textRenderer.render(_text);
        }
    }


    void UIText::draw(const Matrix3 &projectionMatrix) {
        if (_text.isEmpty()) return;
        _font.textShader
                .setTransformationProjectionMatrix(projectionMatrix * _matrix)
                .setColor(getTextColour())
                .setOutlineColor(getOutlineColour())
                .setOutlineRange(fontOutlineStart, fontOutlineEnd)
                .setSmoothness(fontSmoothness)
                .bindVectorTexture(_font.fontGlyphCache.texture())
                .draw(_textRenderer.mesh());
    }

    Range2D UIText::getArea() {
        if (_text.isEmpty()) return UIItem::getArea();

        auto localRect = _textRenderer.rectangle();
        return Range2D(_matrix.transformPoint(localRect.min()), _matrix.transformPoint(localRect.max()));
    }

    void UIText::setText(Containers::StringView text) {
        _text = text;
        if (_text.isEmpty()) return;
        _textRenderer.render(text);
    }

    UITextButton::UITextButton(TextAsset &font, Vector2 position, float size, Text::Alignment alignment,
                               Containers::StringView text, std::function<void()> onClick)
        : UIText(font, position, size, alignment, text)
          , _onClick(onClick) {
    }


    bool UITextButton::handleKeyPress(MagnumGameApp::Key key, MagnumGameApp::Modifiers ) {
        if (key == MagnumGameApp::Key::Enter) {
            if (_onClick) {
                _onClick();
                return true;
            }
        }
        return false;
    }

    bool UITextButton::handleClick() {
        if (_onClick) {
            _onClick();
            return true;
        }
        return false;
    }

    void UIScreen::draw(const Matrix3 &projectionMatrix) {
        for (auto &item: _items) {
            item->draw(projectionMatrix);
        }
    }

    bool UIScreen::handleKeyPress(MagnumGameApp::Key key, MagnumGameApp::Modifiers modifiers) {
        if (auto item = getSelectedItem()) {
            if (item->handleKeyPress(key, modifiers)) {
                return true;
            }
        }
        if (key == MagnumGameApp::Key::Down) {
            for (auto offset = 1U; offset < _items.size(); ++offset) {
                auto tryIndex = (_selectedItemIndex + offset) % _items.size();
                if (_items[tryIndex]->isSelectable()) {
                    setSelectedIndex(tryIndex);
                    return true;
                }
            }
        } else if (key == MagnumGameApp::Key::Up) {
            for (auto offset = 1U; offset < _items.size(); ++offset) {
                auto tryIndex = (_selectedItemIndex + _items.size() - offset) % _items.size();
                if (_items[tryIndex]->isSelectable()) {
                    setSelectedIndex(tryIndex);
                    return true;
                }
            }
        }
        return false;
    }

    void UIScreen::setSelectedIndex(int itemIndex) {
        if (auto selectedItem = getSelectedItem()) {
            selectedItem->setSelected(false);
        }
        _selectedItemIndex = itemIndex;
        if (auto selectedItem = getSelectedItem()) {
            selectedItem->setSelected(true);
        }
    }

    UserInterface::UserInterface(Containers::Pointer<Text::AbstractFont> &&font)
        : _textAsset(std::move(font)) {
    }

    void UserInterface::draw(Vector2 windowSize, UIScreen &screen) {
        auto projectionMatrix = Matrix3::projection(windowSize);
        screen.draw(projectionMatrix);
    }


    int UIScreen::getSelectableItemIndexAt(Vector2 position) {
        for (auto itemIndex = 0U; itemIndex < _items.size(); ++itemIndex) {
            auto &item = _items[itemIndex];
            if (item->isSelectable()) {
                auto area = item->getArea();
                if (area.contains(position)) {
                    return itemIndex;
                }
            }
        }
        return -1;
    }

    bool UIScreen::handleMouseMove(Vector2 mousePos) {
        auto itemIndex = getSelectableItemIndexAt(mousePos);
        if (itemIndex != -1) {
            setSelectedIndex(itemIndex);
            return true;
        }
        return false;
    }

    bool UIScreen::handleClick(Vector2 pressLocation, Vector2 releaseLocation) {
        auto pressIndex = getSelectableItemIndexAt(pressLocation);
        auto releaseIndex = getSelectableItemIndexAt(releaseLocation);
        if (releaseIndex != -1 && pressIndex == releaseIndex) {
            setSelectedIndex(releaseIndex);
            auto item = getSelectedItem();
            if (item->handleClick()) {
                return true;
            }
        }
        return false;
    }
}
