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

#include <FileClasses/FileManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/Cpsfile.h>
#include <FileClasses/Wsafile.h>

#include <misc/draw_util.h>
#include <misc/Scaler.h>

#include <stdexcept>
#include <memory>

using std::shared_ptr;

PictureFactory::PictureFactory() {
    shared_ptr<SDL_Surface> ScreenPic = shared_ptr<SDL_Surface>( LoadCPS_RW(pFileManager->openFile("SCREEN.CPS"),true), SDL_FreeSurface);
    if(ScreenPic.get() == NULL) {
        throw std::runtime_error("PictureFactory::PictureFactory(): Cannot read SCREEN.CPS!");
    }

    shared_ptr<SDL_Surface> FamePic = shared_ptr<SDL_Surface>( LoadCPS_RW(pFileManager->openFile("FAME.CPS"),true), SDL_FreeSurface);
    if(FamePic.get() == NULL) {
        throw std::runtime_error("PictureFactory::PictureFactory(): Cannot read FAME.CPS!");
    }

    shared_ptr<SDL_Surface> ChoamPic = shared_ptr<SDL_Surface>( LoadCPS_RW(pFileManager->openFile("CHOAM.CPS"),true), SDL_FreeSurface);
    if(ChoamPic.get() == NULL) {
        throw std::runtime_error("PictureFactory::PictureFactory(): Cannot read CHOAM.CPS!");
    }

    creditsBorder = shared_ptr<SDL_Surface>(getSubPicture(ScreenPic.get() ,257,2,63,13), SDL_FreeSurface);

	// background
	background = shared_ptr<SDL_Surface>( SDL_CreateRGBSurface(0,settings.video.width,settings.video.height,8,0,0,0,0), SDL_FreeSurface);
	if(background.get() == NULL) {
		throw std::runtime_error("PictureFactory::PictureFactory: Cannot create new Picture!");
	}
	palette.applyToSurface(background.get());

    shared_ptr<SDL_Surface> PatternNormal = shared_ptr<SDL_Surface>( getSubPicture(FamePic.get(),0,1,63,67), SDL_FreeSurface);
	shared_ptr<SDL_Surface> PatternHFlipped = shared_ptr<SDL_Surface>( flipHSurface(getSubPicture(FamePic.get(),0,1,63,67)), SDL_FreeSurface);
	shared_ptr<SDL_Surface> PatternVFlipped = shared_ptr<SDL_Surface>( flipVSurface(getSubPicture(FamePic.get(),0,1,63,67)), SDL_FreeSurface);
	shared_ptr<SDL_Surface> PatternHVFlipped = shared_ptr<SDL_Surface>( flipHSurface(flipVSurface(getSubPicture(FamePic.get(),0,1,63,67))), SDL_FreeSurface);

	SDL_Rect dest;
	dest.w = 63;
	dest.h = 67;
	for(dest.y = 0; dest.y < settings.video.height; dest.y+= 67) {
		for(dest.x = 0; dest.x < settings.video.width; dest.x+= 63) {
			if((dest.x % (63*2) == 0) && (dest.y % (67*2) == 0)) {
			    SDL_Rect tmpDest = dest;
				SDL_BlitSurface(PatternNormal.get(), NULL, background.get(), &tmpDest);
			} else if((dest.x % (63*2) != 0) && (dest.y % (67*2) == 0)) {
			    SDL_Rect tmpDest = dest;
				SDL_BlitSurface(PatternHFlipped.get(), NULL, background.get(), &tmpDest);
			} else if((dest.x % (63*2) == 0) && (dest.y % (67*2) != 0)) {
			    SDL_Rect tmpDest = dest;
				SDL_BlitSurface(PatternVFlipped.get(), NULL, background.get(), &tmpDest);
			} else /*if((dest.x % (63*2) != 0) && (dest.y % (67*2) != 0))*/ {
			    SDL_Rect tmpDest = dest;
				SDL_BlitSurface(PatternHVFlipped.get(), NULL, background.get(), &tmpDest);
			}
		}
	}

	// decoration border
	decorationBorder.ball = shared_ptr<SDL_Surface>( getSubPicture(ScreenPic.get(),241,124,12,11), SDL_FreeSurface);
	decorationBorder.vspacer = shared_ptr<SDL_Surface>( getSubPicture(ScreenPic.get(),241,118,12,5), SDL_FreeSurface);
	decorationBorder.hspacer = shared_ptr<SDL_Surface>( rotateSurfaceRight(copySurface(decorationBorder.vspacer.get() )), SDL_FreeSurface);
	decorationBorder.vborder = shared_ptr<SDL_Surface>( getSubPicture(ScreenPic.get(),241,71,12,13), SDL_FreeSurface);
	decorationBorder.hborder = shared_ptr<SDL_Surface>( rotateSurfaceRight(copySurface(decorationBorder.vborder.get() )), SDL_FreeSurface);

	// simple Frame
	frame[SimpleFrame].leftUpperCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),120,17,8,8), SDL_FreeSurface);
	putPixel(frame[SimpleFrame].leftUpperCorner.get(),7,7,0);
	putPixel(frame[SimpleFrame].leftUpperCorner.get(),6,7,0);
	putPixel(frame[SimpleFrame].leftUpperCorner.get(),7,6,0);

	frame[SimpleFrame].rightUpperCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),312,17,8,8), SDL_FreeSurface);
	putPixel(frame[SimpleFrame].rightUpperCorner.get(),0,7,0);
	putPixel(frame[SimpleFrame].rightUpperCorner.get(),0,6,0);
	putPixel(frame[SimpleFrame].rightUpperCorner.get(),1,7,0);

	frame[SimpleFrame].leftLowerCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),120,31,8,8), SDL_FreeSurface);
	putPixel(frame[SimpleFrame].leftLowerCorner.get(),7,0,0);
	putPixel(frame[SimpleFrame].leftLowerCorner.get(),6,0,0);
	putPixel(frame[SimpleFrame].leftLowerCorner.get(),7,1,0);

	frame[SimpleFrame].rightLowerCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),312,31,8,8), SDL_FreeSurface);
	putPixel(frame[SimpleFrame].rightLowerCorner.get(),0,0,0);
	putPixel(frame[SimpleFrame].rightLowerCorner.get(),1,0,0);
	putPixel(frame[SimpleFrame].rightLowerCorner.get(),0,1,0);

	frame[SimpleFrame].hborder = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),128,17,1,4), SDL_FreeSurface);
	frame[SimpleFrame].vborder = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),120,25,4,1), SDL_FreeSurface);

	// Decoration Frame 1
	frame[DecorationFrame1].leftUpperCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),2,57,11,12), SDL_FreeSurface);
	putPixel(frame[DecorationFrame1].leftUpperCorner.get(),10,11,0);
	putPixel(frame[DecorationFrame1].leftUpperCorner.get(),9,11,0);
	putPixel(frame[DecorationFrame1].leftUpperCorner.get(),10,10,0);

	frame[DecorationFrame1].rightUpperCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),44,57,11,12), SDL_FreeSurface);
	putPixel(frame[DecorationFrame1].rightUpperCorner.get(),0,11,0);
	putPixel(frame[DecorationFrame1].rightUpperCorner.get(),0,10,0);
	putPixel(frame[DecorationFrame1].rightUpperCorner.get(),1,11,0);

	frame[DecorationFrame1].leftLowerCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),2,132,11,11), SDL_FreeSurface);
	putPixel(frame[DecorationFrame1].leftLowerCorner.get(),10,0,0);
	putPixel(frame[DecorationFrame1].leftLowerCorner.get(),9,0,0);
	putPixel(frame[DecorationFrame1].leftLowerCorner.get(),10,1,0);

	frame[DecorationFrame1].rightLowerCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),44,132,11,11), SDL_FreeSurface);
	putPixel(frame[DecorationFrame1].rightLowerCorner.get(),0,0,0);
	putPixel(frame[DecorationFrame1].rightLowerCorner.get(),1,0,0);
	putPixel(frame[DecorationFrame1].rightLowerCorner.get(),0,1,0);

	frame[DecorationFrame1].hborder = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),13,57,1,4), SDL_FreeSurface);
	frame[DecorationFrame1].vborder = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),2,69,4,1), SDL_FreeSurface);

	// Decoration Frame 2
	frame[DecorationFrame2].leftUpperCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),121,41,9,9), SDL_FreeSurface);
	drawHLine(frame[DecorationFrame2].leftUpperCorner.get(),6,6,8,0);
	drawHLine(frame[DecorationFrame2].leftUpperCorner.get(),6,7,8,0);
	drawHLine(frame[DecorationFrame2].leftUpperCorner.get(),6,8,8,0);

	frame[DecorationFrame2].rightUpperCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),309,41,10,9), SDL_FreeSurface);
	drawHLine(frame[DecorationFrame2].rightUpperCorner.get(),0,6,3,0);
	drawHLine(frame[DecorationFrame2].rightUpperCorner.get(),0,7,3,0);
	drawHLine(frame[DecorationFrame2].rightUpperCorner.get(),0,8,3,0);

	frame[DecorationFrame2].leftLowerCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),121,157,9,10), SDL_FreeSurface);
	drawHLine(frame[DecorationFrame2].leftLowerCorner.get(),6,0,8,0);
	drawHLine(frame[DecorationFrame2].leftLowerCorner.get(),6,1,8,0);
	drawHLine(frame[DecorationFrame2].leftLowerCorner.get(),6,2,8,0);
	drawHLine(frame[DecorationFrame2].leftLowerCorner.get(),7,3,8,0);

	frame[DecorationFrame2].rightLowerCorner = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),309,158,10,9), SDL_FreeSurface);
	drawHLine(frame[DecorationFrame2].rightLowerCorner.get(),0,0,3,0);
	drawHLine(frame[DecorationFrame2].rightLowerCorner.get(),0,1,3,0);
	drawHLine(frame[DecorationFrame2].rightLowerCorner.get(),0,2,3,0);

	frame[DecorationFrame2].hborder = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),133,41,1,4), SDL_FreeSurface);
	frame[DecorationFrame2].vborder = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),121,51,4,1), SDL_FreeSurface);

	for(int i=0; i < NUM_DECORATIONFRAMES; i++) {
		SDL_SetColorKey(frame[i].leftUpperCorner.get(), SDL_TRUE, 0);
		SDL_SetColorKey(frame[i].leftLowerCorner.get(), SDL_TRUE, 0);
		SDL_SetColorKey(frame[i].rightUpperCorner.get(), SDL_TRUE, 0);
		SDL_SetColorKey(frame[i].rightLowerCorner.get(), SDL_TRUE, 0);
		SDL_SetColorKey(frame[i].hborder.get(), SDL_TRUE, 0);
		SDL_SetColorKey(frame[i].vborder.get(), SDL_TRUE, 0);
	}

	// House Logos
	harkonnenLogo = shared_ptr<SDL_Surface>( getSubPicture(FamePic.get(),10,137,53,54), SDL_FreeSurface);
	atreidesLogo = shared_ptr<SDL_Surface>( getSubPicture(FamePic.get(),66,137,53,54), SDL_FreeSurface);
	ordosLogo = shared_ptr<SDL_Surface>( getSubPicture(FamePic.get(),122,137,53,54), SDL_FreeSurface);

	gameStatsBackground = shared_ptr<SDL_Surface>( copySurface(background.get()), SDL_FreeSurface);
	shared_ptr<SDL_Surface> FamePic2 = shared_ptr<SDL_Surface>( Scaler::defaultDoubleSurface(FamePic.get(), false), SDL_FreeSurface);
	shared_ptr<SDL_Surface> pSurface = shared_ptr<SDL_Surface>( getSubPicture(FamePic2.get(),16,160,610,74), SDL_FreeSurface);

	SDL_Rect dest2 = calcDrawingRect(pSurface.get(), 16, 234);
	SDL_BlitSurface(pSurface.get(), NULL, FamePic2.get(), &dest2);

	SDL_Rect dest3 = calcDrawingRect(pSurface.get(), 16, 234 + 74);
	SDL_BlitSurface(pSurface.get() , NULL, FamePic2.get(), &dest3);

    SDL_Rect dest4 = calcAlignedDrawingRect(FamePic2.get(), gameStatsBackground.get());
	SDL_BlitSurface(FamePic2.get(), NULL, gameStatsBackground.get() , &dest4);

	messageBoxBorder = shared_ptr<SDL_Surface>(getSubPicture(ScreenPic.get(),0,17,320,22), SDL_FreeSurface);

    if(pFileManager->exists("MISC." + _("LanguageFileExtension"))) {
        mentatHouseChoiceQuestionSurface = shared_ptr<SDL_Surface>(Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MISC." + _("LanguageFileExtension")), true), true), SDL_FreeSurface);
    } else {
        mentatHouseChoiceQuestionSurface = shared_ptr<SDL_Surface>(Scaler::defaultDoubleSurface(LoadCPS_RW(pFileManager->openFile("MISC.CPS"), true), true), SDL_FreeSurface);
    }


    // create builder list upper cap
    builderListUpperCap = shared_ptr<SDL_Surface>( SDL_CreateRGBSurface(0, 112, 21, 8, 0, 0, 0, 0), SDL_FreeSurface);
	if(builderListUpperCap.get() == NULL) {
		throw std::runtime_error("PictureFactory::PictureFactory: Cannot create new Picture!");
	}
	palette.applyToSurface(builderListUpperCap.get());
	SDL_FillRect(builderListUpperCap.get(), NULL, PALCOLOR_TRANSPARENT);

	shared_ptr<SDL_Surface> builderListUpperCapLeft = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),64,3,42,18), SDL_FreeSurface);
	SDL_Rect dest5 = {  0, 0, 42, 18 };
	SDL_BlitSurface(builderListUpperCapLeft.get(), NULL, builderListUpperCap.get(), &dest5);

    shared_ptr<SDL_Surface> builderListUpperCapMiddle = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),69,3,38,13), SDL_FreeSurface);
	SDL_Rect dest6 = {  42, 0, 38, 13 };
	SDL_BlitSurface(builderListUpperCapMiddle.get(), NULL, builderListUpperCap.get(), &dest6);

    shared_ptr<SDL_Surface> builderListUpperCapRight = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),69,3,48,21), SDL_FreeSurface);
	SDL_Rect dest7 = {  64, 0, 48, 21 };
	SDL_BlitSurface(builderListUpperCapRight.get(), NULL, builderListUpperCap.get(), &dest7);

	replaceColor(builderListUpperCap.get(), 30, 0);
    SDL_SetColorKey(builderListUpperCap.get(), SDL_TRUE, 0);

    // create builder list lower cap
    builderListLowerCap = shared_ptr<SDL_Surface>( SDL_CreateRGBSurface(0, 112, 17, 8, 0, 0, 0, 0), SDL_FreeSurface);
	if(builderListLowerCap.get() == NULL) {
		throw std::runtime_error("PictureFactory::PictureFactory: Cannot create new Picture!");
	}
	palette.applyToSurface(builderListLowerCap.get());
	SDL_FillRect(builderListLowerCap.get(), NULL, PALCOLOR_TRANSPARENT);

	shared_ptr<SDL_Surface> builderListLowerCapLeft = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),64,149,44,17), SDL_FreeSurface);
	SDL_Rect dest8 = {  0, 0, 44, 17 };
	SDL_BlitSurface(builderListLowerCapLeft.get(), NULL, builderListLowerCap.get(), &dest8);

    shared_ptr<SDL_Surface> builderListLowerCapMiddle = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),68,152,40,14), SDL_FreeSurface);
	SDL_Rect dest9 = {  44, 3, 40, 14 };
	SDL_BlitSurface(builderListLowerCapMiddle.get(), NULL, builderListLowerCap.get(), &dest9);

    shared_ptr<SDL_Surface> builderListLowerCapRight = shared_ptr<SDL_Surface>( getSubPicture(ChoamPic.get(),68,149,48,17), SDL_FreeSurface);
	SDL_Rect dest10 = {  64, 0, 48, 17 };
	SDL_BlitSurface(builderListLowerCapRight.get(), NULL, builderListLowerCap.get(), &dest10);

	replaceColor(builderListLowerCap.get(), 30, 0);
    SDL_SetColorKey(builderListLowerCap.get(), SDL_TRUE, 0);
}

