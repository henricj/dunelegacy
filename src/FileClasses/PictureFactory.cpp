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

#include <FileClasses/PictureFactory.h>

#include <globals.h>

#include <config.h>

#include <FileClasses/Cpsfile.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/Wsafile.h>

#include <misc/Scaler.h>
#include <misc/draw_util.h>
#include <misc/exceptions.h>

#include <memory>

PictureFactory::PictureFactory() {
    auto ScreenPic = LoadCPS_RW(pFileManager->openFile("SCREEN.CPS").get());
    if (ScreenPic == nullptr) {
        THROW(std::runtime_error, "PictureFactory::PictureFactory(): Cannot read SCREEN.CPS!");
    }

    auto FamePic = LoadCPS_RW(pFileManager->openFile("FAME.CPS").get());
    if (FamePic == nullptr) {
        THROW(std::runtime_error, "PictureFactory::PictureFactory(): Cannot read FAME.CPS!");
    }

    auto ChoamPic = LoadCPS_RW(pFileManager->openFile("CHOAM.CPS").get());
    if (ChoamPic == nullptr) {
        THROW(std::runtime_error, "PictureFactory::PictureFactory(): Cannot read CHOAM.CPS!");
    }

    creditsBorder = getSubPicture(ScreenPic.get(), 257, 2, 63, 13);

    // background
    background = sdl2::surface_ptr {SDL_CreateRGBSurface(0, settings.video.width, settings.video.height, 8, 0, 0, 0, 0)};
    if (background == nullptr) {
        THROW(std::runtime_error, "PictureFactory::PictureFactory: Cannot create new Picture!");
    }
    palette.applyToSurface(background.get());

    auto PatternNormal    = getSubPicture(FamePic.get(), 0, 1, 63, 67);
    auto PatternHFlipped  = flipHSurface(getSubPicture(FamePic.get(), 0, 1, 63, 67).get());
    auto PatternVFlipped  = flipVSurface(getSubPicture(FamePic.get(), 0, 1, 63, 67).get());
    auto PatternHVFlipped = flipHSurface(flipVSurface(getSubPicture(FamePic.get(), 0, 1, 63, 67).get()).get());

    SDL_Rect dest;
    dest.w = 63;
    dest.h = 67;
    for (dest.y = 0; dest.y < settings.video.height; dest.y += 67) {
        for (dest.x = 0; dest.x < settings.video.width; dest.x += 63) {
            if ((dest.x % (63 * 2) == 0) && (dest.y % (67 * 2) == 0)) {
                SDL_Rect tmpDest = dest;
                SDL_BlitSurface(PatternNormal.get(), nullptr, background.get(), &tmpDest);
            } else if ((dest.x % (63 * 2) != 0) && (dest.y % (67 * 2) == 0)) {
                SDL_Rect tmpDest = dest;
                SDL_BlitSurface(PatternHFlipped.get(), nullptr, background.get(), &tmpDest);
            } else if ((dest.x % (63 * 2) == 0) && (dest.y % (67 * 2) != 0)) {
                SDL_Rect tmpDest = dest;
                SDL_BlitSurface(PatternVFlipped.get(), nullptr, background.get(), &tmpDest);
            } else /*if((dest.x % (63*2) != 0) && (dest.y % (67*2) != 0))*/ {
                SDL_Rect tmpDest = dest;
                SDL_BlitSurface(PatternHVFlipped.get(), nullptr, background.get(), &tmpDest);
            }
        }
    }

    // decoration border
    decorationBorder.ball    = getSubPicture(ScreenPic.get(), 241, 124, 12, 11);
    decorationBorder.vspacer = getSubPicture(ScreenPic.get(), 241, 118, 12, 5);
    decorationBorder.hspacer = rotateSurfaceRight(decorationBorder.vspacer.get());
    decorationBorder.vborder = getSubPicture(ScreenPic.get(), 241, 71, 12, 13);
    decorationBorder.hborder = rotateSurfaceRight(decorationBorder.vborder.get());

    // simple Frame
    frame[SimpleFrame].leftUpperCorner = getSubPicture(ChoamPic.get(), 120, 17, 8, 8);
    putPixel(frame[SimpleFrame].leftUpperCorner.get(), 7, 7, 0);
    putPixel(frame[SimpleFrame].leftUpperCorner.get(), 6, 7, 0);
    putPixel(frame[SimpleFrame].leftUpperCorner.get(), 7, 6, 0);

    frame[SimpleFrame].rightUpperCorner = getSubPicture(ChoamPic.get(), 312, 17, 8, 8);
    putPixel(frame[SimpleFrame].rightUpperCorner.get(), 0, 7, 0);
    putPixel(frame[SimpleFrame].rightUpperCorner.get(), 0, 6, 0);
    putPixel(frame[SimpleFrame].rightUpperCorner.get(), 1, 7, 0);

    frame[SimpleFrame].leftLowerCorner = getSubPicture(ChoamPic.get(), 120, 31, 8, 8);
    putPixel(frame[SimpleFrame].leftLowerCorner.get(), 7, 0, 0);
    putPixel(frame[SimpleFrame].leftLowerCorner.get(), 6, 0, 0);
    putPixel(frame[SimpleFrame].leftLowerCorner.get(), 7, 1, 0);

    frame[SimpleFrame].rightLowerCorner = getSubPicture(ChoamPic.get(), 312, 31, 8, 8);
    putPixel(frame[SimpleFrame].rightLowerCorner.get(), 0, 0, 0);
    putPixel(frame[SimpleFrame].rightLowerCorner.get(), 1, 0, 0);
    putPixel(frame[SimpleFrame].rightLowerCorner.get(), 0, 1, 0);

    frame[SimpleFrame].hborder = getSubPicture(ChoamPic.get(), 128, 17, 1, 4);
    frame[SimpleFrame].vborder = getSubPicture(ChoamPic.get(), 120, 25, 4, 1);

    // Decoration Frame 1
    frame[DecorationFrame1].leftUpperCorner = getSubPicture(ChoamPic.get(), 2, 57, 11, 12);
    putPixel(frame[DecorationFrame1].leftUpperCorner.get(), 10, 11, 0);
    putPixel(frame[DecorationFrame1].leftUpperCorner.get(), 9, 11, 0);
    putPixel(frame[DecorationFrame1].leftUpperCorner.get(), 10, 10, 0);

    frame[DecorationFrame1].rightUpperCorner = getSubPicture(ChoamPic.get(), 44, 57, 11, 12);
    putPixel(frame[DecorationFrame1].rightUpperCorner.get(), 0, 11, 0);
    putPixel(frame[DecorationFrame1].rightUpperCorner.get(), 0, 10, 0);
    putPixel(frame[DecorationFrame1].rightUpperCorner.get(), 1, 11, 0);

    frame[DecorationFrame1].leftLowerCorner = getSubPicture(ChoamPic.get(), 2, 132, 11, 11);
    putPixel(frame[DecorationFrame1].leftLowerCorner.get(), 10, 0, 0);
    putPixel(frame[DecorationFrame1].leftLowerCorner.get(), 9, 0, 0);
    putPixel(frame[DecorationFrame1].leftLowerCorner.get(), 10, 1, 0);

    frame[DecorationFrame1].rightLowerCorner = getSubPicture(ChoamPic.get(), 44, 132, 11, 11);
    putPixel(frame[DecorationFrame1].rightLowerCorner.get(), 0, 0, 0);
    putPixel(frame[DecorationFrame1].rightLowerCorner.get(), 1, 0, 0);
    putPixel(frame[DecorationFrame1].rightLowerCorner.get(), 0, 1, 0);

    frame[DecorationFrame1].hborder = getSubPicture(ChoamPic.get(), 13, 57, 1, 4);
    frame[DecorationFrame1].vborder = getSubPicture(ChoamPic.get(), 2, 69, 4, 1);

    // Decoration Frame 2
    frame[DecorationFrame2].leftUpperCorner = getSubPicture(ChoamPic.get(), 121, 41, 9, 9);
    drawHLine(frame[DecorationFrame2].leftUpperCorner.get(), 6, 6, 8, 0);
    drawHLine(frame[DecorationFrame2].leftUpperCorner.get(), 6, 7, 8, 0);
    drawHLine(frame[DecorationFrame2].leftUpperCorner.get(), 6, 8, 8, 0);

    frame[DecorationFrame2].rightUpperCorner = getSubPicture(ChoamPic.get(), 309, 41, 10, 9);
    drawHLine(frame[DecorationFrame2].rightUpperCorner.get(), 0, 6, 3, 0);
    drawHLine(frame[DecorationFrame2].rightUpperCorner.get(), 0, 7, 3, 0);
    drawHLine(frame[DecorationFrame2].rightUpperCorner.get(), 0, 8, 3, 0);

    frame[DecorationFrame2].leftLowerCorner = getSubPicture(ChoamPic.get(), 121, 157, 9, 10);
    drawHLine(frame[DecorationFrame2].leftLowerCorner.get(), 6, 0, 8, 0);
    drawHLine(frame[DecorationFrame2].leftLowerCorner.get(), 6, 1, 8, 0);
    drawHLine(frame[DecorationFrame2].leftLowerCorner.get(), 6, 2, 8, 0);
    drawHLine(frame[DecorationFrame2].leftLowerCorner.get(), 7, 3, 8, 0);

    frame[DecorationFrame2].rightLowerCorner = getSubPicture(ChoamPic.get(), 309, 158, 10, 9);
    drawHLine(frame[DecorationFrame2].rightLowerCorner.get(), 0, 0, 3, 0);
    drawHLine(frame[DecorationFrame2].rightLowerCorner.get(), 0, 1, 3, 0);
    drawHLine(frame[DecorationFrame2].rightLowerCorner.get(), 0, 2, 3, 0);

    frame[DecorationFrame2].hborder = getSubPicture(ChoamPic.get(), 133, 41, 1, 4);
    frame[DecorationFrame2].vborder = getSubPicture(ChoamPic.get(), 121, 51, 4, 1);

    for (auto& f : frame) {
        SDL_SetColorKey(f.leftUpperCorner.get(), SDL_TRUE, 0);
        SDL_SetColorKey(f.leftLowerCorner.get(), SDL_TRUE, 0);
        SDL_SetColorKey(f.rightUpperCorner.get(), SDL_TRUE, 0);
        SDL_SetColorKey(f.rightLowerCorner.get(), SDL_TRUE, 0);
        SDL_SetColorKey(f.hborder.get(), SDL_TRUE, 0);
        SDL_SetColorKey(f.vborder.get(), SDL_TRUE, 0);
    }

    // House Logos
    harkonnenLogo = getSubPicture(FamePic.get(), 10, 137, 53, 54);
    atreidesLogo  = getSubPicture(FamePic.get(), 66, 137, 53, 54);
    ordosLogo     = getSubPicture(FamePic.get(), 122, 137, 53, 54);

    gameStatsBackground = copySurface(background.get());

    {
        auto FamePic2  = Scaler::defaultDoubleSurface(FamePic.get());
        auto pSurface  = getSubPicture(FamePic2.get(), 16, 160, 610, 74);
        SDL_Rect dest2 = calcDrawingRect(pSurface.get(), 16, 234);
        SDL_BlitSurface(pSurface.get(), nullptr, FamePic2.get(), &dest2);

        SDL_Rect dest3 = calcDrawingRect(pSurface.get(), 16, 234 + 74);
        SDL_BlitSurface(pSurface.get(), nullptr, FamePic2.get(), &dest3);

        SDL_Rect dest4 = calcAlignedDrawingRect(FamePic2.get(), gameStatsBackground.get());
        SDL_BlitSurface(FamePic2.get(), nullptr, gameStatsBackground.get(), &dest4);
    }

    messageBoxBorder = getSubPicture(ScreenPic.get(), 0, 17, 320, 22);

    if (pFileManager->exists("MISC." + _("LanguageFileExtension"))) {
        mentatHouseChoiceQuestionSurface = Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MISC." + _("LanguageFileExtension")).get()).get());
    } else {
        mentatHouseChoiceQuestionSurface = Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MISC.CPS").get()).get());
    }

    // create builder list upper cap
    builderListUpperCap = sdl2::surface_ptr {SDL_CreateRGBSurface(0, 112, 21, 8, 0, 0, 0, 0)};
    if (builderListUpperCap == nullptr) {
        THROW(std::runtime_error, "PictureFactory::PictureFactory: Cannot create new Picture!");
    }
    palette.applyToSurface(builderListUpperCap.get());
    SDL_FillRect(builderListUpperCap.get(), nullptr, PALCOLOR_TRANSPARENT);

    {
        auto builderListUpperCapLeft = getSubPicture(ChoamPic.get(), 64, 3, 42, 18);
        SDL_Rect dest5               = {0, 0, 42, 18};
        SDL_BlitSurface(builderListUpperCapLeft.get(), nullptr, builderListUpperCap.get(), &dest5);
    }
    {
        auto builderListUpperCapMiddle = getSubPicture(ChoamPic.get(), 69, 3, 38, 13);
        SDL_Rect dest6                 = {42, 0, 38, 13};
        SDL_BlitSurface(builderListUpperCapMiddle.get(), nullptr, builderListUpperCap.get(), &dest6);
    }
    {
        auto builderListUpperCapRight = getSubPicture(ChoamPic.get(), 69, 3, 48, 21);
        SDL_Rect dest7                = {64, 0, 48, 21};
        SDL_BlitSurface(builderListUpperCapRight.get(), nullptr, builderListUpperCap.get(), &dest7);
    }
    replaceColor(builderListUpperCap.get(), 30, 0);
    SDL_SetColorKey(builderListUpperCap.get(), SDL_TRUE, 0);

    // create builder list lower cap
    builderListLowerCap = sdl2::surface_ptr {SDL_CreateRGBSurface(0, 112, 17, 8, 0, 0, 0, 0)};
    if (builderListLowerCap == nullptr) {
        THROW(std::runtime_error, "PictureFactory::PictureFactory: Cannot create new Picture!");
    }

    palette.applyToSurface(builderListLowerCap.get());
    SDL_FillRect(builderListLowerCap.get(), nullptr, PALCOLOR_TRANSPARENT);

    {
        auto builderListLowerCapLeft = getSubPicture(ChoamPic.get(), 64, 149, 44, 17);
        SDL_Rect dest8               = {0, 0, 44, 17};
        SDL_BlitSurface(builderListLowerCapLeft.get(), nullptr, builderListLowerCap.get(), &dest8);
    }
    {
        auto builderListLowerCapMiddle = getSubPicture(ChoamPic.get(), 68, 152, 40, 14);
        SDL_Rect dest9                 = {44, 3, 40, 14};
        SDL_BlitSurface(builderListLowerCapMiddle.get(), nullptr, builderListLowerCap.get(), &dest9);
    }
    {
        auto builderListLowerCapRight = getSubPicture(ChoamPic.get(), 68, 149, 48, 17);
        SDL_Rect dest10               = {64, 0, 48, 17};
        SDL_BlitSurface(builderListLowerCapRight.get(), nullptr, builderListLowerCap.get(), &dest10);
    }

    replaceColor(builderListLowerCap.get(), 30, 0);
    SDL_SetColorKey(builderListLowerCap.get(), SDL_TRUE, 0);
}

