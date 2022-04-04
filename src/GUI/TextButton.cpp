#include <GUI/TextButton.h>

TextButton::~TextButton() = default;

void TextButton::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture) {
        invalidateTextures();

        auto& gui = GUIStyle::getInstance();

        setSurfaces(gui.createButtonSurface(getSize().x, getSize().y, text, false, false, textcolor, textshadowcolor),
                    gui.createButtonSurface(getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor),
                    gui.createButtonSurface(getSize().x, getSize().y, text, false, true, textcolor, textshadowcolor));
    }
}