PictureFactory::~PictureFactory() {
}

SDL_Surface* PictureFactory::createTopBar() {
	SDL_Surface* topBar;
	topBar = getSubPicture(background.get() ,0,0,settings.video.width-SIDEBARWIDTH,32+12);
	SDL_Rect dest1 = {0,31,getWidth(topBar),12};
	SDL_FillRect(topBar,&dest1,PALCOLOR_TRANSPARENT);

	SDL_Rect dest2 = calcDrawingRect(decorationBorder.hborder.get(),0,32);
	for(dest2.x = 0; dest2.x < topBar->w; dest2.x+=decorationBorder.hborder.get()->w) {
	    SDL_Rect tmpDest = dest2;
		SDL_BlitSurface(decorationBorder.hborder.get(),NULL,topBar,&tmpDest);
	}

	drawVLine(topBar,topBar->w-7,32,topBar->h-1,96);

	SDL_Rect dest3 = {getWidth(topBar) - 6, getHeight(topBar)-12, 12, 5};
	SDL_BlitSurface(decorationBorder.hspacer.get(),NULL,topBar,&dest3);

	drawVLine(topBar,topBar->w-1,0,topBar->h-1,0);

	return topBar;
}

SDL_Surface* PictureFactory::createSideBar(bool bEditor) {
	SDL_Surface* sideBar;
	sideBar = getSubPicture(background.get(),0,0,SIDEBARWIDTH,settings.video.height);
	SDL_Rect dest1 = {0,0,13,getHeight(sideBar)};
	SDL_FillRect(sideBar,&dest1,PALCOLOR_TRANSPARENT);


	SDL_Rect dest2 = calcDrawingRect(decorationBorder.vborder.get(),0,0);
	for(dest2.y = 0; dest2.y < sideBar->h; dest2.y+=decorationBorder.vborder.get()->h) {
	    SDL_Rect tmpDest = dest2;
		SDL_BlitSurface(decorationBorder.vborder.get(),NULL,sideBar,&tmpDest);
	}

	SDL_Rect dest3 = calcDrawingRect(decorationBorder.vspacer.get(),0,30,HAlign::Left,VAlign::Bottom);
	SDL_BlitSurface(decorationBorder.vspacer.get(),NULL,sideBar,&dest3);

	drawHLine(sideBar,0,32-decorationBorder.vspacer.get()->h-2,decorationBorder.vspacer.get()->w-1,96);
	drawHLine(sideBar,0,31,decorationBorder.vspacer.get()->w-1,0);

	SDL_Rect dest4 = calcDrawingRect(decorationBorder.ball.get(),0,32);
	SDL_BlitSurface(decorationBorder.ball.get(),NULL,sideBar,&dest4);

	drawHLine(sideBar,0,43,decorationBorder.vspacer.get()->w-1,0);
	SDL_Rect dest5 = calcDrawingRect(decorationBorder.vspacer.get(),0,44);
	SDL_BlitSurface(decorationBorder.vspacer.get(),NULL,sideBar,&dest5);
	drawHLine(sideBar,0,44+decorationBorder.vspacer.get()->h,decorationBorder.vspacer.get()->w-1,96);

	SDL_Rect dest6 = {13,0,getWidth(sideBar)-1,132};
	SDL_FillRect(sideBar,&dest6,PALCOLOR_TRANSPARENT);
	drawRect(sideBar,13,1,sideBar->w-2,130,115);

	SDL_Rect dest7 = calcDrawingRect(decorationBorder.vspacer.get(),0,130,HAlign::Left,VAlign::Bottom);
	SDL_BlitSurface(decorationBorder.vspacer.get(),NULL,sideBar,&dest7);

	drawHLine(sideBar,0,132-decorationBorder.vspacer.get()->h-2,decorationBorder.vspacer.get()->w-1,96);
	drawHLine(sideBar,0,131,decorationBorder.vspacer.get()->w-1,0);

	SDL_Rect dest8 = calcDrawingRect(decorationBorder.ball.get(),0,132);
	SDL_BlitSurface(decorationBorder.ball.get(),NULL,sideBar,&dest8);

	drawHLine(sideBar,0,143,decorationBorder.vspacer.get()->w-1,0);
	SDL_Rect dest9 = calcDrawingRect(decorationBorder.vspacer.get(),0,144);
	SDL_BlitSurface(decorationBorder.vspacer.get(),NULL,sideBar,&dest9);
	drawHLine(sideBar,0,144+decorationBorder.vspacer.get()->h,decorationBorder.vspacer.get()->w-1,96);

	SDL_Rect dest10 = calcDrawingRect(decorationBorder.hspacer.get(),13,132);
	SDL_BlitSurface(decorationBorder.hspacer.get(),NULL,sideBar,&dest10);

	drawVLine(sideBar,18,132,132+decorationBorder.hspacer.get()->h-1,96);
	drawHLine(sideBar,13,132+decorationBorder.hspacer.get()->h,sideBar->w-1,0);

	SDL_Rect dest11 = calcDrawingRect(decorationBorder.hborder.get(),0,132);
	for(dest11.x = 19; dest11.x < sideBar->w; dest11.x+=decorationBorder.hborder.get()->w) {
	    SDL_Rect tmpDest = dest11;
		SDL_BlitSurface(decorationBorder.hborder.get(),NULL,sideBar,&tmpDest);
	}

    if(bEditor) {
        SDL_Rect dest12 = calcDrawingRect(decorationBorder.vspacer.get(),0,getHeight(sideBar) - 32 - 14,HAlign::Left,VAlign::Bottom);
        SDL_BlitSurface(decorationBorder.vspacer.get(),NULL,sideBar,&dest12);

        drawHLine(sideBar,0,sideBar->h - 32 - 12 - decorationBorder.vspacer.get()->h-2,decorationBorder.vspacer.get()->w-1,96);
        drawHLine(sideBar,0,sideBar->h - 32 - 12 - 1,decorationBorder.vspacer.get()->w-1,0);

        SDL_Rect dest13 = calcDrawingRect(decorationBorder.ball.get(),0,getHeight(sideBar) - 32 - 12);
        SDL_BlitSurface(decorationBorder.ball.get(),NULL,sideBar,&dest13);

        drawHLine(sideBar,0,sideBar->h - 32 - 1,decorationBorder.vspacer.get()->w-1,0);
        SDL_Rect dest14 = calcDrawingRect(decorationBorder.vspacer.get(),0,getHeight(sideBar) - 32);
        SDL_BlitSurface(decorationBorder.vspacer.get(),NULL,sideBar,&dest14);
        drawHLine(sideBar,0,sideBar->h - 32 + decorationBorder.vspacer.get()->h,decorationBorder.vspacer.get()->w-1,96);
    } else {
        SDL_Rect dest15 = calcDrawingRect(creditsBorder.get(),46,132);
        SDL_BlitSurface(creditsBorder.get(),NULL,sideBar,&dest15);
    }

	return sideBar;
}