PictureFactory::~PictureFactory() = default;

sdl2::surface_ptr PictureFactory::createTopBar() const {
    auto topBar          = getSubPicture(background.get(), 0, 0, settings.video.width - SIDEBARWIDTH, 32 + 12);
    const SDL_Rect dest1 = {0, 31, getWidth(topBar.get()), 12};
    SDL_FillRect(topBar.get(), &dest1, PALCOLOR_TRANSPARENT);

    SDL_Rect dest2 = calcDrawingRect(decorationBorder.hborder.get(), 0, 32);
    for (dest2.x = 0; dest2.x < topBar->w; dest2.x += decorationBorder.hborder.get()->w) {
        SDL_Rect tmpDest = dest2;
        SDL_BlitSurface(decorationBorder.hborder.get(), nullptr, topBar.get(), &tmpDest);
    }

    drawVLine(topBar.get(), topBar->w - 7, 32, topBar->h - 1, 96);

    SDL_Rect dest3 = {getWidth(topBar.get()) - 6, getHeight(topBar.get()) - 12, 12, 5};
    SDL_BlitSurface(decorationBorder.hspacer.get(), nullptr, topBar.get(), &dest3);

    drawVLine(topBar.get(), topBar->w - 1, 0, topBar->h - 1, 0);

    return topBar;
}

