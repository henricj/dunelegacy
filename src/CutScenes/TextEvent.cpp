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

TextEvent::TextEvent(const std::string& text, Uint32 color, int startFrame, int lengthInFrames, bool bFadeIn, bool bFadeOut, bool bCenterVertical)
 : text(text), startFrame(startFrame), lengthInFrames(lengthInFrames), bFadeIn(bFadeIn), bFadeOut(bFadeOut), bCenterVertical(bCenterVertical)
{
    sdl2::surface_ptr pSurface = pFontManager->createSurfaceWithMultilineText(text, color, 28, true);
    pTexture = sdl2::texture_ptr{ SDL_CreateTextureFromSurface(renderer, pSurface.get()) };

    SDL_SetTextureBlendMode(pTexture.get(), SDL_BLENDMODE_BLEND);
}

TextEvent::~TextEvent() = default;

void TextEvent::draw(int currentFrameNumber) const
{
    if(currentFrameNumber < startFrame || currentFrameNumber > startFrame + lengthInFrames) {
        return;
    }

    int alpha = 255;
    if((bFadeIn == false) && (currentFrameNumber == startFrame)) {
        alpha = 255;
    } else if(bFadeIn && (currentFrameNumber - startFrame <= TEXT_FADE_TIME)) {
        alpha = ((currentFrameNumber - startFrame)*255)/TEXT_FADE_TIME;
    } else if (bFadeOut && ((startFrame + lengthInFrames) - currentFrameNumber <= TEXT_FADE_TIME)) {
        alpha = (((startFrame + lengthInFrames) - currentFrameNumber)*255)/TEXT_FADE_TIME;
    }

    SDL_Rect dest = calcAlignedDrawingRect(pTexture.get(), HAlign::Center, VAlign::Center);
    if(bCenterVertical == false) {
        dest.y = getRendererHeight()/2 + 480/2 - 5*pFontManager->getTextHeight(28)/2;
    }

    SDL_SetTextureAlphaMod(pTexture.get(), alpha);
    SDL_RenderCopy(renderer, pTexture.get(), nullptr, &dest);
}
