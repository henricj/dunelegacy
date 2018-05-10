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

#include <CutScenes/Intro.h>
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
#include <FileClasses/Vocfile.h>
#include <FileClasses/IndexedTextFile.h>

#include <misc/sound_util.h>

#include <string>

const char* Intro::VoiceFileNames[Voice_NUM_ENTRIES] =   {  "BLDING.VOC",
                                                            "DYNASTY.VOC",
                                                            "PLANET.VOC",
                                                            "KNOWN.VOC",
                                                            "SANDLAND.VOC",
                                                            "HOME.VOC",
                                                            "SPICE.VOC",
                                                            "MELANGE.VOC",
                                                            "SPICE2.VOC",
                                                            "CONTROLS.VOC",
                                                            "EMPIRE.VOC",
                                                            "WHOEVER.VOC",
                                                            "CONTROL2.VOC",
                                                            "CONTROL3.VOC",
                                                            "KING.VOC",
                                                            "PROPOSED.VOC",
                                                            "EACHHOME.VOC",
                                                            "EHOUSE.VOC",
                                                            "EPRODUCE.VOC",
                                                            "EMOST.VOC",
                                                            "ECONTROL.VOC",
                                                            "ENOSET.VOC",
                                                            "ETERRIT.VOC",
                                                            "EANDNO.VOC",
                                                            "ERULES.VOC",
                                                            "VAST.VOC",
                                                            "ARRIVED.VOC",
                                                            "ANDNOW.VOC",
                                                            "3HOUSES.VOC",
                                                            "CONTROL4.VOC",
                                                            "OFDUNE.VOC",
                                                            "NOBLE.VOC",
                                                            "INSID.VOC",
                                                            "ORD.VOC",
                                                            "EVIL.VOC",
                                                            "HARK.VOC",
                                                            "HOUSE2.VOC",
                                                            "PREVAIL.VOC",
                                                            "YOUR.VOC",
                                                            "BATTLE.VOC",
                                                            "BEGINS.VOC",
                                                            "NOW.VOC"
                                                        };