sdl2::surface_ptr PictureFactory::createSideBar(bool bEditor) const {
    auto sideBar         = getSubPicture(background.get(), 0, 0, SIDEBARWIDTH, settings.video.height);
    const SDL_Rect dest1 = {0, 0, 13, getHeight(sideBar.get())};
    SDL_FillRect(sideBar.get(), &dest1, PALCOLOR_TRANSPARENT);

    SDL_Rect dest2 = calcDrawingRect(decorationBorder.vborder.get(), 0, 0);
    for (dest2.y = 0; dest2.y < sideBar->h; dest2.y += decorationBorder.vborder.get()->h) {
        SDL_Rect tmpDest = dest2;
        SDL_BlitSurface(decorationBorder.vborder.get(), nullptr, sideBar.get(), &tmpDest);
    }

    SDL_Rect dest3 = calcDrawingRect(decorationBorder.vspacer.get(), 0, 30, HAlign::Left, VAlign::Bottom);
    SDL_BlitSurface(decorationBorder.vspacer.get(), nullptr, sideBar.get(), &dest3);

    drawHLine(sideBar.get(), 0, 32 - decorationBorder.vspacer.get()->h - 2, decorationBorder.vspacer.get()->w - 1, 96);
    drawHLine(sideBar.get(), 0, 31, decorationBorder.vspacer.get()->w - 1, 0);

    SDL_Rect dest4 = calcDrawingRect(decorationBorder.ball.get(), 0, 32);
    SDL_BlitSurface(decorationBorder.ball.get(), nullptr, sideBar.get(), &dest4);

    drawHLine(sideBar.get(), 0, 43, decorationBorder.vspacer.get()->w - 1, 0);
    SDL_Rect dest5 = calcDrawingRect(decorationBorder.vspacer.get(), 0, 44);
    SDL_BlitSurface(decorationBorder.vspacer.get(), nullptr, sideBar.get(), &dest5);
    drawHLine(sideBar.get(), 0, 44 + decorationBorder.vspacer.get()->h, decorationBorder.vspacer.get()->w - 1, 96);

    const SDL_Rect dest6 = {13, 0, getWidth(sideBar.get()) - 1, 132};
    SDL_FillRect(sideBar.get(), &dest6, PALCOLOR_TRANSPARENT);
    drawRect(sideBar.get(), 13, 1, sideBar->w - 2, 130, 115);

    SDL_Rect dest7 = calcDrawingRect(decorationBorder.vspacer.get(), 0, 130, HAlign::Left, VAlign::Bottom);
    SDL_BlitSurface(decorationBorder.vspacer.get(), nullptr, sideBar.get(), &dest7);

    drawHLine(sideBar.get(), 0, 132 - decorationBorder.vspacer.get()->h - 2, decorationBorder.vspacer.get()->w - 1, 96);
    drawHLine(sideBar.get(), 0, 131, decorationBorder.vspacer.get()->w - 1, 0);

    SDL_Rect dest8 = calcDrawingRect(decorationBorder.ball.get(), 0, 132);
    SDL_BlitSurface(decorationBorder.ball.get(), nullptr, sideBar.get(), &dest8);

    drawHLine(sideBar.get(), 0, 143, decorationBorder.vspacer.get()->w - 1, 0);
    SDL_Rect dest9 = calcDrawingRect(decorationBorder.vspacer.get(), 0, 144);
    SDL_BlitSurface(decorationBorder.vspacer.get(), nullptr, sideBar.get(), &dest9);
    drawHLine(sideBar.get(), 0, 144 + decorationBorder.vspacer.get()->h, decorationBorder.vspacer.get()->w - 1, 96);

    SDL_Rect dest10 = calcDrawingRect(decorationBorder.hspacer.get(), 13, 132);
    SDL_BlitSurface(decorationBorder.hspacer.get(), nullptr, sideBar.get(), &dest10);

    drawVLine(sideBar.get(), 18, 132, 132 + decorationBorder.hspacer.get()->h - 1, 96);
    drawHLine(sideBar.get(), 13, 132 + decorationBorder.hspacer.get()->h, sideBar->w - 1, 0);

    SDL_Rect dest11 = calcDrawingRect(decorationBorder.hborder.get(), 0, 132);
    for (dest11.x = 19; dest11.x < sideBar->w; dest11.x += decorationBorder.hborder.get()->w) {
        SDL_Rect tmpDest = dest11;
        SDL_BlitSurface(decorationBorder.hborder.get(), nullptr, sideBar.get(), &tmpDest);
    }

    if (bEditor) {
        SDL_Rect dest12 = calcDrawingRect(decorationBorder.vspacer.get(), 0, getHeight(sideBar.get()) - 32 - 14, HAlign::Left, VAlign::Bottom);
        SDL_BlitSurface(decorationBorder.vspacer.get(), nullptr, sideBar.get(), &dest12);

        drawHLine(sideBar.get(), 0, sideBar->h - 32 - 12 - decorationBorder.vspacer.get()->h - 2, decorationBorder.vspacer.get()->w - 1, 96);
        drawHLine(sideBar.get(), 0, sideBar->h - 32 - 12 - 1, decorationBorder.vspacer.get()->w - 1, 0);

        SDL_Rect dest13 = calcDrawingRect(decorationBorder.ball.get(), 0, getHeight(sideBar.get()) - 32 - 12);
        SDL_BlitSurface(decorationBorder.ball.get(), nullptr, sideBar.get(), &dest13);

        drawHLine(sideBar.get(), 0, sideBar->h - 32 - 1, decorationBorder.vspacer.get()->w - 1, 0);
        SDL_Rect dest14 = calcDrawingRect(decorationBorder.vspacer.get(), 0, getHeight(sideBar.get()) - 32);
        SDL_BlitSurface(decorationBorder.vspacer.get(), nullptr, sideBar.get(), &dest14);
        drawHLine(sideBar.get(), 0, sideBar->h - 32 + decorationBorder.vspacer.get()->h, decorationBorder.vspacer.get()->w - 1, 96);
    } else {
        SDL_Rect dest15 = calcDrawingRect(creditsBorder.get(), 46, 132);
        SDL_BlitSurface(creditsBorder.get(), nullptr, sideBar.get(), &dest15);
    }

    return sideBar;
}

