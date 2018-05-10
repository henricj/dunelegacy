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

#include <CutScenes/Meanwhile.h>
#include <CutScenes/WSAVideoEvent.h>
#include <CutScenes/HoldPictureVideoEvent.h>
#include <CutScenes/FadeOutVideoEvent.h>
#include <CutScenes/FadeInVideoEvent.h>
#include <CutScenes/TextEvent.h>
#include <CutScenes/CutSceneMusicTrigger.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/Pakfile.h>
#include <FileClasses/Wsafile.h>
#include <FileClasses/IndexedTextFile.h>

#include <misc/exceptions.h>

#include <string>

Meanwhile::Meanwhile(int house, bool firstMeanwhile) {

    if(house != HOUSE_HARKONNEN && house != HOUSE_ATREIDES && house != HOUSE_ORDOS) {
        THROW(std::invalid_argument, "Invalid house number %d!", house);
    }

    pMeanwhile = create_wsafile("MEANWHIL.WSA");
    pImperator = create_wsafile("EFINALA.WSA");

    IndexedTextFile dune_text{ pFileManager->openFile("DUNE." + _("LanguageFileExtension")).get() };

    int textBaseIndex = MeanwhileText_Base + ((house+2)%3) * MeanwhileText_NumTextsPerHouse;

    if(dune_text.getNumStrings() == 335) {
        // Dune II 1.0 has 2 ranks less
        textBaseIndex -= 2;
    }

    int houseOfVisitor = (house+2)%3;
    Uint32 color = SDL2RGB(palette[houseToPaletteIndex[house]+1]);
    Uint32 sardaukarColor = SDL2RGB(palette[PALCOLOR_SARDAUKAR+1]);
    Uint32 visitorColor = SDL2RGB(palette[houseToPaletteIndex[houseOfVisitor]+1]);

    if(firstMeanwhile == true) {
        // Meanwhile after level 4
        static const int meanwhileFrame[] = { 1, 2, 0};

        startNewScene();

        addVideoEvent(std::make_unique<HoldPictureVideoEvent>(nullptr, 45));
        addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_At_the_Emperor_s_Palace),color,0,42,true,true,true));
        addTrigger(std::make_unique<CutSceneMusicTrigger>(0,MUSIC_MEANWHILE));

        startNewScene();

        addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pMeanwhile->getPicture(meanwhileFrame[house]).get(), 75));
        addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_You_of_all_people),sardaukarColor,0,45,true,true,false));
        addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_Yes_your_excellency_I),visitorColor,45,30,true,false,false));

        startNewScene();

        addVideoEvent(std::make_unique<WSAVideoEvent>(pImperator.get()));
        addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pImperator->getPicture(pImperator->getNumFrames()-1).get(), 3));
        addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_You_let_the),sardaukarColor,3,100,false,false,false));

        startNewScene();
        addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pMeanwhile->getPicture(meanwhileFrame[house]).get(), 75));
        addVideoEvent(std::make_unique<FadeOutVideoEvent>(pMeanwhile->getPicture(meanwhileFrame[house]).get(), 20));
        addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_I_did_not_let),visitorColor,0,35,true,false,false));
        addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_I_will_not_allow),sardaukarColor,37,38,false,true,false));

    } else {
        // Meanwhile after level 8
        static const int meanwhileFrame[] = { 3, 5, 4};

        startNewScene();

        addVideoEvent(std::make_unique<HoldPictureVideoEvent>(nullptr, 45));
        addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_At_the_Emperor_s_Palace_on_Dune),color,0,42,true,true,true));
        addTrigger(std::make_unique<CutSceneMusicTrigger>(0,MUSIC_MEANWHILE));

        if(house == HOUSE_ATREIDES) {
            startNewScene();

            addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pMeanwhile->getPicture(meanwhileFrame[house]).get(), 130));
            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_Fools),sardaukarColor,0,45,true,false,false));
            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_And_still_you_fail),sardaukarColor,50,45,false,false,false));
            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_But_excell),visitorColor,100,30,true,false,false));

            startNewScene();

            addVideoEvent(std::make_unique<WSAVideoEvent>(pImperator.get()));
            addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pImperator->getPicture(pImperator->getNumFrames()-1).get(), 3));
            addVideoEvent(std::make_unique<FadeOutVideoEvent>(pImperator->getPicture(pImperator->getNumFrames()-1).get(), 20));
            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_Enough_Together_we_must),sardaukarColor,3,42,false,true,false));
        } else {
            startNewScene();

            addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pMeanwhile->getPicture(meanwhileFrame[house]).get(), 80));
            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_The_Ordos_were_not_supposed),sardaukarColor,0,45,true,true,false));
            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_Your_highness),visitorColor,46,35,true,false,false));

            startNewScene();

            addVideoEvent(std::make_unique<WSAVideoEvent>(pImperator.get()));
            addVideoEvent(std::make_unique<WSAVideoEvent>(pImperator.get()));
            addVideoEvent(std::make_unique<FadeOutVideoEvent>(pImperator->getPicture(pImperator->getNumFrames()-1).get(), 20));

            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_No_more_explanations),sardaukarColor,3, (house == HOUSE_ORDOS) ? 21 : 11,false,false,false));
            addTextEvent(std::make_unique<TextEvent>(dune_text.getString(textBaseIndex+MeanwhileText_Only_together_will_we),sardaukarColor,(house == HOUSE_ORDOS) ? 28 : 18,(house == HOUSE_ORDOS) ? 39 : 49,false,true,false));
        }
    }
}

Meanwhile::~Meanwhile() = default;

