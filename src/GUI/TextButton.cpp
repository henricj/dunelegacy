#include <GUI/TextButton.h>

TextButton::TextButton() {
    parent::enableResizing(true, true);
}

TextButton::TextButton(TextButton&&) noexcept = default;

TextButton& TextButton::operator=(TextButton&&) noexcept = default;

TextButton::~TextButton() = default;

void TextButton::setText(std::string text) {
    if (text_ == text)
        return;

    text_ = std::move(text);
    resizeAll();
}

void TextButton::setText(std::string_view text) {
    if (text_ != text)
        setText(std::string{text});
}

void TextButton::resize(uint32_t width, uint32_t height) {
    invalidateTextures();
    parent::resize(width, height);
}

void TextButton::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture_) {
        invalidateTextures();

        const auto& gui      = GUIStyle::getInstance();
        auto* const renderer = dune::globals::renderer.get();

        setTextures(gui.createButtonText(getSize().x, getSize().y, text_, false, textcolor_, textshadowcolor_)
                        .createTexture(renderer),
                    gui.createButtonText(getSize().x, getSize().y, text_, true, textcolor_, textshadowcolor_)
                        .createTexture(renderer),
                    gui.createButtonText(getSize().x, getSize().y, text_, true, textcolor_, textshadowcolor_)
                        .createTexture(renderer));
    }
}