sdl2::surface_ptr PictureFactory::createBottomBar() const {
    auto BottomBar       = getSubPicture(background.get(), 0, 0, settings.video.width - SIDEBARWIDTH, 32 + 12);
    const SDL_Rect dest1 = {0, 0, getWidth(BottomBar.get()), 13};
    SDL_FillRect(BottomBar.get(), &dest1, PALCOLOR_TRANSPARENT);

    SDL_Rect dest2 = calcDrawingRect(decorationBorder.hborder.get(), 0, 0);
    for (dest2.x = 0; dest2.x < BottomBar->w; dest2.x += decorationBorder.hborder.get()->w) {
        SDL_Rect tmpDest = dest2;
        SDL_BlitSurface(decorationBorder.hborder.get(), nullptr, BottomBar.get(), &tmpDest);
    }

    drawVLine(BottomBar.get(), BottomBar->w - 7, 0, 11, 96);

    SDL_Rect dest3 = {getWidth(BottomBar.get()) - 6, 0, 12, 5};
    SDL_BlitSurface(decorationBorder.hspacer.get(), nullptr, BottomBar.get(), &dest3);

    drawVLine(BottomBar.get(), BottomBar->w - 1, 0, BottomBar->h - 1, 0);

    return BottomBar;
}

sdl2::surface_ptr PictureFactory::createPlacingGrid(int size, int color) {
    sdl2::surface_ptr placingGrid {SDL_CreateRGBSurface(0, size, size, 8, 0, 0, 0, 0)};
    if (placingGrid == nullptr) {
        THROW(sdl_error, "Cannot create new surface: %s!", SDL_GetError());
    }

    palette.applyToSurface(placingGrid.get());
    sdl2::surface_lock lock {placingGrid.get()};

    for (auto y = 0; y < size; y++) {
        auto* const out = static_cast<uint8_t*>(placingGrid->pixels) + y * placingGrid->pitch;
        for (auto x = 0; x < size; x++) {
            if (x % 2 == y % 2) {
                out[x] = color;
            } else {
                out[x] = 0;
            }
        }
    }

    SDL_SetColorKey(placingGrid.get(), SDL_TRUE, 0);

    return placingGrid;
}

void PictureFactory::drawFrame(SDL_Surface* Pic, unsigned int DecorationType, SDL_Rect* dest) const {
    if (Pic == nullptr)
        return;

    if (DecorationType >= NUM_DECORATIONFRAMES)
        return;

    SDL_Rect tmp;
    if (dest == nullptr) {
        tmp.x = 0;
        tmp.y = 0;
        tmp.w = Pic->w;
        tmp.h = Pic->h;
        dest  = &tmp;
    }

    // corners
    SDL_Rect dest1 = calcDrawingRect(frame[DecorationType].leftUpperCorner.get(), dest->x, dest->y);
    SDL_BlitSurface(frame[DecorationType].leftUpperCorner.get(), nullptr, Pic, &dest1);

    SDL_Rect dest2 = calcDrawingRect(frame[DecorationType].rightUpperCorner.get(), dest->w - 1, dest->y, HAlign::Right, VAlign::Top);
    SDL_BlitSurface(frame[DecorationType].rightUpperCorner.get(), nullptr, Pic, &dest2);

    SDL_Rect dest3 = calcDrawingRect(frame[DecorationType].leftLowerCorner.get(), dest->x, dest->h - 1, HAlign::Left, VAlign::Bottom);
    SDL_BlitSurface(frame[DecorationType].leftLowerCorner.get(), nullptr, Pic, &dest3);

    SDL_Rect dest4 = calcDrawingRect(frame[DecorationType].rightLowerCorner.get(), dest->w - 1, dest->h - 1, HAlign::Right, VAlign::Bottom);
    SDL_BlitSurface(frame[DecorationType].rightLowerCorner.get(), nullptr, Pic, &dest4);

    // hborders
    SDL_Rect dest5 = calcDrawingRect(frame[DecorationType].hborder.get(), dest->x, dest->y);
    for (dest5.x = frame[DecorationType].leftUpperCorner.get()->w + dest->x;
         dest5.x <= dest->w - frame[DecorationType].rightUpperCorner.get()->w - 1;
         dest5.x += frame[DecorationType].hborder.get()->w) {
        SDL_Rect tmpDest = dest5;
        SDL_BlitSurface(frame[DecorationType].hborder.get(), nullptr, Pic, &tmpDest);
    }

    SDL_Rect dest6 = calcDrawingRect(frame[DecorationType].hborder.get(), dest->x, dest->h - 1, HAlign::Left, VAlign::Bottom);
    for (dest6.x = frame[DecorationType].leftLowerCorner.get()->w + dest->x;
         dest6.x <= dest->w - frame[DecorationType].rightLowerCorner.get()->w - 1;
         dest6.x += frame[DecorationType].hborder.get()->w) {
        SDL_Rect tmpDest = dest6;
        SDL_BlitSurface(frame[DecorationType].hborder.get(), nullptr, Pic, &tmpDest);
    }

    // vborders
    SDL_Rect dest7 = calcDrawingRect(frame[DecorationType].vborder.get(), dest->x, dest->y);
    for (dest7.y = frame[DecorationType].leftUpperCorner.get()->h + dest->y;
         dest7.y <= dest->h - frame[DecorationType].leftLowerCorner.get()->h - 1;
         dest7.y += frame[DecorationType].vborder.get()->h) {
        SDL_Rect tmpDest = dest7;
        SDL_BlitSurface(frame[DecorationType].vborder.get(), nullptr, Pic, &tmpDest);
    }

    SDL_Rect dest8 = calcDrawingRect(frame[DecorationType].vborder.get(), dest->w - 1, dest->y, HAlign::Right, VAlign::Top);
    for (dest8.y = frame[DecorationType].rightUpperCorner.get()->h + dest->y;
         dest8.y <= dest->h - frame[DecorationType].rightLowerCorner.get()->h - 1;
         dest8.y += frame[DecorationType].vborder.get()->h) {
        SDL_Rect tmpDest = dest8;
        SDL_BlitSurface(frame[DecorationType].vborder.get(), nullptr, Pic, &tmpDest);
    }
}

