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

#include <SDL.h>

#include <memory>

class PictureFactory {
public:
	PictureFactory();
	~PictureFactory();

	SDL_Surface* createTopBar();
	SDL_Surface* createSideBar(bool bEditor);
	SDL_Surface* createBottomBar();
	SDL_Surface* createPlacingGrid(int size, int color);
	void drawFrame(SDL_Surface* Pic, unsigned int DecorationType, SDL_Rect* dest=NULL);
	SDL_Surface* createBackground();
	SDL_Surface* createMainBackground();
	SDL_Surface* createGameStatsBackground(int House);
	SDL_Surface* createFrame(unsigned int DecorationType,int width, int height,bool UseBackground);
	SDL_Surface* createMenu(int x,int y);
	SDL_Surface* createMenu(SDL_Surface* CaptionPic,int y);
	SDL_Surface* createOptionsMenu();
	SDL_Surface* createMessageBoxBorder();
	SDL_Surface* createHouseSelect(SDL_Surface* HouseChoice);
	SDL_Surface* createGreyHouseChoice(SDL_Surface* HouseChoice);
	SDL_Surface* createMapChoiceScreen(int House);
	SDL_Surface* createMentatHouseChoiceQuestion(int House, Palette& benePalette);
	SDL_Surface* createBuilderListUpperCap();
	SDL_Surface* createBuilderListLowerCap();

	SDL_Surface* createHeraldFre(SDL_Surface* heraldHark);
	SDL_Surface* createHeraldSard(SDL_Surface* heraldOrd, SDL_Surface* heraldAtre);
	SDL_Surface* createHeraldMerc(SDL_Surface* heraldAtre, SDL_Surface* heraldOrd);

	static Animation* createFremenPlanet(SDL_Surface* heraldFre);
	static Animation* createSardaukarPlanet(Animation* ordosPlanetAnimation, SDL_Surface* heraldSard);
	static Animation* createMercenaryPlanet(Animation* atreidesPlanetAnimation, SDL_Surface* heraldMerc);

	static SDL_Surface* mapMentatSurfaceToFremen(SDL_Surface* atreidesMentat);
    static Animation* mapMentatAnimationToFremen(Animation* atreidesAnimation);
	static SDL_Surface* mapMentatSurfaceToSardaukar(SDL_Surface* harkonnenMentat);
    static Animation* mapMentatAnimationToSardaukar(Animation* harkonnenAnimation);
	static SDL_Surface* mapMentatSurfaceToMercenary(SDL_Surface* ordosMentat);
    static Animation* mapMentatAnimationToMercenary(Animation* ordosAnimation);

	typedef enum {
		SimpleFrame,
		DecorationFrame1,
		DecorationFrame2,
		NUM_DECORATIONFRAMES
	} DecorationFrame;

private:
	struct DecorationBorderType {
		std::shared_ptr<SDL_Surface> ball;
		std::shared_ptr<SDL_Surface> hspacer;
		std::shared_ptr<SDL_Surface> vspacer;
		std::shared_ptr<SDL_Surface> hborder;
		std::shared_ptr<SDL_Surface> vborder;
	} decorationBorder;

	struct BorderStyle {
		std::shared_ptr<SDL_Surface> leftUpperCorner;
		std::shared_ptr<SDL_Surface> rightUpperCorner;
		std::shared_ptr<SDL_Surface> leftLowerCorner;
		std::shared_ptr<SDL_Surface> rightLowerCorner;
		std::shared_ptr<SDL_Surface> hborder;
		std::shared_ptr<SDL_Surface> vborder;
	} frame[NUM_DECORATIONFRAMES];

	std::shared_ptr<SDL_Surface> background;
	std::shared_ptr<SDL_Surface> gameStatsBackground;
	std::shared_ptr<SDL_Surface> creditsBorder;

	std::shared_ptr<SDL_Surface> harkonnenLogo;
	std::shared_ptr<SDL_Surface> atreidesLogo;
	std::shared_ptr<SDL_Surface> ordosLogo;

	std::shared_ptr<SDL_Surface> messageBoxBorder;

	std::shared_ptr<SDL_Surface> mentatHouseChoiceQuestionSurface;

	std::shared_ptr<SDL_Surface> builderListUpperCap;
	std::shared_ptr<SDL_Surface> builderListLowerCap;
};

#endif // PICTUREFACTORY_H
