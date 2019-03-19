#include <GUI/TextButton.h>

TextButton::~TextButton() = default;

void TextButton::updateTextures()
{
    Button::updateTextures();

    if (!pUnpressedTexture) {
        invalidateTextures();

        setSurfaces(GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, false, false, textcolor, textshadowcolor),
            GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor),
            GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, false, true, textcolor, textshadowcolor));
    }
}
