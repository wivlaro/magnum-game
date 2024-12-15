#pragma once

#include <Corrade/Containers/String.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Pointer.h>
#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Math/Matrix3.h>
#include <Magnum/Shaders/DistanceFieldVectorGL.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/DistanceFieldGlyphCacheGL.h>
#include <Magnum/Text/Renderer.h>

#include "MagnumGameApp.h"
#include "MagnumGameCommon.h"

namespace MagnumGame {
    using namespace Corrade;
    using namespace Magnum;

    static inline Color4 textColour = 0xffffff_rgbf;
    static inline Color4 selectedTextColour = 0xffccaa_rgbf;
    static inline Color4 outlineColour = 0x111111_rgbf;

    static inline float fontSmoothness = 0.01f;
    static inline float fontOutlineStart = 0.45f;
    static inline float fontOutlineEnd = 0.4f;
    static inline float fontLargeSize = 40.0f;
    static inline float fontSmallSize = 20.0f;

    struct TextAsset {
        Containers::Pointer<Text::AbstractFont> font;
        Text::DistanceFieldGlyphCacheGL fontGlyphCache;
        Shaders::DistanceFieldVectorGL2D textShader;

        explicit TextAsset(Containers::Pointer<Text::AbstractFont> &&font);

        DISALLOW_COPY(TextAsset);
    };

    class UIItem {
    public:
        explicit UIItem(Vector2 position);
        virtual ~UIItem() = default;
        virtual void draw(const Matrix3 &projectionMatrix) = 0;
        virtual bool isSelectable() { return false;}
        virtual Range2D getArea();

        UIItem& setPosition(const Vector2& position) {
            _matrix = Matrix3::translation(position);
            return *this;
        }

        virtual void setSelected(bool ) {}

        virtual bool handleKeyPress(MagnumGameApp::Key , MagnumGameApp::Modifiers ) { return false; }

        virtual bool handleClick() { return false; }

    protected:
        Matrix3 _matrix{Math::IdentityInit};

        UIItem() = default;

    };

    class UIText : public UIItem {
    public:
        explicit UIText(TextAsset &font, const Vector2 &position, float size, Text::Alignment alignment, Containers::StringView text = {});
        ~UIText() override = default;

        virtual Color4 getTextColour() const { return textColour;}

        virtual Color4 getOutlineColour() const { return outlineColour;}


        void draw(const Matrix3 &projectionMatrix) override;

        Range2D getArea() override;

        void setText(Containers::StringView text);

    private:
        TextAsset& _font;
        Text::Renderer2D _textRenderer;
        Containers::String _text{};
    };

    class UITextButton : public UIText {
    public:
        explicit UITextButton(TextAsset &font, Vector2 position, float size, Text::Alignment alignment, Containers::StringView text = {}, std::function<void()> onClick = {});
        ~UITextButton() override = default;
        bool isSelectable() override { return true; }

        Color4 getTextColour() const override { return _isSelected ? selectedTextColour : textColour; }

        void setSelected(bool selected) override { _isSelected = selected; }

        bool handleKeyPress(MagnumGameApp::Key key, MagnumGameApp::Modifiers modifiers) override;

        bool handleClick() override;

    private:
        bool _isSelected { false };
        std::function<void()> _onClick;
    };

    class UIScreen {
    public:
        void draw(const Matrix3& matrix3);

        UIItem* getItemByIndex(int index) { return index != -1 ? _items[index].get() : nullptr; }
        UIItem* getSelectedItem() { return getItemByIndex(_selectedItemIndex); }

        int getSelectableItemIndexAt(Vector2 position);
        UIItem* getSelectableItemAt(Vector2 position) { return getItemByIndex(getSelectableItemIndexAt(position)); }

        bool handleMouseMove(Vector2 mousePos);
        bool handleClick(Vector2 mousePos);
        bool handleKeyPress(MagnumGameApp::Key key, MagnumGameApp::Modifiers modifiers);

        template<class U, class ...Args> U* addItem(Args&&... args) {
            return static_cast<U*>(Containers::arrayAppend(_items, InPlaceInit, new U(std::forward<Args>(args)...)).get());
        }

        bool handleClick(Vector2 pressLocation, Vector2 releaseLocation);

        void setSelectedIndex(int itemIndex);


    private:
        Containers::Array<Containers::Pointer<UIItem>> _items{};
        int _selectedItemIndex{-1};
    };

    class UserInterface {
    public:
        explicit UserInterface(Containers::Pointer<Text::AbstractFont>&& font);

        void draw(Vector2 windowSize, UIScreen& uiScreen);

        TextAsset& getTextAsset() { return _textAsset; }

    private:
        TextAsset _textAsset;
    };
}