SDL_Surface* PictureFactory::createBottomBar() {
	SDL_Surface* BottomBar;
	BottomBar = getSubPicture(background.get() ,0,0,settings.video.width-SIDEBARWIDTH,32+12);
	SDL_Rect dest1 = {0,0,getWidth(BottomBar),13};
	SDL_FillRect(BottomBar,&dest1,PALCOLOR_TRANSPARENT);

	SDL_Rect dest2 = calcDrawingRect(decorationBorder.hborder.get(), 0, 0);
	for(dest2.x = 0; dest2.x < BottomBar->w; dest2.x+=decorationBorder.hborder.get()->w) {
	    SDL_Rect tmpDest = dest2;
		SDL_BlitSurface(decorationBorder.hborder.get(),NULL,BottomBar,&tmpDest);
	}

	drawVLine(BottomBar,BottomBar->w-7,0,11,96);

	SDL_Rect dest3 = {getWidth(BottomBar) - 6,0,12,5};
	SDL_BlitSurface(decorationBorder.hspacer.get(),NULL,BottomBar,&dest3);

	drawVLine(BottomBar,BottomBar->w-1,0,BottomBar->h-1,0);

	return BottomBar;
}

SDL_Surface* PictureFactory::createPlacingGrid(int size, int color) {
	SDL_Surface* placingGrid;
	if((placingGrid = SDL_CreateRGBSurface(0,size,size,8,0,0,0,0)) == NULL) {
		fprintf(stderr,"PictureFactory::createPlacingGrid: Cannot create new Picture!\n");
		exit(EXIT_FAILURE);
	}
	palette.applyToSurface(placingGrid);

	if (!SDL_MUSTLOCK(placingGrid) || (SDL_LockSurface(placingGrid) == 0))
	{
		for(int y = 0; y < size; y++) {
			for(int x = 0; x < size; x++) {
				if(x%2 == y%2) {
					*((Uint8 *)placingGrid->pixels + y * placingGrid->pitch + x) = color;
				} else {
					*((Uint8 *)placingGrid->pixels + y * placingGrid->pitch + x) = 0;
				}
			}
		}

		if (SDL_MUSTLOCK(placingGrid))
			SDL_UnlockSurface(placingGrid);
	}

	SDL_SetColorKey(placingGrid, SDL_TRUE, 0);

	return placingGrid;
}