sdl2::surface_ptr PictureFactory::createFrame(unsigned int DecorationType, int width, int height, bool UseBackground) {
    sdl2::surface_ptr Pic;
    if (UseBackground) {
        Pic = getSubPicture(background.get(), 0, 0, width, height);
    } else {
        Pic = sdl2::surface_ptr {SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0)};
        if (Pic == nullptr) {
            THROW(sdl_error, "Cannot create new surface: %s!", SDL_GetError());
        }
        palette.applyToSurface(Pic.get());
        SDL_SetColorKey(Pic.get(), SDL_TRUE, 0);
    }

    drawFrame(Pic.get(), DecorationType);

    return Pic;
}

sdl2::surface_ptr PictureFactory::createBackground() const {
    return copySurface(background.get());
}

sdl2::surface_ptr PictureFactory::createMainBackground() const {
    auto Pic = copySurface(background.get());

    SDL_Rect dest0 = {3, 3, getWidth(Pic.get()) - 3, getHeight(Pic.get()) - 3};
    drawFrame(Pic.get(), DecorationFrame2, &dest0);

    SDL_Rect dest1 = calcDrawingRect(harkonnenLogo.get(), 11, 11);
    SDL_BlitSurface(harkonnenLogo.get(), nullptr, Pic.get(), &dest1);

    SDL_Rect dest2 = calcDrawingRect(atreidesLogo.get(), getWidth(Pic.get()) - 11, 11, HAlign::Right, VAlign::Top);
    SDL_BlitSurface(atreidesLogo.get(), nullptr, Pic.get(), &dest2);

    SDL_Rect dest3 = calcDrawingRect(ordosLogo.get(), 11, getHeight(Pic.get()) - 11, HAlign::Left, VAlign::Bottom);
    SDL_BlitSurface(ordosLogo.get(), nullptr, Pic.get(), &dest3);

    const sdl2::surface_ptr Version {getSubPicture(background.get(), 0, 0, 75, 32)};

    sdl2::surface_ptr VersionText {pFontManager->createSurfaceWithText(std::string(VERSION), PALCOLOR_BLACK, 14)};

    SDL_Rect dest4 = calcDrawingRect(VersionText.get(), getWidth(Version.get()) / 2, getHeight(Version.get()) / 2 + 2, HAlign::Center, VAlign::Center);
    SDL_BlitSurface(VersionText.get(), nullptr, Version.get(), &dest4);

    VersionText.reset();

    drawFrame(Version.get(), SimpleFrame);

    SDL_Rect dest5 = calcDrawingRect(Version.get(), getWidth(Pic.get()) - 11, getHeight(Pic.get()) - 11, HAlign::Right, VAlign::Bottom);
    SDL_BlitSurface(Version.get(), nullptr, Pic.get(), &dest5);

    return Pic;
}

sdl2::surface_ptr PictureFactory::createGameStatsBackground(HOUSETYPE House) const {
    auto pSurface = copySurface(gameStatsBackground.get());

    sdl2::surface_ptr pLogo;
    switch (House) {
        case HOUSETYPE::HOUSE_HARKONNEN:
        case HOUSETYPE::HOUSE_SARDAUKAR: {
            pLogo = copySurface(harkonnenLogo.get());
        } break;

        case HOUSETYPE::HOUSE_ATREIDES:
        case HOUSETYPE::HOUSE_FREMEN: {
            pLogo = copySurface(atreidesLogo.get());
        } break;

        case HOUSETYPE::HOUSE_ORDOS:
        case HOUSETYPE::HOUSE_MERCENARY: {
            pLogo = copySurface(ordosLogo.get());
        } break;

        default:
            THROW(std::invalid_argument, "PictureFactory::createGameStatsBackground(): Unknown house %d!", static_cast<int>(House));
    }

    pLogo = Scaler::defaultDoubleSurface(pLogo.get());

    auto dest1 = calcDrawingRect(pLogo.get(), getWidth(gameStatsBackground.get()) / 2 - 320 + 2, getHeight(gameStatsBackground.get()) / 2 - 200 + 16);
    SDL_BlitSurface(pLogo.get(), nullptr, pSurface.get(), &dest1);
    auto dest2 = calcDrawingRect(pLogo.get(), getWidth(gameStatsBackground.get()) / 2 + 320 - 3, getHeight(gameStatsBackground.get()) / 2 - 200 + 16, HAlign::Right, VAlign::Top);
    SDL_BlitSurface(pLogo.get(), nullptr, pSurface.get(), &dest2);

    return pSurface;
}

sdl2::surface_ptr PictureFactory::createMenu(int x, int y) const {
    auto Pic = getSubPicture(background.get(), 0, 0, x, y);

    SDL_Rect dest1 = {0, 0, getWidth(Pic.get()), 27};

    SDL_FillRect(Pic.get(), &dest1, PALCOLOR_GREY);

    drawFrame(Pic.get(), SimpleFrame, &dest1);

    SDL_Rect dest2 = calcDrawingRect(Pic.get(), 0, dest1.h);
    drawFrame(Pic.get(), DecorationFrame1, &dest2);

    return Pic;
}

sdl2::surface_ptr PictureFactory::createMenu(SDL_Surface* CaptionPic, int y) const {
    if (CaptionPic == nullptr)
        return nullptr;

    auto Pic = getSubPicture(background.get(), 0, 0, CaptionPic->w, y);

    SDL_Rect dest1 = calcDrawingRect(CaptionPic, 0, 0);
    SDL_BlitSurface(CaptionPic, nullptr, Pic.get(), &dest1);

    drawFrame(Pic.get(), SimpleFrame, &dest1);

    SDL_Rect dest2 = calcDrawingRect(Pic.get(), 0, dest1.h);
    drawFrame(Pic.get(), DecorationFrame1, &dest2);

    return Pic;
}

sdl2::surface_ptr PictureFactory::createOptionsMenu() {
    auto tmp = LoadPNG_RW(pFileManager->openFile("UI_OptionsMenu.png").get());
    if (tmp == nullptr) {
        THROW(std::runtime_error, "Cannot load 'UI_OptionsMenu.png'!");
    }
    SDL_SetColorKey(tmp.get(), SDL_TRUE, 0);

    auto Pic = getSubPicture(background.get(), 0, 0, tmp->w, tmp->h);
    SDL_BlitSurface(tmp.get(), nullptr, Pic.get(), nullptr);

    tmp.reset();

    SDL_Rect dest1 = {0, 0, getWidth(Pic.get()), 27};
    drawFrame(Pic.get(), SimpleFrame, &dest1);

    SDL_Rect dest2 = calcDrawingRect(Pic.get(), 0, dest1.h);
    drawFrame(Pic.get(), DecorationFrame1, &dest2);

    return Pic;
}

