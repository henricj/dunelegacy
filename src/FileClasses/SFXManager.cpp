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

#include <FileClasses/SFXManager.h>

#include <globals.h>

#include "FileClasses/adl/sound_adlib.h"
#include <FileClasses/FileManager.h>
#include <FileClasses/Vocfile.h>

#include <misc/exceptions.h>
#include <misc/sound_util.h>

#include <adlib.h>

#include <algorithm>
#include <utility>

// Not used:
// - EXCANNON.VOC (same as EXSMALL.VOC)
// - DROPEQ2P.VOC
// - POPPA.VOC

SFXManager::SFXManager() {
    const auto& language = dune::globals::settings.general.language;

    // load voice and language specific sounds
    if (language == "de") {
        loadNonEnglishVoice("G");
    } else if (language == "fr") {
        loadNonEnglishVoice("F");
    } else {
        loadEnglishVoice();
    }

    for (int i = 0; i < static_cast<int>(Sound_enum::NUM_SOUNDCHUNK); i++) {
        if (soundChunk[i] == nullptr) {
            THROW(std::runtime_error, "Not all sounds could be loaded: soundChunk[%d] == nullptr!", i);
        }
    }
}

SFXManager::~SFXManager() = default;

Mix_Chunk* SFXManager::getVoice(Voice_enum id, HOUSETYPE house) const {
    const auto& language = dune::globals::settings.general.language;

    if (language == "de" || language == "fr") {
        return getNonEnglishVoice(id, house);
    }
    return getEnglishVoice(id, house);
}

Mix_Chunk* SFXManager::getSound(Sound_enum id) const {
    const auto sound_index = static_cast<int>(id);

    if (sound_index < 0 || std::cmp_greater_equal(sound_index, soundChunk.size()))
        return nullptr;

    return soundChunk[sound_index].get();
}

sdl2::mix_chunk_ptr
SFXManager::loadMixFromADL(const std::string& adlFile, int index, [[maybe_unused]] int volume) const {

    const auto rwop = dune::globals::pFileManager->openFile(adlFile);

    SoundAdlibPC player{rwop.get(), AUDIO_FREQUENCY};

    auto chunk = player.getSubsong(index);

    return chunk;
}