void PictureFactory::drawFrame(SDL_Surface* Pic, unsigned int DecorationType, SDL_Rect* dest) {
	if(Pic == NULL)
		return;

	if(DecorationType >= NUM_DECORATIONFRAMES)
		return;

	SDL_Rect tmp;
	if(dest == NULL) {
		tmp.x = 0;
		tmp.y = 0;
		tmp.w = Pic->w;
		tmp.h = Pic->h;
		dest = &tmp;
	}

	//corners
	SDL_Rect dest1 = calcDrawingRect(frame[DecorationType].leftUpperCorner.get(), dest->x, dest->y);
	SDL_BlitSurface(frame[DecorationType].leftUpperCorner.get(),NULL,Pic,&dest1);

	SDL_Rect dest2 = calcDrawingRect(frame[DecorationType].rightUpperCorner.get(), dest->w-1, dest->y, HAlign::Right, VAlign::Top);
	SDL_BlitSurface(frame[DecorationType].rightUpperCorner.get(),NULL,Pic,&dest2);

    SDL_Rect dest3 = calcDrawingRect(frame[DecorationType].leftLowerCorner.get(), dest->x, dest->h-1, HAlign::Left, VAlign::Bottom);
	SDL_BlitSurface(frame[DecorationType].leftLowerCorner.get(),NULL,Pic,&dest3);

    SDL_Rect dest4 = calcDrawingRect(frame[DecorationType].rightLowerCorner.get(), dest->w-1, dest->h-1, HAlign::Right, VAlign::Bottom);
	SDL_BlitSurface(frame[DecorationType].rightLowerCorner.get(),NULL,Pic,&dest4);

	//hborders
	SDL_Rect dest5 = calcDrawingRect(frame[DecorationType].hborder.get(), dest->x, dest->y);
	for(dest5.x = frame[DecorationType].leftUpperCorner.get()->w + dest->x;
		dest5.x <= dest->w - frame[DecorationType].rightUpperCorner.get()->w - 1;
		dest5.x += frame[DecorationType].hborder.get()->w) {
        SDL_Rect tmpDest = dest5;
		SDL_BlitSurface(frame[DecorationType].hborder.get(),NULL,Pic,&tmpDest);
	}

    SDL_Rect dest6 = calcDrawingRect(frame[DecorationType].hborder.get(), dest->x, dest->h-1, HAlign::Left, VAlign::Bottom);
	for(dest6.x = frame[DecorationType].leftLowerCorner.get()->w + dest->x;
		dest6.x <= dest->w - frame[DecorationType].rightLowerCorner.get()->w - 1;
		dest6.x += frame[DecorationType].hborder.get()->w) {
        SDL_Rect tmpDest = dest6;
		SDL_BlitSurface(frame[DecorationType].hborder.get(),NULL,Pic,&tmpDest);
	}

	//vborders
    SDL_Rect dest7 = calcDrawingRect(frame[DecorationType].vborder.get(), dest->x, dest->y);
	for(dest7.y = frame[DecorationType].leftUpperCorner.get()->h + dest->y;
		dest7.y <= dest->h - frame[DecorationType].leftLowerCorner.get()->h - 1;
		dest7.y += frame[DecorationType].vborder.get()->h) {
        SDL_Rect tmpDest = dest7;
		SDL_BlitSurface(frame[DecorationType].vborder.get(),NULL,Pic,&tmpDest);
	}

    SDL_Rect dest8 = calcDrawingRect(frame[DecorationType].vborder.get(), dest->w-1, dest->y, HAlign::Right, VAlign::Top);
	for(dest8.y = frame[DecorationType].rightUpperCorner.get()->h + dest->y;
		dest8.y <= dest->h - frame[DecorationType].rightLowerCorner.get()->h - 1;
		dest8.y += frame[DecorationType].vborder.get()->h) {
        SDL_Rect tmpDest = dest8;
		SDL_BlitSurface(frame[DecorationType].vborder.get(),NULL,Pic,&tmpDest);
	}

}

