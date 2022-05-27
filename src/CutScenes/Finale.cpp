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

#include <CutScenes/CrossBlendVideoEvent.h>
#include <CutScenes/CutSceneMusicTrigger.h>
#include <CutScenes/CutSceneSoundTrigger.h>
#include <CutScenes/FadeInVideoEvent.h>
#include <CutScenes/FadeOutVideoEvent.h>
#include <CutScenes/Finale.h>
#include <CutScenes/HoldPictureVideoEvent.h>
#include <CutScenes/TextEvent.h>
#include <CutScenes/WSAVideoEvent.h>

#include <FileClasses/Cpsfile.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/IndexedTextFile.h>
#include <FileClasses/Pakfile.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/Vocfile.h>
#include <FileClasses/Wsafile.h>

#include <misc/draw_util.h>
#include <misc/sound_util.h>

Finale::Finale(HOUSETYPE house) {

    const auto* const file_manager = dune::globals::pFileManager.get();
    ;
    switch (house) {
        case HOUSETYPE::HOUSE_HARKONNEN: {
            pPalace1 = std::make_unique<Wsafile>(file_manager->openFile("HFINALA.WSA").get());
            pPalace2 = std::make_unique<Wsafile>(file_manager->openFile("HFINALB.WSA").get(),
                                                 file_manager->openFile("HFINALC.WSA").get());
        } break;

        case HOUSETYPE::HOUSE_ATREIDES: {
            pPalace1 = std::make_unique<Wsafile>(file_manager->openFile("AFINALA.WSA").get());
            pPalace2 = std::make_unique<Wsafile>(file_manager->openFile("AFINALB.WSA").get());
        } break;

        case HOUSETYPE::HOUSE_ORDOS: {
            pPalace1 = std::make_unique<Wsafile>(file_manager->openFile("OFINALA.WSA").get(),
                                                 file_manager->openFile("OFINALB.WSA").get(),
                                                 file_manager->openFile("OFINALC.WSA").get());
            pPalace2 = std::make_unique<Wsafile>(file_manager->openFile("OFINALD.WSA").get());
        } break;

        default: {
            // Nothing
        } break;
    }

    if (house == HOUSETYPE::HOUSE_HARKONNEN || house == HOUSETYPE::HOUSE_ATREIDES || house == HOUSETYPE::HOUSE_ORDOS) {
        pImperator        = std::make_unique<Wsafile>(file_manager->openFile("EFINALA.WSA").get());
        pImperatorShocked = std::make_unique<Wsafile>(file_manager->openFile("EFINALB.WSA").get());
    }

    const auto pPlanetDuneNormalSurface = LoadCPS_RW(file_manager->openFile("BIGPLAN.CPS").get());
    if (pPlanetDuneNormalSurface == nullptr) {
        THROW(std::runtime_error, "Finale::Finale(): Cannot open BIGPLAN.CPS!");
    }

    auto pPlanetDuneInHouseColorSurface = LoadCPS_RW(file_manager->openFile("MAPPLAN.CPS").get());
    if (pPlanetDuneInHouseColorSurface == nullptr) {
        THROW(std::runtime_error, "Finale::Finale(): Cannot open MAPPLAN.CPS!");
    }
    pPlanetDuneInHouseColorSurface =
        mapSurfaceColorRange(pPlanetDuneInHouseColorSurface.get(),
                             dune::globals::houseToPaletteIndex[static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)],
                             dune::globals::houseToPaletteIndex[static_cast<int>(house)]);

    if (house == HOUSETYPE::HOUSE_HARKONNEN || house == HOUSETYPE::HOUSE_ATREIDES || house == HOUSETYPE::HOUSE_ORDOS) {
        lizard  = getChunkFromFile("LIZARD1.VOC");
        glass   = getChunkFromFile("GLASS6.VOC");
        click   = getChunkFromFile("CLICK.VOC");
        blaster = getChunkFromFile("BLASTER.VOC");
        blowup  = getChunkFromFile("BLOWUP1.VOC");
    }

    const auto pIntroText =
        std::make_unique<IndexedTextFile>(file_manager->openFile("INTRO." + _("LanguageFileExtension")).get());

    const auto& palette = dune::globals::palette;

    const Uint32 color          = SDL2RGB(palette[dune::globals::houseToPaletteIndex[static_cast<int>(house)] + 1]);
    const Uint32 sardaukarColor = SDL2RGB(palette[PALCOLOR_SARDAUKAR + 1]);

    switch (house) {
        case HOUSETYPE::HOUSE_HARKONNEN: {
            startNewScene();

            addVideoEvent<FadeInVideoEvent>(pPalace1->getPicture(0).get(), 20);
            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(0).get(), 25);
            addVideoEvent<WSAVideoEvent>(pPalace1.get());
            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(0).get(), 52);
            addVideoEvent<WSAVideoEvent>(pPalace1.get());
            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(0).get(), 23);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_You_are_indeed_not_entirely), color, 22, 47, false,
                                    true, false);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_You_have_lied_to_us), color, 70, 60, true, true,
                                    false);
            addTrigger<CutSceneMusicTrigger>(0, MUSIC_FINALE_H);

            startNewScene();

            addVideoEvent<WSAVideoEvent>(pImperator.get());
            addVideoEvent(std::make_unique<HoldPictureVideoEvent>(
                pImperator->getPicture(pImperator->getNumFrames() - 1).get(), 3));
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_What_lies_What_are), sardaukarColor, 2, 100, false,
                                    true, false);

            startNewScene();

            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(0).get(), 50);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_Your_lies_of_loyalty), color, 0, 50, true, true,
                                    false);

            startNewScene();

            addVideoEvent<HoldPictureVideoEvent>(pImperatorShocked->getPicture(0).get(), 45);
            addVideoEvent<HoldPictureVideoEvent>(pImperatorShocked->getPicture(1).get(), 15);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_A_crime_for_which_you), color, 2, 38, true, false,
                                    false);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_with_your_life), color, 42, 100, false, false,
                                    false);

            startNewScene();

            addVideoEvent<HoldPictureVideoEvent>(pPalace2->getPicture(0).get(), 5);
            addVideoEvent<WSAVideoEvent>(pPalace2.get());
            addVideoEvent<HoldPictureVideoEvent>(pPalace2->getPicture(pPalace2->getNumFrames() - 1).get(), 15);
            addVideoEvent<FadeOutVideoEvent>(pPalace2->getPicture(pPalace2->getNumFrames() - 1).get(), 20);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_NO_NO_NOOO), sardaukarColor, 10, 30, false, true,
                                    false);
            addTrigger<CutSceneSoundTrigger>(10, click.get());
            addTrigger<CutSceneSoundTrigger>(15, blaster.get());
            addTrigger<CutSceneSoundTrigger>(17, blowup.get());
            addTrigger<CutSceneSoundTrigger>(38, click.get());
            addTrigger<CutSceneSoundTrigger>(43, blaster.get());
            addTrigger<CutSceneSoundTrigger>(45, blowup.get());
            addTrigger<CutSceneSoundTrigger>(67, glass.get());

        } break;

        case HOUSETYPE::HOUSE_ATREIDES: {
            startNewScene();

            addVideoEvent<FadeInVideoEvent>(pPalace1->getPicture(0).get(), 20);
            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(0).get(), 50);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_Greetings_Emperor), color, 15, 48, false, true,
                                    false);
            addTrigger<CutSceneMusicTrigger>(0, MUSIC_FINALE_A);

            startNewScene();

            addVideoEvent<WSAVideoEvent>(pImperator.get());
            addVideoEvent<HoldPictureVideoEvent>(pImperator->getPicture(pImperator->getNumFrames() - 1).get(), 3);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_What_is_the_meaning), sardaukarColor, 2, 100,
                                    false, false, false);

            startNewScene();

            addVideoEvent<WSAVideoEvent>(pPalace1.get());
            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(pPalace1->getNumFrames() - 1).get(), 25);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_You_are_formally_charged), color, 0, 105, true,
                                    false, false);

            startNewScene();

            addVideoEvent<HoldPictureVideoEvent>(pImperatorShocked->getPicture(0).get(), 34);
            addVideoEvent<HoldPictureVideoEvent>(pImperatorShocked->getPicture(1).get(), 20);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_The_House_shall_determine), color, 2, 40, false,
                                    false, false);

            startNewScene();

            addVideoEvent<HoldPictureVideoEvent>(pPalace2->getPicture(0).get(), 15);
            addVideoEvent<WSAVideoEvent>(pPalace2.get());
            addVideoEvent<HoldPictureVideoEvent>(pPalace2->getPicture(pPalace2->getNumFrames() - 1).get(), 30);
            addVideoEvent<FadeOutVideoEvent>(pPalace2->getPicture(pPalace2->getNumFrames() - 1).get(), 20);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_Until_then_you_shall_no), color, 2, 48, false,
                                    true, false);

        } break;

        case HOUSETYPE::HOUSE_ORDOS: {
            startNewScene();

            addVideoEvent<FadeInVideoEvent>(pPalace1->getPicture(0).get(), 20);
            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(0).get(), 50);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_You_are_aware_Emperor), color, 22, 46, false, true,
                                    false);
            addTrigger<CutSceneMusicTrigger>(0, MUSIC_FINALE_O);

            startNewScene();

            addVideoEvent<WSAVideoEvent>(pImperator.get());
            addVideoEvent<HoldPictureVideoEvent>(pImperator->getPicture(pImperator->getNumFrames() - 1).get(), 3);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_What_games_What_are_you), sardaukarColor, 2, 100,
                                    false, true, false);

            startNewScene();

            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(0).get(), 40);
            addVideoEvent<WSAVideoEvent>(pPalace1.get());
            addVideoEvent<HoldPictureVideoEvent>(pPalace1->getPicture(pPalace1->getNumFrames() - 1).get(), 65);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_I_am_referring_to_your_game), color, 2, 35, false,
                                    true, false);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_We_were_your_pawns_and_Dune), color, 40, 45, true,
                                    true, false);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_We_have_decided_to_take), color, 88, 105, true,
                                    false, false);
            addTrigger<CutSceneSoundTrigger>(42, lizard.get());
            addTrigger<CutSceneSoundTrigger>(62, lizard.get());

            startNewScene();

            addVideoEvent<HoldPictureVideoEvent>(pImperatorShocked->getPicture(0).get(), 29);
            addVideoEvent<HoldPictureVideoEvent>(pImperatorShocked->getPicture(1).get(), 20);
            addTextEvent<TextEvent>(pIntroText->getString(FinaleText_You_are_to_be_our_pawn), color, 2, 47, false, true,
                                    false);

            startNewScene();

            addVideoEvent<WSAVideoEvent>(pPalace2.get());
            addVideoEvent<HoldPictureVideoEvent>(pPalace2->getPicture(pPalace2->getNumFrames() - 1).get(), 15);
            addVideoEvent<FadeOutVideoEvent>(pPalace2->getPicture(pPalace2->getNumFrames() - 1).get(), 20);

        } break;

        default: {
            // Nothing
        } break;
    }

    addVideoEvent<FadeInVideoEvent>(pPlanetDuneNormalSurface.get(), 20);
    addVideoEvent<HoldPictureVideoEvent>(pPlanetDuneNormalSurface.get(), 10);
    addVideoEvent<CrossBlendVideoEvent>(pPlanetDuneNormalSurface.get(), pPlanetDuneInHouseColorSurface.get());
    addVideoEvent<HoldPictureVideoEvent>(pPlanetDuneInHouseColorSurface.get(), 50);
    addVideoEvent<FadeOutVideoEvent>(pPlanetDuneInHouseColorSurface.get(), 20);
    addVideoEvent<HoldPictureVideoEvent>(nullptr, 10);
}

Finale::~Finale() = default;