sdl2::surface_ptr PictureFactory::createMessageBoxBorder() const {
    return copySurface(messageBoxBorder.get());
}

sdl2::surface_ptr PictureFactory::createHouseSelect(SDL_Surface* HouseChoice) const {
    auto Pic = copySurface(HouseChoice);

    const SDL_Rect dest = {0, 50, getWidth(Pic.get()), getHeight(Pic.get()) - 50};
    SDL_FillRect(Pic.get(), &dest, PALCOLOR_BLACK);

    drawFrame(Pic.get(), SimpleFrame, nullptr);

    return Pic;
}

sdl2::surface_ptr PictureFactory::createGreyHouseChoice(SDL_Surface* HouseChoice) {
    static const unsigned char index2greyindex[] = {
        0, 0, 0, 13, 233, 127, 0, 131, 0, 0, 0, 0, 0, 13, 14, 15,
        15, 127, 127, 14, 14, 14, 14, 130, 24, 131, 131, 13, 13, 29, 30, 31,
        0, 128, 128, 14, 14, 14, 14, 130, 130, 24, 24, 14, 13, 13, 0, 29,
        0, 0, 30, 0, 0, 183, 0, 0, 0, 0, 0, 0, 14, 30, 30, 30,
        126, 0, 0, 126, 128, 0, 0, 14, 14, 14, 0, 14, 14, 0, 0, 0,
        14, 14, 0, 0, 0, 0, 0, 14, 0, 0, 130, 13, 131, 13, 13, 29,
        30, 30, 183, 175, 175, 0, 0, 0, 0, 0, 0, 0, 14, 233, 14, 14,
        14, 14, 14, 130, 24, 0, 0, 0, 131, 0, 122, 0, 24, 0, 0, 0,
        0, 14, 130, 131, 29, 133, 134, 127, 233, 14, 14, 24, 131, 13, 29, 183,
        30, 30, 183, 183, 175, 175, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 13, 29, 183, 175, 0, 0, 30, 0, 0, 13, 0, 0, 30, 174, 175,
        14, 24, 131, 13, 30, 183, 175, 122, 0, 0, 0, 0, 0, 0, 0, 0,
        14, 24, 131, 13, 30, 122, 175, 0, 0, 0, 0, 13, 0, 0, 0, 0,
        14, 24, 131, 13, 30, 122, 175, 24, 14, 0, 0, 29, 0, 0, 0, 0,
        14, 24, 131, 13, 30, 122, 175, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 13, 0, 30, 30, 183, 250, 250, 0, 0, 0, 0, 0};

    auto pic = copySurface(HouseChoice);

    for (auto y = 0; y < pic->h; ++y) {
        unsigned char* const RESTRICT p = static_cast<unsigned char*>(pic->pixels) + y * pic->pitch;
        for (auto x = 0; x < pic->w; ++x) {
            const auto inputIndex  = p[x];
            const auto outputIndex = index2greyindex[inputIndex];
            *(p + x)               = outputIndex;
        }
    }

    return pic;
}

sdl2::surface_ptr PictureFactory::createMapChoiceScreen(HOUSETYPE House) const {
    sdl2::surface_ptr pMapChoiceScreen {LoadCPS_RW(pFileManager->openFile("MAPMACH.CPS").get())};
    if (pMapChoiceScreen == nullptr) {
        THROW(std::runtime_error, "Cannot load 'MAPMACH.CPS'!");
    }

    auto LeftLogo  = calcDrawingRect(harkonnenLogo.get(), 2, 145);
    auto RightLogo = calcDrawingRect(harkonnenLogo.get(), 266, 145);

    switch (House) {
        case HOUSETYPE::HOUSE_HARKONNEN:
        case HOUSETYPE::HOUSE_SARDAUKAR: {
            SDL_BlitSurface(harkonnenLogo.get(), nullptr, pMapChoiceScreen.get(), &LeftLogo);
            SDL_BlitSurface(harkonnenLogo.get(), nullptr, pMapChoiceScreen.get(), &RightLogo);
        } break;

        case HOUSETYPE::HOUSE_ATREIDES:
        case HOUSETYPE::HOUSE_FREMEN: {
            SDL_BlitSurface(atreidesLogo.get(), nullptr, pMapChoiceScreen.get(), &LeftLogo);
            SDL_BlitSurface(atreidesLogo.get(), nullptr, pMapChoiceScreen.get(), &RightLogo);
        } break;

        case HOUSETYPE::HOUSE_ORDOS:
        case HOUSETYPE::HOUSE_MERCENARY: {
            SDL_BlitSurface(ordosLogo.get(), nullptr, pMapChoiceScreen.get(), &LeftLogo);
            SDL_BlitSurface(ordosLogo.get(), nullptr, pMapChoiceScreen.get(), &RightLogo);
        } break;

        default: {

        } break;
    }

    if (settings.general.language == "de") {
        auto tmp      = getSubPicture(pMapChoiceScreen.get(), 8, 120, 303, 23);
        tmp           = copySurface(tmp.get()); // Workaround: SDL2 leaks memory when blitting from A to B and afterwards from B to A
        SDL_Rect dest = {8, 0, 303, 23};
        SDL_BlitSurface(tmp.get(), nullptr, pMapChoiceScreen.get(), &dest);
    } else if (settings.general.language == "fr") {
        auto tmp      = getSubPicture(pMapChoiceScreen.get(), 8, 96, 303, 23);
        tmp           = copySurface(tmp.get()); // Workaround: SDL2 leaks memory when blitting from A to B and afterwards from B to A
        SDL_Rect dest = {8, 0, 303, 23};
        SDL_BlitSurface(tmp.get(), nullptr, pMapChoiceScreen.get(), &dest);
    } else {
        ; // Nothing to do (use English)
    }

    // clear everything in the middle
    const SDL_Rect clearRect = {8, 24, 304, 119};
    SDL_FillRect(pMapChoiceScreen.get(), &clearRect, PALCOLOR_TRANSPARENT);

    pMapChoiceScreen          = Scaler::defaultDoubleSurface(mapSurfaceColorRange(pMapChoiceScreen.get(), PALCOLOR_HARKONNEN, houseToPaletteIndex[static_cast<int>(House)]).get());
    auto pFullMapChoiceScreen = copySurface(background.get());

    SDL_Rect dest = calcAlignedDrawingRect(pMapChoiceScreen.get(), pFullMapChoiceScreen.get());
    SDL_BlitSurface(pMapChoiceScreen.get(), nullptr, pFullMapChoiceScreen.get(), &dest);

    return pFullMapChoiceScreen;
}