SDL_Surface* PictureFactory::createFrame(unsigned int DecorationType,int width, int height,bool UseBackground) {
	SDL_Surface* Pic;
	if(UseBackground) {
		Pic = getSubPicture(background.get(),0,0,width,height);
	} else {
		if((Pic = SDL_CreateRGBSurface(0,width,height,8,0,0,0,0)) == NULL) {
			fprintf(stderr,"PictureFactory::createFrame: Cannot create new Picture!\n");
			exit(EXIT_FAILURE);
		}
		palette.applyToSurface(Pic);
		SDL_SetColorKey(Pic, SDL_TRUE, 0);
	}

	drawFrame(Pic,DecorationType);

	return Pic;
}

SDL_Surface* PictureFactory::createBackground() {
	return copySurface(background.get());
}

SDL_Surface* PictureFactory::createMainBackground() {
	SDL_Surface* Pic;
	Pic = copySurface(background.get());

	SDL_Rect dest0 = { 3,3,getWidth(Pic)-3, getHeight(Pic)-3};
	drawFrame(Pic,DecorationFrame2,&dest0);

	SDL_Rect dest1 = calcDrawingRect(harkonnenLogo.get(),11,11);
	SDL_BlitSurface(harkonnenLogo.get(),NULL,Pic,&dest1);

	SDL_Rect dest2 = calcDrawingRect(atreidesLogo.get(),getWidth(Pic)-11,11,HAlign::Right,VAlign::Top);
	SDL_BlitSurface(atreidesLogo.get(),NULL,Pic,&dest2);

	SDL_Rect dest3 = calcDrawingRect(ordosLogo.get(),11,getHeight(Pic)-11,HAlign::Left,VAlign::Bottom);
	SDL_BlitSurface(ordosLogo.get(),NULL,Pic,&dest3);

	SDL_Surface* Version = getSubPicture(background.get(),0,0,75,32);

	char versionString[100];
	sprintf(versionString, "%s", VERSION);

	SDL_Surface *VersionText = pFontManager->createSurfaceWithText(versionString, PALCOLOR_BLACK, FONT_STD12);

	SDL_Rect dest4 = calcDrawingRect(VersionText, getWidth(Version)/2, getHeight(Version)/2 + 2, HAlign::Center, VAlign::Center);
	SDL_BlitSurface(VersionText,NULL,Version,&dest4);

	SDL_FreeSurface(VersionText);

	drawFrame(Version,SimpleFrame);

	SDL_Rect dest5 = calcDrawingRect(Version, getWidth(Pic) - 11, getHeight(Pic) - 11, HAlign::Right, VAlign::Bottom);
	SDL_BlitSurface(Version,NULL,Pic,&dest5);

	SDL_FreeSurface(Version);

	return Pic;
}

SDL_Surface* PictureFactory::createGameStatsBackground(int House) {
    SDL_Surface* pSurface = copySurface(gameStatsBackground.get());

    SDL_Surface* pLogo = NULL;
    switch(House) {
        case HOUSE_HARKONNEN:
        case HOUSE_SARDAUKAR: {
            pLogo = copySurface(harkonnenLogo.get());
        } break;

        case HOUSE_ATREIDES:
        case HOUSE_FREMEN: {
            pLogo = copySurface(atreidesLogo.get());
        } break;

        case HOUSE_ORDOS:
        case HOUSE_MERCENARY: {
            pLogo = copySurface(ordosLogo.get());
        } break;
    }

    pLogo = Scaler::defaultDoubleSurface(pLogo,true);

    SDL_Rect dest1 = calcDrawingRect(pLogo, getWidth(gameStatsBackground.get())/2 - 320 + 2, getHeight(gameStatsBackground.get())/2 - 200 + 16);
    SDL_BlitSurface(pLogo, NULL, pSurface, &dest1);
    SDL_Rect dest2 = calcDrawingRect(pLogo, getWidth(gameStatsBackground.get())/2 + 320 - 3, getHeight(gameStatsBackground.get())/2 - 200 + 16, HAlign::Right, VAlign::Top);
    SDL_BlitSurface(pLogo, NULL, pSurface, &dest2);

    SDL_FreeSurface(pLogo);

    return pSurface;
}

SDL_Surface* PictureFactory::createMenu(int x,int y) {
	SDL_Surface* Pic = getSubPicture(background.get(), 0,0,x,y);

	SDL_Rect dest1 = {0,0,getWidth(Pic),27};

    SDL_FillRect(Pic,&dest1,PALCOLOR_GREY);

	drawFrame(Pic,SimpleFrame,&dest1);

	SDL_Rect dest2 = calcDrawingRect(Pic,0,dest1.h);
	drawFrame(Pic,DecorationFrame1,&dest2);

	return Pic;
}

SDL_Surface* PictureFactory::createMenu(SDL_Surface* CaptionPic,int y) {
	if(CaptionPic == NULL)
		return NULL;

	SDL_Surface* Pic = getSubPicture(background.get(), 0,0,CaptionPic->w,y);

	SDL_Rect dest1 = calcDrawingRect(CaptionPic,0,0);
	SDL_BlitSurface(CaptionPic, NULL,Pic,&dest1);

	drawFrame(Pic,SimpleFrame,&dest1);

	SDL_Rect dest2 = calcDrawingRect(Pic,0,dest1.h);
	drawFrame(Pic,DecorationFrame1,&dest2);

	return Pic;
}