Intro::Intro() {

    pDuneText = create_wsafile("INTRO1.WSA");
    pPlanet = create_wsafile("INTRO2.WSA");
    pSandstorm = create_wsafile("INTRO3.WSA");
    pHarvesters = create_wsafile("INTRO9.WSA");
    pPalace = create_wsafile("INTRO10.WSA");
    pImperator = create_wsafile("INTRO11.WSA");
    pStarport = create_wsafile("INTRO4.WSA");
    pAtreides = create_wsafile("INTRO6.WSA");
    pOrdos = create_wsafile("INTRO7A.WSA", "INTRO7B.WSA");
    pHarkonnen = create_wsafile("INTRO8A.WSA", "INTRO8B.WSA", "INTRO8C.WSA");
    pDestroyedTank = create_wsafile("INTRO5.WSA");

    IndexedTextFile intro_text{ pFileManager->openFile("INTRO." + _("LanguageFileExtension")).get() };


    wind = getChunkFromFile("WIND2BP.VOC");
    carryallLanding = getChunkFromFile("CLANK.VOC");
    harvester = getChunkFromFile("BRAKES2P.VOC");

    {
        auto singleGunshot = getChunkFromFile("GUNSHOT.VOC");
        auto fourGunshots = concat4Chunks(singleGunshot.get(), singleGunshot.get(), singleGunshot.get(), singleGunshot.get());
        auto thirteenGunshots = concat4Chunks(fourGunshots.get(), fourGunshots.get(), fourGunshots.get(), singleGunshot.get());
        gunshot = concat3Chunks(thirteenGunshots.get(), singleGunshot.get(), singleGunshot.get());
    }

    glass = getChunkFromFile("GLASS.VOC");
    missle = getChunkFromFile("MISSLE8.VOC");
    blaster = getChunkFromFile("BLASTER.VOC");
    blowup1 = getChunkFromFile("BLOWUP1.VOC");
    blowup2 = getChunkFromFile("BLOWUP2.VOC");


    bool bEnableVoice = (settings.general.language == "en");
    if(bEnableVoice == true) {
        // Load english voice
        for(int i=0;i<Voice_NUM_ENTRIES;i++) {
            voice[i] = getChunkFromFile(VoiceFileNames[i]);
        }
    } else {
        for(int i=0;i<Voice_NUM_ENTRIES;i++) {
            voice[i] = nullptr;
        }
    }

    Uint32 color = SDL2RGB(palette[PALCOLOR_HARKONNEN+1]);
    Uint32 sardaukarColor = SDL2RGB(palette[PALCOLOR_SARDAUKAR+1]);

    startNewScene();

    addVideoEvent(std::make_unique<WSAVideoEvent>(pDuneText.get(), false));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pDuneText->getPicture(pDuneText->getNumFrames() - 1).get(), 30, false));
    addVideoEvent(std::make_unique<FadeOutVideoEvent>(pDuneText->getPicture(pDuneText->getNumFrames() - 1).get(), 20, false));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_The_Battle_for_Arrakis),color,48,40,true,true,true));
    addTextEvent(std::make_unique<TextEvent>("The remake is called Dune Legacy!",color,48,40,true));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(52,voice[Voice_The_building].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(64,voice[Voice_of_a_Dynasty].get()));

    startNewScene();

    addVideoEvent(std::make_unique<FadeInVideoEvent>(pPlanet->getPicture(0).get(), 20));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pPlanet.get()));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pPlanet->getPicture(pPlanet->getNumFrames() - 1).get(), 20));
    addVideoEvent(std::make_unique<FadeOutVideoEvent>(pPlanet->getPicture(pPlanet->getNumFrames() - 1).get(), 20));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_The_planet_Arrakis),color,20,60,true,true,false));
    addTrigger(std::make_unique<CutSceneMusicTrigger>(25,MUSIC_INTRO));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(25,voice[Voice_The_Planet_Arrakis].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(38,voice[Voice_Known_As_Dune].get()));

    startNewScene();

    addVideoEvent(std::make_unique<FadeInVideoEvent>(pSandstorm->getPicture(0).get(), 20));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pSandstorm.get()));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pSandstorm->getPicture(pSandstorm->getNumFrames() - 1).get(), 50));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Land_of_sand),color,20,40,true,true));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Home_of_the_Spice_Melange),color,61,45,true,true,false));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(25,wind.get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(15,voice[Voice_Land_of_sand].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(70,voice[Voice_Home].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(78,voice[Voice_of_the_spice].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(92,voice[Voice_Melange].get()));

    startNewScene();

    addVideoEvent(std::make_unique<CrossBlendVideoEvent>(pSandstorm->getPicture(pSandstorm->getNumFrames() - 1).get(), pHarvesters->getPicture(0).get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pHarvesters.get()));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pHarvesters->getPicture(pHarvesters->getNumFrames() - 1).get(), 22));
    addVideoEvent(std::make_unique<FadeOutVideoEvent>(pHarvesters->getPicture(pHarvesters->getNumFrames() - 1).get(), 20));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Spice_controls_the_Empire),color,25,40,true,true,false));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Whoever_controls_Dune),color,66,55,true,true,false));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(45,carryallLanding.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(79,harvester.get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(40,voice[Voice_The_spice].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(47,voice[Voice_controls].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(53,voice[Voice_the_Empire].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(90,voice[Voice_Whoever].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(96,voice[Voice_controls_dune].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(108,voice[Voice_controls_the_spice].get()));

    startNewScene();

    addVideoEvent(std::make_unique<FadeInVideoEvent>(pPalace->getPicture(0).get(), 20));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pPalace->getPicture(0).get(), 50));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_The_Emperor_has_proposed),color,20,48,true,true,false));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(22,voice[Voice_The_Emperor].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(31,voice[Voice_has_proposed].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(49,voice[Voice_to_each_of_the_houses].get()));

    startNewScene();

    addVideoEvent(std::make_unique<WSAVideoEvent>(pImperator.get()));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pImperator->getPicture(pImperator->getNumFrames() - 1).get(), 30));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pImperator.get()));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pImperator->getPicture(pImperator->getNumFrames() - 1).get(), 15));
    addVideoEvent(std::make_unique<FadeOutVideoEvent>(pImperator->getPicture(pImperator->getNumFrames() - 1).get(), 20));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_The_House_that_produces),sardaukarColor,0,52,true,true,false));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_There_are_no_set_territories),sardaukarColor,68,30,true,true,false));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_And_no_rules_of_engagement),sardaukarColor,99,30,true,true,false));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(8,voice[Voice_The_House].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(14,voice[Voice_that_produces].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(20,voice[Voice_the_most_spice].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(30,voice[Voice_will_control_dune].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(81,voice[Voice_There_are_no_set].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(89,voice[Voice_territories].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(98,voice[Voice_and_no].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(102,voice[Voice_rules_of_engagment].get()));

    startNewScene();

    addVideoEvent(std::make_unique<FadeInVideoEvent>(pStarport->getPicture(0).get(), 20));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pStarport.get()));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pStarport->getPicture(pStarport->getNumFrames() - 1).get(), 20));
    addVideoEvent(std::make_unique<FadeOutVideoEvent>(pStarport->getPicture(pStarport->getNumFrames() - 1).get(), 20));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Vast_armies_have_arrived),color,25,60,true,true,false));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(57,carryallLanding.get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(30,voice[Voice_Vast_armies].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(37,voice[Voice_have_arrived].get()));

    startNewScene();

    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(nullptr, 80));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Now_three_houses_fight),color,0,80,true,false,true));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(10,voice[Voice_Now].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(17,voice[Voice_three_Houses_fight].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(34,voice[Voice_for_control].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(46,voice[Voice_of_Dune].get()));

    startNewScene();

    addVideoEvent(std::make_unique<WSAVideoEvent>(pAtreides.get()));
    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(pAtreides->getPicture(pAtreides->getNumFrames()-1).get(), 8));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_The_noble_Atreides),color,25,58,true,true,false));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(21,gunshot.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(31,glass.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(32,glass.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(33,glass.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(51,gunshot.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(61,glass.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(62,glass.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(63,glass.get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(36,voice[Voice_The_noble_Atreides].get()));


    startNewScene();

    addVideoEvent(std::make_unique<WSAVideoEvent>(pOrdos.get()));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_The_insidious_Ordos),color,-2,47,true,true,false));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(3,missle.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(8,missle.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(28,missle.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(38,missle.get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(12,voice[Voice_The_insidious].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(20,voice[Voice_Ordos].get()));

    startNewScene();

    addVideoEvent(std::make_unique<WSAVideoEvent>(pHarkonnen.get()));
    addVideoEvent(std::make_unique<FadeOutVideoEvent>(pHarkonnen->getPicture(pHarkonnen->getNumFrames() - 1).get(), 15, true, true));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_And_the_evil_Harkonnen),color,-2,45,true,true,false));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(0,gunshot.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(5,blaster.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(7,blaster.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(17,gunshot.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(21,blaster.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(28,gunshot.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(37,gunshot.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(50,blowup1.get()));
    addTrigger(std::make_unique<CutSceneSoundTrigger>(60,blowup2.get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(4,voice[Voice_And_the].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(10,voice[Voice_evil_Harkonnen].get()));

    startNewScene();

    addVideoEvent(std::make_unique<FadeInVideoEvent>(pDestroyedTank->getPicture(0).get(), 15, true, true));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<WSAVideoEvent>(pDestroyedTank.get()));
    addVideoEvent(std::make_unique<FadeOutVideoEvent>(pDestroyedTank->getPicture(pDestroyedTank->getNumFrames() - 1).get(), 15));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Only_one_House_will_prevail),color,18,35,true,true,false));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(21,voice[Voice_Only_one_house].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(30,voice[Voice_will_prevail].get()));

    startNewScene();

    addVideoEvent(std::make_unique<HoldPictureVideoEvent>(nullptr, 184));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_Your_battle_for_Dune_begins),color,20,45,true,false,true));
    addTextEvent(std::make_unique<TextEvent>(intro_text.getString(IntroText_NOW),color,68,83,false,true,true));
//    addTextEvent(std::make_unique<TextEvent>("",COLOR_BLACK,115,10,false,false,true));    // padding to give music time to complete
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(30,voice[Voice_Your].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(36,voice[Voice_battle_for_Dune].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(48,voice[Voice_begins].get()));
    if(bEnableVoice) addTrigger(std::make_unique<CutSceneSoundTrigger>(68,voice[Voice_Now_Now].get()));
}

Intro::~Intro() = default;