sdl2::surface_ptr PictureFactory::createMentatHouseChoiceQuestion(HOUSETYPE House, Palette& benePalette) const {
    sdl2::surface_ptr pSurface {SDL_CreateRGBSurface(0, 416 + 208, 48, 8, 0, 0, 0, 0)};
    if (pSurface == nullptr) {
        THROW(sdl_error, "Cannot create new surface: %s!", SDL_GetError());
    }

    benePalette.applyToSurface(pSurface.get());
    SDL_SetColorKey(pSurface.get(), SDL_TRUE, 0);

    const auto pQuestionPart1 = getSubPicture(mentatHouseChoiceQuestionSurface.get(), 0, 0, 416, 48);

    sdl2::surface_ptr pQuestionPart2 = nullptr;

    // clang-format off
    switch(House) {
        case HOUSETYPE::HOUSE_HARKONNEN:   pQuestionPart2 = getSubPicture(mentatHouseChoiceQuestionSurface.get(),0, 48, 208, 48);   break;
        case HOUSETYPE::HOUSE_ATREIDES:    pQuestionPart2 = getSubPicture(mentatHouseChoiceQuestionSurface.get(),0, 96, 208, 48);   break;
        case HOUSETYPE::HOUSE_ORDOS:       pQuestionPart2 = getSubPicture(mentatHouseChoiceQuestionSurface.get(),0, 144, 208, 48);  break;
        case HOUSETYPE::HOUSE_FREMEN:      pQuestionPart2 = Scaler::defaultDoubleSurface(LoadPNG_RW(pFileManager->openFile("Fremen.png").get()).get());      break;
        case HOUSETYPE::HOUSE_SARDAUKAR:   pQuestionPart2 = Scaler::defaultDoubleSurface(LoadPNG_RW(pFileManager->openFile("Sardaukar.png").get()).get());   break;
        case HOUSETYPE::HOUSE_MERCENARY:   pQuestionPart2 = Scaler::defaultDoubleSurface(LoadPNG_RW(pFileManager->openFile("Mercenary.png").get()).get());   break;
        default:    break;
    }
    // clang-format on

    SDL_SetColorKey(pQuestionPart2.get(), SDL_TRUE, 0);

    SDL_Rect dest1 = calcDrawingRect(pQuestionPart1.get(), 0, 0);
    SDL_BlitSurface(pQuestionPart1.get(), nullptr, pSurface.get(), &dest1);

    SDL_Rect dest2 = calcDrawingRect(pQuestionPart2.get(), getWidth(pQuestionPart1.get()), 0);
    SDL_BlitSurface(pQuestionPart2.get(), nullptr, pSurface.get(), &dest2);

    return pSurface;
}

sdl2::surface_ptr PictureFactory::createBuilderListUpperCap() const {
    return copySurface(builderListUpperCap.get());
}

sdl2::surface_ptr PictureFactory::createBuilderListLowerCap() const {
    return copySurface(builderListLowerCap.get());
}

sdl2::surface_ptr PictureFactory::createHeraldFre(SDL_Surface* heraldHark) {
    auto pRedReplaced = mapSurfaceColorRange(heraldHark, PALCOLOR_HARKONNEN, PALCOLOR_FREMEN);

    const auto pBlueReplaced = mapSurfaceColorRange(pRedReplaced.get(), PALCOLOR_ATREIDES, PALCOLOR_FREMEN + 1);
    pRedReplaced.reset();

    replaceColor(pBlueReplaced.get(), 170, 194);
    replaceColor(pBlueReplaced.get(), 173, 195);

    auto pTmp1     = scaleSurface(Wsafile(pFileManager->openFile("WORM.WSA").get()).getPicture(0).get(), 0.5);
    auto pSandworm = getSubPicture(pTmp1.get(), 40 - 18, 6 - 12, 83, 91);
    pTmp1.reset();

    auto pMask = LoadPNG_RW(pFileManager->openFile("HeraldFreMask.png").get());
    SDL_SetColorKey(pMask.get(), SDL_TRUE, 0);

    SDL_BlitSurface(pMask.get(), nullptr, pBlueReplaced.get(), nullptr);
    pMask.reset();

    SDL_SetColorKey(pBlueReplaced.get(), SDL_TRUE, 223);

    SDL_BlitSurface(pBlueReplaced.get(), nullptr, pSandworm.get(), nullptr);

    return pSandworm;
}

sdl2::surface_ptr PictureFactory::createHeraldSard(SDL_Surface* heraldOrd, SDL_Surface* heraldAtre) {
    const auto pGreenReplaced = mapSurfaceColorRange(heraldOrd, PALCOLOR_ORDOS, PALCOLOR_SARDAUKAR - 1);

    replaceColor(pGreenReplaced.get(), 3, 209);

    auto pCurtain = mapSurfaceColorRange(heraldAtre, PALCOLOR_ATREIDES, PALCOLOR_SARDAUKAR);
    pCurtain      = getSubPicture(pCurtain.get(), 7, 7, 69, 49);

    auto pFrameAndCurtain = combinePictures(pGreenReplaced.get(), pCurtain.get(), 7, 7);

    const auto pMask = sdl2::surface_ptr {LoadPNG_RW(pFileManager->openFile("HeraldSardMask.png").get())};
    SDL_SetColorKey(pMask.get(), SDL_TRUE, 0);

    SDL_BlitSurface(pMask.get(), nullptr, pFrameAndCurtain.get(), nullptr);

    return pFrameAndCurtain;
}

sdl2::surface_ptr PictureFactory::createHeraldMerc(SDL_Surface* heraldAtre, SDL_Surface* heraldOrd) {
    auto pBlueReplaced = mapSurfaceColorRange(heraldAtre, PALCOLOR_ATREIDES, PALCOLOR_MERCENARY);

    const auto pRedReplaced = mapSurfaceColorRange(pBlueReplaced.get(), PALCOLOR_HARKONNEN, PALCOLOR_ATREIDES);
    pBlueReplaced.reset();

    auto pCurtain = mapSurfaceColorRange(heraldOrd, PALCOLOR_ORDOS, PALCOLOR_MERCENARY);
    pCurtain      = getSubPicture(pCurtain.get(), 7, 7, 69, 49);

    const auto pFrameAndCurtain = combinePictures(pRedReplaced.get(), pCurtain.get(), 7, 7);

    auto pSoldier = Wsafile(pFileManager->openFile("INFANTRY.WSA").get()).getPicture(0);
    pSoldier      = getSubPicture(pSoldier.get(), 49, 17, 83, 91);

    auto pMask = LoadPNG_RW(pFileManager->openFile("HeraldMercMask.png").get());
    SDL_SetColorKey(pMask.get(), SDL_TRUE, 0);

    SDL_BlitSurface(pMask.get(), nullptr, pFrameAndCurtain.get(), nullptr);
    pMask.reset();

    SDL_SetColorKey(pFrameAndCurtain.get(), SDL_TRUE, 223);

    SDL_BlitSurface(pFrameAndCurtain.get(), nullptr, pSoldier.get(), nullptr);

    return pSoldier;
}

