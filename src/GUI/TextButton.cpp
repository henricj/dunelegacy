#include <GUI/TextButton.h>

TextButton::TextButton() {
    parent::enableResizing(true, true);
}

TextButton::~TextButton() = default;

void TextButton::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture) {
        invalidateTextures();

        const auto& gui      = GUIStyle::getInstance();
        auto* const renderer = dune::globals::renderer.get();

        setTextures(gui.createButtonText(getSize().x, getSize().y, text, false, textcolor, textshadowcolor)
                        .createTexture(renderer),
                    gui.createButtonText(getSize().x, getSize().y, text, true, textcolor, textshadowcolor)
                        .createTexture(renderer),
                    gui.createButtonText(getSize().x, getSize().y, text, true, textcolor, textshadowcolor)
                        .createTexture(renderer));
    }
}
