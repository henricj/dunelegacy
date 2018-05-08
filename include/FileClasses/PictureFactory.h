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

#ifndef PICTUREFACTORY_H
#define PICTUREFACTORY_H

#include <FileClasses/Animation.h>
#include <FileClasses/Palette.h>
#include <misc/SDL2pp.h>

#include <memory>

class PictureFactory {
public:
    PictureFactory();
    ~PictureFactory();

    PictureFactory(const PictureFactory &) = delete;
    PictureFactory(PictureFactory &&) = delete;
    PictureFactory& operator=(const PictureFactory &) = delete;
    PictureFactory& operator=(PictureFactory &&) = delete;

    sdl2::surface_ptr createTopBar() const;
    sdl2::surface_ptr createSideBar(bool bEditor) const;
    sdl2::surface_ptr createBottomBar() const;
    sdl2::surface_ptr createPlacingGrid(int size, int color) const;
    void drawFrame(SDL_Surface* Pic, unsigned int DecorationType, SDL_Rect* dest=nullptr) const;
    sdl2::surface_ptr createBackground() const;
    sdl2::surface_ptr createMainBackground() const;
    sdl2::surface_ptr createGameStatsBackground(int House) const;
    sdl2::surface_ptr createFrame(unsigned int DecorationType,int width, int height,bool UseBackground) const;
    sdl2::surface_ptr createMenu(int x,int y) const;
    sdl2::surface_ptr createMenu(SDL_Surface* CaptionPic,int y) const;
    sdl2::surface_ptr createOptionsMenu() const;
    sdl2::surface_ptr createMessageBoxBorder() const;
    sdl2::surface_ptr createHouseSelect(SDL_Surface* HouseChoice) const;
    sdl2::surface_ptr createGreyHouseChoice(SDL_Surface* HouseChoice) const;
    sdl2::surface_ptr createMapChoiceScreen(int House) const;
    sdl2::surface_ptr createMentatHouseChoiceQuestion(int House, Palette& benePalette) const;
    sdl2::surface_ptr createBuilderListUpperCap() const;
    sdl2::surface_ptr createBuilderListLowerCap() const;

    sdl2::surface_ptr createHeraldFre(SDL_Surface* heraldHark) const;
    sdl2::surface_ptr createHeraldSard(SDL_Surface* heraldOrd, SDL_Surface* heraldAtre) const;
    sdl2::surface_ptr createHeraldMerc(SDL_Surface* heraldAtre, SDL_Surface* heraldOrd) const;

    static std::unique_ptr<Animation> createFremenPlanet(SDL_Surface* heraldFre);
    static std::unique_ptr<Animation> createSardaukarPlanet(Animation* ordosPlanetAnimation, SDL_Surface* heraldSard);
    static std::unique_ptr<Animation> createMercenaryPlanet(Animation* atreidesPlanetAnimation, SDL_Surface* heraldMerc);

    static sdl2::surface_ptr mapMentatSurfaceToFremen(SDL_Surface* atreidesMentat);
    static std::unique_ptr<Animation> mapMentatAnimationToFremen(Animation* atreidesAnimation);
    static sdl2::surface_ptr mapMentatSurfaceToSardaukar(SDL_Surface* harkonnenMentat);
    static std::unique_ptr<Animation> mapMentatAnimationToSardaukar(Animation* harkonnenAnimation);
    static sdl2::surface_ptr mapMentatSurfaceToMercenary(SDL_Surface* ordosMentat);
    static std::unique_ptr<Animation> mapMentatAnimationToMercenary(Animation* ordosAnimation);

    typedef enum {
        SimpleFrame,
        DecorationFrame1,
        DecorationFrame2,
        NUM_DECORATIONFRAMES
    } DecorationFrame;


private:
    struct DecorationBorderType {
        sdl2::surface_ptr ball;
        sdl2::surface_ptr hspacer;
        sdl2::surface_ptr vspacer;
        sdl2::surface_ptr hborder;
        sdl2::surface_ptr vborder;
    } decorationBorder;

    struct BorderStyle {
        sdl2::surface_ptr leftUpperCorner;
        sdl2::surface_ptr rightUpperCorner;
        sdl2::surface_ptr leftLowerCorner;
        sdl2::surface_ptr rightLowerCorner;
        sdl2::surface_ptr hborder;
        sdl2::surface_ptr vborder;
    } frame[NUM_DECORATIONFRAMES];

    sdl2::surface_ptr background;
    sdl2::surface_ptr gameStatsBackground;
    sdl2::surface_ptr creditsBorder;

    sdl2::surface_ptr harkonnenLogo;
    sdl2::surface_ptr atreidesLogo;
    sdl2::surface_ptr ordosLogo;

    sdl2::surface_ptr messageBoxBorder;

    sdl2::surface_ptr mentatHouseChoiceQuestionSurface;

    sdl2::surface_ptr builderListUpperCap;
    sdl2::surface_ptr builderListLowerCap;

};

#endif // PICTUREFACTORY_H