SDL_Surface* PictureFactory::createOptionsMenu() {
	SDL_Surface* tmp;
	if((tmp = SDL_LoadBMP_RW(pFileManager->openFile("UI_OptionsMenu.bmp"),true)) == NULL) {
		fprintf(stderr,"PictureFactory::createOptionsMenu(): Cannot load UI_OptionsMenu.bmp!\n");
		exit(EXIT_FAILURE);
	}
	SDL_SetColorKey(tmp, SDL_TRUE, 0);

	SDL_Surface* Pic = getSubPicture(background.get(),0,0,tmp->w,tmp->h);
	SDL_BlitSurface(tmp,NULL,Pic,NULL);

	SDL_Rect dest1 = {0,0,getWidth(Pic),27};
	drawFrame(Pic,SimpleFrame,&dest1);

	SDL_Rect dest2 = calcDrawingRect(Pic,0,dest1.h);
	drawFrame(Pic,DecorationFrame1,&dest2);

	SDL_FreeSurface(tmp);
	return Pic;
}

SDL_Surface* PictureFactory::createMessageBoxBorder() {
	return copySurface(messageBoxBorder.get());
}

SDL_Surface* PictureFactory::createHouseSelect(SDL_Surface* HouseChoice) {
	SDL_Surface* Pic = copySurface(HouseChoice);

    SDL_Rect dest = { 0, 50, getWidth(Pic), getHeight(Pic)-50 };
	SDL_FillRect(Pic, &dest, PALCOLOR_BLACK);

	drawFrame(Pic,SimpleFrame,NULL);

	return Pic;
}


SDL_Surface* PictureFactory::createGreyHouseChoice(SDL_Surface* HouseChoice) {
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
		0, 0, 0, 0, 13, 0, 30, 30, 183, 250, 250, 0, 0, 0, 0, 0 };


	SDL_Surface* Pic = copySurface(HouseChoice);

	for(int y = 0; y < Pic->h; y++) {
		for(int x = 0; x < Pic->w; x++) {
			unsigned char inputIndex = *( ((unsigned char*) (Pic->pixels)) + y*Pic->pitch + x);
			unsigned char outputIndex = index2greyindex[inputIndex];
			*( ((unsigned char*) (Pic->pixels)) + y*Pic->pitch + x) = outputIndex;
		}
	}

	return Pic;
}


SDL_Surface* PictureFactory::createMapChoiceScreen(int House) {
	SDL_Surface* MapChoiceScreen;

	if((MapChoiceScreen = LoadCPS_RW(pFileManager->openFile("MAPMACH.CPS"),true)) == NULL) {
		fprintf(stderr,"PictureFactory::createMapChoiceScreen(): Cannot read MAPMACH.CPS!\n");
		exit(EXIT_FAILURE);
	}

	SDL_Rect LeftLogo = calcDrawingRect(harkonnenLogo.get(),2,145);
	SDL_Rect RightLogo = calcDrawingRect(harkonnenLogo.get(),266,145);

	switch(House) {
		case HOUSE_HARKONNEN:
		case HOUSE_SARDAUKAR: {
			SDL_BlitSurface(harkonnenLogo.get(),NULL,MapChoiceScreen,&LeftLogo);
			SDL_BlitSurface(harkonnenLogo.get(),NULL,MapChoiceScreen,&RightLogo);
		} break;

		case HOUSE_ATREIDES:
		case HOUSE_FREMEN: {
			SDL_BlitSurface(atreidesLogo.get(),NULL,MapChoiceScreen,&LeftLogo);
			SDL_BlitSurface(atreidesLogo.get(),NULL,MapChoiceScreen,&RightLogo);
		} break;

		case HOUSE_ORDOS:
		case HOUSE_MERCENARY: {
			SDL_BlitSurface(ordosLogo.get(),NULL,MapChoiceScreen,&LeftLogo);
			SDL_BlitSurface(ordosLogo.get(),NULL,MapChoiceScreen,&RightLogo);
		} break;

        default: {

        } break;
	}

    if(settings.general.language == "de") {
        SDL_Surface* tmp = getSubPicture(MapChoiceScreen,8,120, 303, 23);
        SDL_Rect dest = {8,0,303,23};
        SDL_BlitSurface(tmp,NULL,MapChoiceScreen,&dest);
        SDL_FreeSurface(tmp);
    } else if(settings.general.language == "fr") {
        SDL_Surface* tmp = getSubPicture(MapChoiceScreen,8,96, 303, 23);
        SDL_Rect dest = {8,0,303,23};
        SDL_BlitSurface(tmp,NULL,MapChoiceScreen,&dest);
        SDL_FreeSurface(tmp);
    } else {
		; // Nothing to do (use English)
	}

	// clear everything in the middle
	SDL_Rect clearRect = {8,24,304,119};
	SDL_FillRect(MapChoiceScreen,&clearRect,PALCOLOR_TRANSPARENT);

	MapChoiceScreen = Scaler::defaultDoubleSurface(mapSurfaceColorRange(MapChoiceScreen, PALCOLOR_HARKONNEN, houseToPaletteIndex[House], true), true);
	SDL_Surface* FullMapChoiceScreen = copySurface(background.get());

	SDL_Rect dest = calcAlignedDrawingRect(MapChoiceScreen, FullMapChoiceScreen);
	SDL_BlitSurface(MapChoiceScreen,NULL,FullMapChoiceScreen,&dest);
	SDL_FreeSurface(MapChoiceScreen);
	return FullMapChoiceScreen;
}

