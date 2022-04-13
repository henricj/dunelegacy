/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <CutScenes/TextEvent.h>
#include <FileClasses/FontManager.h>
#include <globals.h>

TextEvent::TextEvent(const std::string& text, uint32_t color, int startFrame, int lengthInFrames, bool bFadeIn,
                     bool bFadeOut, bool bCenterVertical)
    : text(text), startFrame(startFrame), lengthInFrames(lengthInFrames), bFadeIn(bFadeIn), bFadeOut(bFadeOut),
      bCenterVertical(bCenterVertical) {

    const auto& gui = GUIStyle::getInstance();

    const auto font_size = static_cast<int>(std::round(gui.getScale() * 28));

    text_height_ = pFontManager->getTextHeight(font_size);

    pTexture = pFontManager->createTextureWithMultilineText(text, color, font_size, true);
}

TextEvent::~TextEvent() = default;

void TextEvent::draw(int currentFrameNumber) const {
    if (currentFrameNumber < startFrame || currentFrameNumber > startFrame + lengthInFrames) {
        return;
    }

    uint8_t alpha = 255;
    if (!bFadeIn && currentFrameNumber == startFrame) {
        alpha = 255;
    } else if (bFadeIn && currentFrameNumber - startFrame <= TEXT_FADE_TIME) {
        alpha = static_cast<uint8_t>((currentFrameNumber - startFrame) * 255 / TEXT_FADE_TIME);
    } else if (bFadeOut && startFrame + lengthInFrames - currentFrameNumber <= TEXT_FADE_TIME) {
        alpha = static_cast<uint8_t>((startFrame + lengthInFrames - currentFrameNumber) * 255 / TEXT_FADE_TIME);
    }

    const auto& gui = GUIStyle::getInstance();

    const auto scale = 1.f / gui.getZoom();

    const auto render_size = getRendererSize();
    const auto render_w    = static_cast<float>(render_size.w);
    const auto render_h    = static_cast<float>(render_size.h);

    int w, h;
    if (0 != SDL_QueryTexture(pTexture.get(), nullptr, nullptr, &w, &h))
        return;

    const auto scaled_w = static_cast<float>(w) * scale;
    const auto scaled_h = static_cast<float>(h) * scale;

    const auto scaled_text_height = static_cast<float>(text_height_) * scale;

    const auto dest_y = bCenterVertical ? (render_h - scaled_h) / 2 : (render_h + 480.f - 5.f * scaled_text_height) / 2;

    const SDL_FRect dest{(render_w - scaled_w) / 2, dest_y, scaled_w, scaled_h};

    SDL_SetTextureAlphaMod(pTexture.get(), alpha);
    Dune_RenderCopyF(renderer, pTexture.get(), nullptr, &dest);
}
