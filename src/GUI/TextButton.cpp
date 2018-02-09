#include <GUI/TextButton.h>

TextButton::~TextButton() = default;

void TextButton::updateTextures()
{
    Button::updateTextures();

    if (pUnpressedTexture == nullptr) {
        invalidateTextures();

        setSurfaces(GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, false, false, textcolor, textshadowcolor), true,
            GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor), true,
            GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, false, true, textcolor, textshadowcolor), true);
    }
}