void SFXManager::loadEnglishVoice() {
    lngVoice.clear();
    lngVoice.resize(static_cast<int>(Voice_enum::NUM_VOICE) * NUM_HOUSES);

    // now we can load
    for (auto house = 0; house < NUM_HOUSES; house++) {
        sdl2::mix_chunk_ptr HouseNameChunk;

        std::string HouseString;
        const int VoiceNum = house;
        switch (static_cast<HOUSETYPE>(house)) {
            case HOUSETYPE::HOUSE_HARKONNEN:
                HouseString    = "H";
                HouseNameChunk = getChunkFromFile(HouseString + "HARK.VOC");
                break;
            case HOUSETYPE::HOUSE_ATREIDES:
                HouseString    = "A";
                HouseNameChunk = getChunkFromFile(HouseString + "ATRE.VOC");
                break;
            case HOUSETYPE::HOUSE_ORDOS:
                HouseString    = "O";
                HouseNameChunk = getChunkFromFile(HouseString + "ORDOS.VOC");
                break;
            case HOUSETYPE::HOUSE_FREMEN:
                HouseString    = "A";
                HouseNameChunk = getChunkFromFile(HouseString + "FREMEN.VOC");
                break;
            case HOUSETYPE::HOUSE_SARDAUKAR:
                HouseString    = "H";
                HouseNameChunk = getChunkFromFile(HouseString + "SARD.VOC");
                break;
            case HOUSETYPE::HOUSE_MERCENARY:
                HouseString    = "O";
                HouseNameChunk = getChunkFromFile(HouseString + "MERC.VOC");
                break;
            default: break;
        }

        { // Scope
            // "... Harvester deployed", "... Unit deployed" and "... Unit launched"
            auto Harvester = getChunkFromFile(HouseString + "HARVEST.VOC");
            auto Unit      = getChunkFromFile(HouseString + "UNIT.VOC");
            auto Deployed  = getChunkFromFile(HouseString + "DEPLOY.VOC");
            auto Launched  = getChunkFromFile(HouseString + "LAUNCH.VOC");
            lngVoice[static_cast<int>(Voice_enum::HarvesterDeployed) * NUM_HOUSES + VoiceNum] =
                concat3Chunks(HouseNameChunk.get(), Harvester.get(), Deployed.get());
            lngVoice[static_cast<int>(Voice_enum::UnitDeployed) * NUM_HOUSES + VoiceNum] =
                concat3Chunks(HouseNameChunk.get(), Unit.get(), Deployed.get());
            lngVoice[static_cast<int>(Voice_enum::UnitLaunched) * NUM_HOUSES + VoiceNum] =
                concat3Chunks(HouseNameChunk.get(), Unit.get(), Launched.get());
        }

        // "Construction complete"
        lngVoice[static_cast<int>(Voice_enum::ConstructionComplete) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile(HouseString + "CONST.VOC");

        { // Scope
          // "Vehicle repaired"
            auto Vehicle  = getChunkFromFile(HouseString + "VEHICLE.VOC");
            auto Repaired = getChunkFromFile(HouseString + "REPAIR.VOC");
            lngVoice[static_cast<int>(Voice_enum::VehicleRepaired) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(Vehicle.get(), Repaired.get());
        }

        { // Scope
          // "Frigate has arrived"
            auto FrigateChunk    = getChunkFromFile(HouseString + "FRIGATE.VOC");
            auto HasArrivedChunk = getChunkFromFile(HouseString + "ARRIVE.VOC");
            lngVoice[static_cast<int>(Voice_enum::FrigateHasArrived) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(FrigateChunk.get(), HasArrivedChunk.get());
        }

        // "Your mission is complete"
        lngVoice[static_cast<int>(Voice_enum::YourMissionIsComplete) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile(HouseString + "WIN.VOC");

        // "You have failed your mission"
        lngVoice[static_cast<int>(Voice_enum::YouHaveFailedYourMission) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile(HouseString + "LOSE.VOC");

        { // Scope
          // "Radar activated"/"Radar deactivated"
            auto RadarChunk            = getChunkFromFile(HouseString + "RADAR.VOC");
            auto RadarActivatedChunk   = getChunkFromFile(HouseString + "ON.VOC");
            auto RadarDeactivatedChunk = getChunkFromFile(HouseString + "OFF.VOC");
            lngVoice[static_cast<int>(Voice_enum::RadarActivated) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(RadarChunk.get(), RadarActivatedChunk.get());
            lngVoice[static_cast<int>(Voice_enum::RadarDeactivated) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(RadarChunk.get(), RadarDeactivatedChunk.get());
        }

        { // Scope
          // "Bloom located"
            auto Bloom   = getChunkFromFile(HouseString + "BLOOM.VOC");
            auto Located = getChunkFromFile(HouseString + "LOCATED.VOC");
            lngVoice[static_cast<int>(Voice_enum::BloomLocated) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(Bloom.get(), Located.get());
        }

        { // Scope
          // "Warning Wormsign"
            auto WarningChunk  = getChunkFromFile(HouseString + "WARNING.VOC");
            auto WormSignChunk = getChunkFromFile(HouseString + "WORMY.VOC");
            lngVoice[static_cast<int>(Voice_enum::WarningWormSign) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(WarningChunk.get(), WormSignChunk.get());
        }

        // "Our base is under attack"
        lngVoice[static_cast<int>(Voice_enum::BaseIsUnderAttack) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile(HouseString + "ATTACK.VOC");

        { // Scope
          // "Saboteur approaching" and "Missile approaching"
            auto SabotChunk       = getChunkFromFile(HouseString + "SABOT.VOC");
            auto MissileChunk     = getChunkFromFile(HouseString + "MISSILE.VOC");
            auto ApproachingChunk = getChunkFromFile(HouseString + "APPRCH.VOC");
            lngVoice[static_cast<int>(Voice_enum::SaboteurApproaching) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(SabotChunk.get(), ApproachingChunk.get());
            lngVoice[static_cast<int>(Voice_enum::MissileApproaching) * NUM_HOUSES + VoiceNum] =
                concat2Chunks(MissileChunk.get(), ApproachingChunk.get());
        }

        HouseNameChunk.reset();

        // "Yes Sir"
        lngVoice[static_cast<int>(Voice_enum::YesSir) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile("ZREPORT1.VOC", "REPORT1.VOC");

        // "Reporting"
        lngVoice[static_cast<int>(Voice_enum::Reporting) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile("ZREPORT2.VOC", "REPORT2.VOC");

        // "Acknowledged"
        lngVoice[static_cast<int>(Voice_enum::Acknowledged) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile("ZREPORT3.VOC", "REPORT3.VOC");

        // "Affirmative"
        lngVoice[static_cast<int>(Voice_enum::Affirmative) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile("ZAFFIRM.VOC", "AFFIRM.VOC");

        // "Moving out"
        lngVoice[static_cast<int>(Voice_enum::MovingOut) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile("ZMOVEOUT.VOC", "MOVEOUT.VOC");

        // "Infantry out"
        lngVoice[static_cast<int>(Voice_enum::InfantryOut) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile("ZOVEROUT.VOC", "OVEROUT.VOC");

        // "Something's under the sand"
        lngVoice[static_cast<int>(Voice_enum::SomethingUnderTheSand) * NUM_HOUSES + VoiceNum] =
            getChunkFromFile("SANDBUG.VOC");

        // "House Harkonnen"
        lngVoice[static_cast<int>(Voice_enum::HouseHarkonnen) * NUM_HOUSES + VoiceNum] = getChunkFromFile("MHARK.VOC");

        // "House Atreides"
        lngVoice[static_cast<int>(Voice_enum::HouseAtreides) * NUM_HOUSES + VoiceNum] = getChunkFromFile("MATRE.VOC");

        // "House Ordos"
        lngVoice[static_cast<int>(Voice_enum::HouseOrdos) * NUM_HOUSES + VoiceNum] = getChunkFromFile("MORDOS.VOC");
    }

    const auto bad_voice = std::find(lngVoice.cbegin(), lngVoice.cend(), nullptr);

    if (bad_voice != lngVoice.cend()) {
        THROW(std::runtime_error, "Not all voice sounds could be loaded: lngVoice[%d] == nullptr!",
              static_cast<int>(bad_voice - lngVoice.cbegin()));
    }

    loadSounds();
}

Mix_Chunk* SFXManager::getEnglishVoice(Voice_enum id, HOUSETYPE house) const {
    const auto voice_index = static_cast<int>(id) * NUM_HOUSES + static_cast<int>(house);

    if (voice_index < 0 || std::cmp_greater_equal(voice_index, lngVoice.size()))
        return nullptr;

    return lngVoice[voice_index].get();
}

void SFXManager::loadNonEnglishVoice(const std::string& languagePrefix) {
    lngVoice.clear();
    lngVoice.resize(static_cast<int>(Voice_enum::NUM_VOICE));

    // "Harvester deployed"
    lngVoice[static_cast<int>(Voice_enum::HarvesterDeployed)] = getChunkFromFile(languagePrefix + "HARVEST.VOC");

    // "Unit deployed"
    lngVoice[static_cast<int>(Voice_enum::UnitDeployed)] = getChunkFromFile(languagePrefix + "DEPLOY.VOC");

    // "Unit launched"
    lngVoice[static_cast<int>(Voice_enum::UnitLaunched)] = getChunkFromFile(languagePrefix + "VEHICLE.VOC");

    // "Construction complete"
    lngVoice[static_cast<int>(Voice_enum::ConstructionComplete)] = getChunkFromFile(languagePrefix + "CONST.VOC");

    // "Vehicle repaired"
    lngVoice[static_cast<int>(Voice_enum::VehicleRepaired)] = getChunkFromFile(languagePrefix + "REPAIR.VOC");

    // "Frigate has arrived"
    lngVoice[static_cast<int>(Voice_enum::FrigateHasArrived)] = getChunkFromFile(languagePrefix + "FRIGATE.VOC");

    // "Your mission is complete" (No non-english voc available)
    lngVoice[static_cast<int>(Voice_enum::YourMissionIsComplete)] = createEmptyChunk();

    // "You have failed your mission" (No non-english voc available)
    lngVoice[static_cast<int>(Voice_enum::YouHaveFailedYourMission)] = createEmptyChunk();

    // "Radar activated"/"Radar deactivated"
    lngVoice[static_cast<int>(Voice_enum::RadarActivated)]   = getChunkFromFile(languagePrefix + "ON.VOC");
    lngVoice[static_cast<int>(Voice_enum::RadarDeactivated)] = getChunkFromFile(languagePrefix + "OFF.VOC");

    // "Bloom located"
    lngVoice[static_cast<int>(Voice_enum::BloomLocated)] = getChunkFromFile(languagePrefix + "BLOOM.VOC");

    // "Warning Wormsign"
    if (dune::globals::pFileManager->exists(languagePrefix + "WORMY.VOC")) {
        const auto WarningChunk  = getChunkFromFile(languagePrefix + "WARNING.VOC");
        const auto WormSignChunk = getChunkFromFile(languagePrefix + "WORMY.VOC");
        lngVoice[static_cast<int>(Voice_enum::WarningWormSign)] =
            concat2Chunks(WarningChunk.get(), WormSignChunk.get());
    } else {
        lngVoice[static_cast<int>(Voice_enum::WarningWormSign)] = getChunkFromFile(languagePrefix + "WARNING.VOC");
    }

    // "Our base is under attack"
    lngVoice[static_cast<int>(Voice_enum::BaseIsUnderAttack)] = getChunkFromFile(languagePrefix + "ATTACK.VOC");

    // "Saboteur approaching"
    lngVoice[static_cast<int>(Voice_enum::SaboteurApproaching)] = getChunkFromFile(languagePrefix + "SABOT.VOC");

    // "Missile approaching"
    lngVoice[static_cast<int>(Voice_enum::MissileApproaching)] = getChunkFromFile(languagePrefix + "MISSILE.VOC");

    // "Yes Sir"
    lngVoice[static_cast<int>(Voice_enum::YesSir)] = getChunkFromFile(languagePrefix + "REPORT1.VOC");

    // "Reporting"
    lngVoice[static_cast<int>(Voice_enum::Reporting)] = getChunkFromFile(languagePrefix + "REPORT2.VOC");

    // "Acknowledged"
    lngVoice[static_cast<int>(Voice_enum::Acknowledged)] = getChunkFromFile(languagePrefix + "REPORT3.VOC");

    // "Affirmative"
    lngVoice[static_cast<int>(Voice_enum::Affirmative)] = getChunkFromFile(languagePrefix + "AFFIRM.VOC");

    // "Moving out"
    lngVoice[static_cast<int>(Voice_enum::MovingOut)] = getChunkFromFile(languagePrefix + "MOVEOUT.VOC");

    // "Infantry out"
    lngVoice[static_cast<int>(Voice_enum::InfantryOut)] = getChunkFromFile(languagePrefix + "OVEROUT.VOC");

    // "Something's under the sand"
    lngVoice[static_cast<int>(Voice_enum::SomethingUnderTheSand)] = getChunkFromFile("SANDBUG.VOC");

    // "House Atreides"
    lngVoice[static_cast<int>(Voice_enum::HouseAtreides)] = getChunkFromFile(languagePrefix + "ATRE.VOC");

    // "House Ordos"
    lngVoice[static_cast<int>(Voice_enum::HouseOrdos)] = getChunkFromFile(languagePrefix + "ORDOS.VOC");

    // "House Harkonnen"
    lngVoice[static_cast<int>(Voice_enum::HouseHarkonnen)] = getChunkFromFile(languagePrefix + "HARK.VOC");

    const auto bad_voice = std::find(lngVoice.cbegin(), lngVoice.cend(), nullptr);

    if (bad_voice != lngVoice.cend()) {
        THROW(std::runtime_error, "Not all voice sounds could be loaded: lngVoice[%d] == nullptr!",
              static_cast<int>(bad_voice - lngVoice.cbegin()));
    }

    loadSounds();
}

void SFXManager::loadSounds() {

    auto rwop = dune::globals::pFileManager->openFile("DUNE1.ADL");

    SoundAdlibPC dune1{rwop.get(), AUDIO_FREQUENCY};

    rwop.reset();

    // Sfx
    soundChunk[static_cast<int>(Sound_enum::Sound_PlaceStructure)] = getChunkFromFile("EXDUD.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_ButtonClick)]    = getChunkFromFile("BUTTON.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_InvalidAction)]  = dune1.getSubsong(47);

    const auto volume = dune1.getVolume();
    dune1.setVolume(MIX_MAX_VOLUME);
    soundChunk[static_cast<int>(Sound_enum::Sound_CreditsTick)]     = dune1.getSubsong(52);
    soundChunk[static_cast<int>(Sound_enum::Sound_CreditsTickDown)] = dune1.getSubsong(53);
    dune1.setVolume(volume);

    soundChunk[static_cast<int>(Sound_enum::Sound_Tick)]               = dune1.getSubsong(38);
    soundChunk[static_cast<int>(Sound_enum::Sound_RadarNoise)]         = getChunkFromFile("STATICP.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_ExplosionGas)]       = getChunkFromFile("EXGAS.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_ExplosionTiny)]      = getChunkFromFile("EXTINY.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_ExplosionSmall)]     = getChunkFromFile("EXSMALL.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_ExplosionMedium)]    = getChunkFromFile("EXMED.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_ExplosionLarge)]     = getChunkFromFile("EXLARGE.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_ExplosionStructure)] = getChunkFromFile("CRUMBLE.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_WormAttack)]         = getChunkFromFile("WORMET3P.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Gun)]                = getChunkFromFile("GUN.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Rocket)]             = getChunkFromFile("ROCKET.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Bloom)]              = getChunkFromFile("EXSAND.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Scream1)]            = getChunkFromFile("VSCREAM1.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Scream2)]            = getChunkFromFile("VSCREAM2.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Scream3)]            = getChunkFromFile("VSCREAM3.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Scream4)]            = getChunkFromFile("VSCREAM4.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Scream5)]            = getChunkFromFile("VSCREAM5.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Trumpet)]            = dune1.getSubsong(30);
    soundChunk[static_cast<int>(Sound_enum::Sound_Drop)]               = dune1.getSubsong(24);
    soundChunk[static_cast<int>(Sound_enum::Sound_Squashed)]           = getChunkFromFile("SQUISH2.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_MachineGun)]         = getChunkFromFile("GUNMULTI.VOC");
    soundChunk[static_cast<int>(Sound_enum::Sound_Sonic)]              = dune1.getSubsong(43);
    soundChunk[static_cast<int>(Sound_enum::Sound_RocketSmall)]        = getChunkFromFile("MISLTINP.VOC");
}

Mix_Chunk* SFXManager::getNonEnglishVoice(Voice_enum id, [[maybe_unused]] HOUSETYPE house) const {
    const auto voice_index = static_cast<int>(id);

    if (voice_index < 0 || std::cmp_greater_equal(voice_index, lngVoice.size()))
        return nullptr;

    return lngVoice[voice_index].get();
}
