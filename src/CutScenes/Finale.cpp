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

#include <globals.h>

#include <CutScenes/Finale.h>
#include <CutScenes/WSAVideoEvent.h>
#include <CutScenes/HoldPictureVideoEvent.h>
#include <CutScenes/FadeOutVideoEvent.h>
#include <CutScenes/FadeInVideoEvent.h>
#include <CutScenes/CrossBlendVideoEvent.h>
#include <CutScenes/TextEvent.h>
#include <CutScenes/CutSceneSoundTrigger.h>
#include <CutScenes/CutSceneMusicTrigger.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/Pakfile.h>
#include <FileClasses/Wsafile.h>
#include <FileClasses/Cpsfile.h>
#include <FileClasses/Vocfile.h>
#include <FileClasses/IndexedTextFile.h>

#include <misc/draw_util.h>
#include <misc/sound_util.h>

#include <string>

Finale::Finale(int house) {

    switch(house) {
        case HOUSE_HARKONNEN: {
            pPalace1 = std::make_unique<Wsafile>(pFileManager->openFile("HFINALA.WSA").get());
            pPalace2 = std::make_unique<Wsafile>(pFileManager->openFile("HFINALB.WSA").get(), pFileManager->openFile("HFINALC.WSA").get());
        } break;

        case HOUSE_ATREIDES: {
            pPalace1 = std::make_unique<Wsafile>(pFileManager->openFile("AFINALA.WSA").get());
            pPalace2 = std::make_unique<Wsafile>(pFileManager->openFile("AFINALB.WSA").get());
        } break;

        case HOUSE_ORDOS: {
            pPalace1 = std::make_unique<Wsafile>(pFileManager->openFile("OFINALA.WSA").get(), pFileManager->openFile("OFINALB.WSA").get(), pFileManager->openFile("OFINALC.WSA").get());
            pPalace2 = std::make_unique<Wsafile>(pFileManager->openFile("OFINALD.WSA").get());
        } break;

        default: {
            // Nothing
        } break;
    }

    if(house == HOUSE_HARKONNEN || house == HOUSE_ATREIDES || house == HOUSE_ORDOS) {
        pImperator = std::make_unique<Wsafile>(pFileManager->openFile("EFINALA.WSA").get());
        pImperatorShocked = std::make_unique<Wsafile>(pFileManager->openFile("EFINALB.WSA").get());
    }

    auto pPlanetDuneNormalSurface = LoadCPS_RW(pFileManager->openFile("BIGPLAN.CPS").get());
    if(pPlanetDuneNormalSurface == nullptr) {
        THROW(std::runtime_error, "Finale::Finale(): Cannot open BIGPLAN.CPS!");
    }

    auto pPlanetDuneInHouseColorSurface = LoadCPS_RW(pFileManager->openFile("MAPPLAN.CPS").get());
    if(pPlanetDuneInHouseColorSurface == nullptr) {
        THROW(std::runtime_error, "Finale::Finale(): Cannot open MAPPLAN.CPS!");
    }
    pPlanetDuneInHouseColorSurface = mapSurfaceColorRange(pPlanetDuneInHouseColorSurface.get(), houseToPaletteIndex[HOUSE_HARKONNEN], houseToPaletteIndex[house]);

    if(house == HOUSE_HARKONNEN || house == HOUSE_ATREIDES || house == HOUSE_ORDOS) {
        lizard = getChunkFromFile("LIZARD1.VOC");
        glass = getChunkFromFile("GLASS6.VOC");
        click = getChunkFromFile("CLICK.VOC");
        blaster = getChunkFromFile("BLASTER.VOC");
        blowup = getChunkFromFile("BLOWUP1.VOC");
    }

    const auto pIntroText = std::make_unique<IndexedTextFile>(pFileManager->openFile("INTRO." + _("LanguageFileExtension")).get());

    const Uint32 color = SDL2RGB(palette[houseToPaletteIndex[house]+1]);
    const Uint32 sardaukarColor = SDL2RGB(palette[PALCOLOR_SARDAUKAR+1]);

    switch(house) {
        case HOUSE_HARKONNEN: {
            startNewScene();

            addVideoEvent(new FadeInVideoEvent(pPalace1->getPicture(0).release(), 20, true));
            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(0).release(), 25, true));
            addVideoEvent(new WSAVideoEvent(pPalace1.get()));
            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(0).release(), 52, true));
            addVideoEvent(new WSAVideoEvent(pPalace1.get()));
            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(0).release(), 23, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_You_are_indeed_not_entirely),color,22,47,false,true,false));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_You_have_lied_to_us),color,70,60,true,true,false));
            addTrigger(new CutSceneMusicTrigger(0,MUSIC_FINALE_H));

            startNewScene();

            addVideoEvent(new WSAVideoEvent(pImperator.get()));
            addVideoEvent(new HoldPictureVideoEvent(pImperator->getPicture(pImperator->getNumFrames()-1).release(), 3, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_What_lies_What_are),sardaukarColor,2,100,false,true,false));

            startNewScene();

            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(0).release(), 50, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_Your_lies_of_loyalty),color,0,50,true,true,false));

            startNewScene();

            addVideoEvent(new HoldPictureVideoEvent(pImperatorShocked->getPicture(0).release(), 45, true));
            addVideoEvent(new HoldPictureVideoEvent(pImperatorShocked->getPicture(1).release(), 15, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_A_crime_for_which_you),color,2,38,true,false,false));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_with_your_life),color,42,100,false,false,false));

            startNewScene();

            addVideoEvent(new HoldPictureVideoEvent(pPalace2->getPicture(0).release(), 5, true));
            addVideoEvent(new WSAVideoEvent(pPalace2.get()));
            addVideoEvent(new HoldPictureVideoEvent(pPalace2->getPicture(pPalace2->getNumFrames()-1).release(), 15, true));
            addVideoEvent(new FadeOutVideoEvent(pPalace2->getPicture(pPalace2->getNumFrames()-1).release(), 20, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_NO_NO_NOOO),sardaukarColor,10,30,false,true,false));
            addTrigger(new CutSceneSoundTrigger(10,click.get()));
            addTrigger(new CutSceneSoundTrigger(15,blaster.get()));
            addTrigger(new CutSceneSoundTrigger(17,blowup.get()));
            addTrigger(new CutSceneSoundTrigger(38,click.get()));
            addTrigger(new CutSceneSoundTrigger(43,blaster.get()));
            addTrigger(new CutSceneSoundTrigger(45,blowup.get()));
            addTrigger(new CutSceneSoundTrigger(67,glass.get()));

        } break;

        case HOUSE_ATREIDES: {
            startNewScene();

            addVideoEvent(new FadeInVideoEvent(pPalace1->getPicture(0).release(), 20, true));
            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(0).release(), 50, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_Greetings_Emperor),color,15,48,false,true,false));
            addTrigger(new CutSceneMusicTrigger(0,MUSIC_FINALE_A));

            startNewScene();

            addVideoEvent(new WSAVideoEvent(pImperator.get()));
            addVideoEvent(new HoldPictureVideoEvent(pImperator->getPicture(pImperator->getNumFrames()-1).release(), 3, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_What_is_the_meaning),sardaukarColor,2,100,false,false,false));

            startNewScene();

            addVideoEvent(new WSAVideoEvent(pPalace1.get()));
            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(pPalace1->getNumFrames()-1).release(), 25, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_You_are_formally_charged),color,0,105,true,false,false));

            startNewScene();

            addVideoEvent(new HoldPictureVideoEvent(pImperatorShocked->getPicture(0).release(), 34, true));
            addVideoEvent(new HoldPictureVideoEvent(pImperatorShocked->getPicture(1).release(), 20, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_The_House_shall_determine),color,2,40,false,false,false));

            startNewScene();

            addVideoEvent(new HoldPictureVideoEvent(pPalace2->getPicture(0).release(), 15, true));
            addVideoEvent(new WSAVideoEvent(pPalace2.get()));
            addVideoEvent(new HoldPictureVideoEvent(pPalace2->getPicture(pPalace2->getNumFrames()-1).release(), 30, true));
            addVideoEvent(new FadeOutVideoEvent(pPalace2->getPicture(pPalace2->getNumFrames()-1).release(), 20, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_Until_then_you_shall_no),color,2,48,false,true,false));

        } break;

        case HOUSE_ORDOS: {
            startNewScene();

            addVideoEvent(new FadeInVideoEvent(pPalace1->getPicture(0).release(), 20, true));
            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(0).release(), 50, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_You_are_aware_Emperor),color,22,46,false,true,false));
            addTrigger(new CutSceneMusicTrigger(0,MUSIC_FINALE_O));

            startNewScene();

            addVideoEvent(new WSAVideoEvent(pImperator.get()));
            addVideoEvent(new HoldPictureVideoEvent(pImperator->getPicture(pImperator->getNumFrames()-1).release(), 3, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_What_games_What_are_you),sardaukarColor,2,100,false,true,false));

            startNewScene();

            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(0).release(), 40, true));
            addVideoEvent(new WSAVideoEvent(pPalace1.get()));
            addVideoEvent(new HoldPictureVideoEvent(pPalace1->getPicture(pPalace1->getNumFrames()-1).release(), 65, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_I_am_referring_to_your_game),color,2,35,false,true,false));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_We_were_your_pawns_and_Dune),color,40,45,true,true,false));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_We_have_decided_to_take),color,88,105,true,false,false));
            addTrigger(new CutSceneSoundTrigger(42,lizard.get()));
            addTrigger(new CutSceneSoundTrigger(62,lizard.get()));

            startNewScene();

            addVideoEvent(new HoldPictureVideoEvent(pImperatorShocked->getPicture(0).release(), 29, true));
            addVideoEvent(new HoldPictureVideoEvent(pImperatorShocked->getPicture(1).release(), 20, true));
            addTextEvent(new TextEvent(pIntroText->getString(FinaleText_You_are_to_be_our_pawn),color,2,47,false,true,false));

            startNewScene();

            addVideoEvent(new WSAVideoEvent(pPalace2.get()));
            addVideoEvent(new HoldPictureVideoEvent(pPalace2->getPicture(pPalace2->getNumFrames()-1).release(), 15, true));
            addVideoEvent(new FadeOutVideoEvent(pPalace2->getPicture(pPalace2->getNumFrames()-1).release(), 20, true));

        } break;

        default: {
            // Nothing
        } break;
    }

    addVideoEvent(new FadeInVideoEvent(pPlanetDuneNormalSurface.get(), 20, false));
    addVideoEvent(new HoldPictureVideoEvent(pPlanetDuneNormalSurface.get(), 10, false));
    addVideoEvent(new CrossBlendVideoEvent(pPlanetDuneNormalSurface.get(), pPlanetDuneInHouseColorSurface.get(), false));
    addVideoEvent(new HoldPictureVideoEvent(pPlanetDuneInHouseColorSurface.get(), 50, false));
    addVideoEvent(new FadeOutVideoEvent(pPlanetDuneInHouseColorSurface.get(), 20, false));
    addVideoEvent(new HoldPictureVideoEvent(nullptr, 10, false));
}

Finale::~Finale() = default;