SDL_Surface* PictureFactory::createMentatHouseChoiceQuestion(int House, Palette& benePalette) {
	SDL_Surface* pSurface;
	if((pSurface = SDL_CreateRGBSurface(0,416+208,48,8,0,0,0,0)) == NULL) {
		fprintf(stderr,"PictureFactory::createMentatHouseChoiceQuestion: Cannot create new Picture!\n");
		exit(EXIT_FAILURE);
	}

    benePalette.applyToSurface(pSurface);
    SDL_SetColorKey(pSurface, SDL_TRUE, 0);

	SDL_Surface* pQuestionPart1 = getSubPicture(mentatHouseChoiceQuestionSurface.get(),0,0, 416, 48);

    SDL_Surface* pQuestionPart2 = NULL;

    switch(House) {
        case HOUSE_HARKONNEN:   pQuestionPart2 = getSubPicture(mentatHouseChoiceQuestionSurface.get(),0, 48, 208, 48);   break;
        case HOUSE_ATREIDES:    pQuestionPart2 = getSubPicture(mentatHouseChoiceQuestionSurface.get(),0, 96, 208, 48);   break;
        case HOUSE_ORDOS:       pQuestionPart2 = getSubPicture(mentatHouseChoiceQuestionSurface.get(),0, 144, 208, 48);  break;
        case HOUSE_FREMEN:      pQuestionPart2 = Scaler::defaultDoubleSurface(SDL_LoadBMP_RW(pFileManager->openFile("Fremen.bmp"), true), true);      break;
        case HOUSE_SARDAUKAR:   pQuestionPart2 = Scaler::defaultDoubleSurface(SDL_LoadBMP_RW(pFileManager->openFile("Sardaukar.bmp"), true), true);   break;
        case HOUSE_MERCENARY:   pQuestionPart2 = Scaler::defaultDoubleSurface(SDL_LoadBMP_RW(pFileManager->openFile("Mercenary.bmp"), true), true);   break;
        default:    break;
    }

    SDL_SetColorKey(pQuestionPart2, SDL_TRUE, 0);

	SDL_Rect dest1 = calcDrawingRect(pQuestionPart1, 0, 0);
    SDL_BlitSurface(pQuestionPart1,NULL,pSurface,&dest1);

	SDL_Rect dest2 = calcDrawingRect(pQuestionPart2, getWidth(pQuestionPart1), 0);
    SDL_BlitSurface(pQuestionPart2,NULL,pSurface,&dest2);

    SDL_FreeSurface(pQuestionPart1);
    SDL_FreeSurface(pQuestionPart2);

    return pSurface;
}

SDL_Surface* PictureFactory::createBuilderListUpperCap() {
    return copySurface(builderListUpperCap.get());
}

SDL_Surface* PictureFactory::createBuilderListLowerCap() {
    return copySurface(builderListLowerCap.get());
}

SDL_Surface* PictureFactory::createHeraldFre(SDL_Surface* heraldHark) {
    SDL_Surface* pRedReplaced = mapSurfaceColorRange(heraldHark, PALCOLOR_HARKONNEN, PALCOLOR_FREMEN);

    SDL_Surface* pBlueReplaced = mapSurfaceColorRange(pRedReplaced, PALCOLOR_ATREIDES, PALCOLOR_FREMEN+1);
    SDL_FreeSurface(pRedReplaced);

    replaceColor(pBlueReplaced, 170, 194);
    replaceColor(pBlueReplaced, 173, 195);

    SDL_Surface* pTmp1 = scaleSurface(LoadWSA_RW(pFileManager->openFile("WORM.WSA"), 0, true), 0.5, true);
    SDL_Surface* pSandworm = getSubPicture(pTmp1, 40-18, 6-12, 83, 91);
    SDL_FreeSurface(pTmp1);

    SDL_Surface* pMask = SDL_LoadBMP_RW(pFileManager->openFile("HeraldFreMask.bmp"), true);
    SDL_SetColorKey(pMask, SDL_TRUE, 0);

    SDL_BlitSurface(pMask, NULL, pBlueReplaced, NULL);
    SDL_FreeSurface(pMask);

    SDL_SetColorKey(pBlueReplaced, SDL_TRUE, 223);

    SDL_BlitSurface(pBlueReplaced, NULL, pSandworm, NULL);
    SDL_FreeSurface(pBlueReplaced);

    return pSandworm;
}

SDL_Surface* PictureFactory::createHeraldSard(SDL_Surface* heraldOrd, SDL_Surface* heraldAtre) {
    SDL_Surface* pGreenReplaced = mapSurfaceColorRange(heraldOrd, PALCOLOR_ORDOS, PALCOLOR_SARDAUKAR-1);

    replaceColor(pGreenReplaced, 3, 209);

    SDL_Surface* pTmp1 = mapSurfaceColorRange(heraldAtre, PALCOLOR_ATREIDES, PALCOLOR_SARDAUKAR);
    SDL_Surface* pCurtain = getSubPicture(pTmp1, 7, 7, 69, 49);
    SDL_FreeSurface(pTmp1);

    SDL_Surface* pFrameAndCurtain = combinePictures(pGreenReplaced, pCurtain, 7, 7);

    SDL_Surface* pMask = SDL_LoadBMP_RW(pFileManager->openFile("HeraldSardMask.bmp"), true);
    SDL_SetColorKey(pMask, SDL_TRUE, 0);

    SDL_BlitSurface(pMask, NULL, pFrameAndCurtain, NULL);
    SDL_FreeSurface(pMask);

    return pFrameAndCurtain;
}

SDL_Surface* PictureFactory::createHeraldMerc(SDL_Surface* heraldAtre, SDL_Surface* heraldOrd) {
    SDL_Surface* pBlueReplaced = mapSurfaceColorRange(heraldAtre, PALCOLOR_ATREIDES, PALCOLOR_MERCENARY);

    SDL_Surface* pRedReplaced = mapSurfaceColorRange(pBlueReplaced, PALCOLOR_HARKONNEN, PALCOLOR_ATREIDES);
    SDL_FreeSurface(pBlueReplaced);

    SDL_Surface* pTmp1 = mapSurfaceColorRange(heraldOrd, PALCOLOR_ORDOS, PALCOLOR_MERCENARY);
    SDL_Surface* pCurtain = getSubPicture(pTmp1, 7, 7, 69, 49);
    SDL_FreeSurface(pTmp1);

    SDL_Surface* pFrameAndCurtain = combinePictures(pRedReplaced, pCurtain, 7, 7);

    SDL_Surface* pTmp2 = LoadWSA_RW(pFileManager->openFile("INFANTRY.WSA"), 0, true);
    SDL_Surface* pSoldier = getSubPicture(pTmp2, 49, 17, 83, 91);
    SDL_FreeSurface(pTmp2);

    SDL_Surface* pMask = SDL_LoadBMP_RW(pFileManager->openFile("HeraldMercMask.bmp"), true);
    SDL_SetColorKey(pMask, SDL_TRUE, 0);

    SDL_BlitSurface(pMask, NULL, pFrameAndCurtain, NULL);
    SDL_FreeSurface(pMask);

    SDL_SetColorKey(pFrameAndCurtain, SDL_TRUE, 223);

    SDL_BlitSurface(pFrameAndCurtain, NULL, pSoldier, NULL);
    SDL_FreeSurface(pFrameAndCurtain);

    return pSoldier;
}

Animation* PictureFactory::createFremenPlanet(SDL_Surface* heraldFre) {
    Animation* newAnimation = new Animation();

    SDL_Surface* pTmp = LoadCPS_RW(pFileManager->openFile("BIGPLAN.CPS"), true);
    SDL_Surface* newFrame = getSubPicture(pTmp, -68, -34, 368, 224);
    SDL_FreeSurface(pTmp);

    SDL_Rect src =  {0, 0, getWidth(heraldFre) - 2, 126};
    SDL_Rect dest = {12, 66, getWidth(heraldFre) - 2, getHeight(heraldFre)};
    SDL_BlitSurface(heraldFre,&src,newFrame,&dest);

    drawRect(newFrame, 0, 0, newFrame->w - 1, newFrame->h - 1, PALCOLOR_WHITE);

    newAnimation->addFrame(newFrame);

    return newAnimation;
}