std::unique_ptr<Animation> PictureFactory::createFremenPlanet(SDL_Surface* heraldFre) {
    auto newAnimation = std::make_unique<Animation>();

    auto newFrame = sdl2::surface_ptr {LoadCPS_RW(pFileManager->openFile("BIGPLAN.CPS").get())};
    newFrame      = getSubPicture(newFrame.get(), -68, -34, 368, 224);

    const SDL_Rect src = {0, 0, getWidth(heraldFre) - 2, 126};
    SDL_Rect dest      = {12, 66, getWidth(heraldFre) - 2, getHeight(heraldFre)};
    SDL_BlitSurface(heraldFre, &src, newFrame.get(), &dest);

    drawRect(newFrame.get(), 0, 0, newFrame->w - 1, newFrame->h - 1, PALCOLOR_WHITE);

    newAnimation->addFrame(std::move(newFrame));

    return newAnimation;
}

std::unique_ptr<Animation> PictureFactory::createSardaukarPlanet(Animation* ordosPlanetAnimation, SDL_Surface* heraldSard) {

    const sdl2::surface_ptr maskSurface {Scaler::defaultDoubleSurface(LoadPNG_RW(pFileManager->openFile("PlanetMask.png").get()).get())};
    SDL_SetColorKey(maskSurface.get(), SDL_TRUE, 0);

    auto newAnimation = std::make_unique<Animation>();

    uint8_t colorMap[256];
    for (int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[15]  = 165;
    colorMap[154] = 13;
    colorMap[155] = 29;
    colorMap[156] = 30;
    colorMap[157] = 31;
    colorMap[158] = 164;
    colorMap[159] = 165;
    colorMap[160] = 24;
    colorMap[161] = 22;
    colorMap[162] = 13;
    colorMap[163] = 29;
    colorMap[164] = 31;

    for (const sdl2::surface_ptr& pSurface : ordosPlanetAnimation->getFrames()) {
        sdl2::surface_ptr newFrame = copySurface(pSurface.get());

        mapColor(newFrame.get(), colorMap);

        sdl2::surface_ptr newFrameWithoutPlanet = copySurface(pSurface.get());

        SDL_BlitSurface(maskSurface.get(), nullptr, newFrameWithoutPlanet.get(), nullptr);
        SDL_SetColorKey(newFrameWithoutPlanet.get(), SDL_TRUE, 223);
        SDL_BlitSurface(newFrameWithoutPlanet.get(), nullptr, newFrame.get(), nullptr);

        SDL_Rect src  = {0, 0, getWidth(heraldSard), 126};
        SDL_Rect dest = calcDrawingRect(heraldSard, 12, 66);
        SDL_BlitSurface(heraldSard, &src, newFrame.get(), &dest);

        newAnimation->addFrame(std::move(newFrame));
    }

    return newAnimation;
}

std::unique_ptr<Animation> PictureFactory::createMercenaryPlanet(Animation* atreidesPlanetAnimation, SDL_Surface* heraldMerc) {

    auto newAnimation = std::make_unique<Animation>();

    uint8_t colorMap[256];
    for (int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[3]   = 93;
    colorMap[4]   = 90;
    colorMap[68]  = 87;
    colorMap[69]  = 88;
    colorMap[70]  = 89;
    colorMap[71]  = 90;
    colorMap[72]  = 91;
    colorMap[73]  = 92;
    colorMap[74]  = 93;
    colorMap[75]  = 94;
    colorMap[76]  = 95;
    colorMap[176] = 91;
    colorMap[177] = 92;
    colorMap[178] = 94;
    colorMap[179] = 95;

    for (const sdl2::surface_ptr& pSurface : atreidesPlanetAnimation->getFrames()) {
        sdl2::surface_ptr newFrame = copySurface(pSurface.get());

        mapColor(newFrame.get(), colorMap);

        SDL_Rect src  = {0, 0, getWidth(heraldMerc), 126};
        SDL_Rect dest = calcDrawingRect(heraldMerc, 12, 66);
        SDL_BlitSurface(heraldMerc, &src, newFrame.get(), &dest);

        newAnimation->addFrame(std::move(newFrame));
    }

    return newAnimation;
}

sdl2::surface_ptr PictureFactory::mapMentatSurfaceToMercenary(SDL_Surface* ordosMentat) {
    auto mappedSurface = mapSurfaceColorRange(ordosMentat, PALCOLOR_ORDOS, PALCOLOR_MERCENARY);

    uint8_t colorMap[256];
    for (int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[186] = 245;
    colorMap[187] = 250;

    mapColor(mappedSurface.get(), colorMap);

    return mappedSurface;
}

std::unique_ptr<Animation> PictureFactory::mapMentatAnimationToFremen(Animation* fremenAnimation) {
    auto newAnimation = std::make_unique<Animation>();

    for (const sdl2::surface_ptr& pSurface : fremenAnimation->getFrames()) {
        newAnimation->addFrame(mapMentatSurfaceToFremen(pSurface.get()));
    }

    newAnimation->setFrameDurationTime(fremenAnimation->getFrameDurationTime());
    newAnimation->setNumLoops(fremenAnimation->getLoopsLeft());

    return newAnimation;
}

sdl2::surface_ptr PictureFactory::mapMentatSurfaceToSardaukar(SDL_Surface* harkonnenMentat) {
    auto mappedSurface = mapSurfaceColorRange(harkonnenMentat, PALCOLOR_HARKONNEN, PALCOLOR_SARDAUKAR);

    uint8_t colorMap[256];
    for (int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[54]  = 212;
    colorMap[56]  = 212;
    colorMap[57]  = 213;
    colorMap[58]  = 213;
    colorMap[121] = 211;
    colorMap[199] = 210;
    colorMap[200] = 211;
    colorMap[201] = 211;
    colorMap[202] = 213;

    mapColor(mappedSurface.get(), colorMap);

    return mappedSurface;
}

std::unique_ptr<Animation> PictureFactory::mapMentatAnimationToSardaukar(Animation* harkonnenAnimation) {
    auto newAnimation = std::make_unique<Animation>();

    for (const sdl2::surface_ptr& pSurface : harkonnenAnimation->getFrames()) {
        newAnimation->addFrame(mapMentatSurfaceToSardaukar(pSurface.get()));
    }

    newAnimation->setFrameDurationTime(harkonnenAnimation->getFrameDurationTime());
    newAnimation->setNumLoops(harkonnenAnimation->getLoopsLeft());

    return newAnimation;
}

std::unique_ptr<Animation> PictureFactory::mapMentatAnimationToMercenary(Animation* ordosAnimation) {
    auto newAnimation = std::make_unique<Animation>();

    for (const sdl2::surface_ptr& pSurface : ordosAnimation->getFrames()) {
        newAnimation->addFrame(mapMentatSurfaceToMercenary(pSurface.get()));
    }

    newAnimation->setFrameDurationTime(ordosAnimation->getFrameDurationTime());
    newAnimation->setNumLoops(ordosAnimation->getLoopsLeft());

    return newAnimation;
}

sdl2::surface_ptr PictureFactory::mapMentatSurfaceToFremen(SDL_Surface* fremenMentat) {
    sdl2::surface_ptr mappedSurface {mapSurfaceColorRange(fremenMentat, PALCOLOR_ATREIDES, PALCOLOR_FREMEN)};

    uint8_t colorMap[256];
    for (int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[179] = 12;
    colorMap[180] = 12;
    colorMap[181] = 12;
    colorMap[182] = 12;

    mapColor(mappedSurface.get(), colorMap);

    return mappedSurface;
}
