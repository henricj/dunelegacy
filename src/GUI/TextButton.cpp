#include <GUI/TextButton.h>

TextButton::TextButton() {
    parent::enableResizing(true, true);
}

TextButton::~TextButton() = default;

void TextButton::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture) {
        invalidateTextures();

        auto& gui = GUIStyle::getInstance();

        setSurfaces(gui.createButtonText(getSize().x, getSize().y, text, false, textcolor, textshadowcolor),
                    gui.createButtonText(getSize().x, getSize().y, text, true, textcolor, textshadowcolor),
                    gui.createButtonText(getSize().x, getSize().y, text, true, textcolor, textshadowcolor));
    }
}