Animation* PictureFactory::createSardaukarPlanet(Animation* ordosPlanetAnimation, SDL_Surface* heraldSard) {

    SDL_Surface* maskSurface = Scaler::defaultDoubleSurface(SDL_LoadBMP_RW(pFileManager->openFile("PlanetMask.bmp"), true), true);
    SDL_SetColorKey(maskSurface, SDL_TRUE, 0);

    Animation* newAnimation = new Animation();

    Uint8 colorMap[256];
    for(int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[15] = 165;
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

    const std::vector<SDL_Surface*>& frames = ordosPlanetAnimation->getFrames();
    std::vector<SDL_Surface*>::const_iterator iter;

    for(iter = frames.begin(); iter != frames.end(); ++iter) {
        SDL_Surface* newFrame = copySurface(*iter);

        mapColor(newFrame, colorMap);

        SDL_Surface* newFrameWithoutPlanet = copySurface(*iter);

        SDL_BlitSurface(maskSurface,NULL,newFrameWithoutPlanet,NULL);
        SDL_SetColorKey(newFrameWithoutPlanet, SDL_TRUE, 223);
        SDL_BlitSurface(newFrameWithoutPlanet,NULL,newFrame,NULL);

        SDL_FreeSurface(newFrameWithoutPlanet);

        SDL_Rect src =  {0, 0, getWidth(heraldSard), 126};
        SDL_Rect dest = calcDrawingRect(heraldSard, 12, 66);
        SDL_BlitSurface(heraldSard,&src,newFrame,&dest);

        newAnimation->addFrame(newFrame);
    }

    SDL_FreeSurface(maskSurface);

    return newAnimation;
}

Animation* PictureFactory::createMercenaryPlanet(Animation* atreidesPlanetAnimation, SDL_Surface* heraldMerc) {

    Animation* newAnimation = new Animation();

    Uint8 colorMap[256];
    for(int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[3] = 93;
    colorMap[4] = 90;
    colorMap[68] = 87;
    colorMap[69] = 88;
    colorMap[70] = 89;
    colorMap[71] = 90;
    colorMap[72] = 91;
    colorMap[73] = 92;
    colorMap[74] = 93;
    colorMap[75] = 94;
    colorMap[76] = 95;
    colorMap[176] = 91;
    colorMap[177] = 92;
    colorMap[178] = 94;
    colorMap[179] = 95;

    const std::vector<SDL_Surface*>& frames = atreidesPlanetAnimation->getFrames();
    std::vector<SDL_Surface*>::const_iterator iter;

    for(iter = frames.begin(); iter != frames.end(); ++iter) {
        SDL_Surface* newFrame = copySurface(*iter);

        mapColor(newFrame, colorMap);

        SDL_Rect src =  {0, 0, getWidth(heraldMerc), 126};
        SDL_Rect dest = calcDrawingRect(heraldMerc, 12, 66);
        SDL_BlitSurface(heraldMerc,&src,newFrame,&dest);

        newAnimation->addFrame(newFrame);
    }

    return newAnimation;
}

SDL_Surface* PictureFactory::mapMentatSurfaceToMercenary(SDL_Surface* ordosMentat) {
    SDL_Surface* mappedSurface = mapSurfaceColorRange(ordosMentat, PALCOLOR_ORDOS, PALCOLOR_MERCENARY);

    Uint8 colorMap[256];
    for(int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[186] = 245;
    colorMap[187] = 250;

    mapColor(mappedSurface, colorMap);

    return mappedSurface;
}

Animation* PictureFactory::mapMentatAnimationToFremen(Animation* fremenAnimation) {
    Animation* newAnimation = new Animation();

    const std::vector<SDL_Surface*>& frames = fremenAnimation->getFrames();
    std::vector<SDL_Surface*>::const_iterator iter;

    for(iter = frames.begin(); iter != frames.end(); ++iter) {
        SDL_Surface* newFrame = mapMentatSurfaceToFremen(*iter);
        newAnimation->addFrame(newFrame);
    }

    newAnimation->setFrameDurationTime(fremenAnimation->getFrameDurationTime());
    newAnimation->setNumLoops(fremenAnimation->getLoopsLeft());

    return newAnimation;
}

SDL_Surface* PictureFactory::mapMentatSurfaceToSardaukar(SDL_Surface* harkonnenMentat) {
    SDL_Surface* mappedSurface = mapSurfaceColorRange(harkonnenMentat, PALCOLOR_HARKONNEN, PALCOLOR_SARDAUKAR);

    Uint8 colorMap[256];
    for(int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[54] = 212;
    colorMap[56] = 212;
    colorMap[57] = 213;
    colorMap[58] = 213;
    colorMap[121] = 211;
    colorMap[199] = 210;
    colorMap[200] = 211;
    colorMap[201] = 211;
    colorMap[202] = 213;

    mapColor(mappedSurface, colorMap);

    return mappedSurface;
}

Animation* PictureFactory::mapMentatAnimationToSardaukar(Animation* harkonnenAnimation) {
    Animation* newAnimation = new Animation();

    const std::vector<SDL_Surface*>& frames = harkonnenAnimation->getFrames();
    std::vector<SDL_Surface*>::const_iterator iter;

    for(iter = frames.begin(); iter != frames.end(); ++iter) {
        SDL_Surface* newFrame = mapMentatSurfaceToSardaukar(*iter);
        newAnimation->addFrame(newFrame);
    }

    newAnimation->setFrameDurationTime(harkonnenAnimation->getFrameDurationTime());
    newAnimation->setNumLoops(harkonnenAnimation->getLoopsLeft());

    return newAnimation;
}

Animation* PictureFactory::mapMentatAnimationToMercenary(Animation* ordosAnimation) {
    Animation* newAnimation = new Animation();

    const std::vector<SDL_Surface*>& frames = ordosAnimation->getFrames();
    std::vector<SDL_Surface*>::const_iterator iter;

    for(iter = frames.begin(); iter != frames.end(); ++iter) {
        SDL_Surface* newFrame = mapMentatSurfaceToMercenary(*iter);
        newAnimation->addFrame(newFrame);
    }

    newAnimation->setFrameDurationTime(ordosAnimation->getFrameDurationTime());
    newAnimation->setNumLoops(ordosAnimation->getLoopsLeft());

    return newAnimation;
}

SDL_Surface* PictureFactory::mapMentatSurfaceToFremen(SDL_Surface* fremenMentat) {
    SDL_Surface* mappedSurface = mapSurfaceColorRange(fremenMentat, PALCOLOR_ATREIDES, PALCOLOR_FREMEN);

    Uint8 colorMap[256];
    for(int i = 0; i < 256; i++) {
        colorMap[i] = i;
    }

    colorMap[179] = 12;
    colorMap[180] = 12;
    colorMap[181] = 12;
    colorMap[182] = 12;

    mapColor(mappedSurface, colorMap);

    return mappedSurface;
}
