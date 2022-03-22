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

#include <FileClasses/TextManager.h>

#include <misc/string_util.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/POFile.h>

#include <misc/FileSystem.h>
#include <misc/exceptions.h>

#ifdef _
#    undef _
#endif
#define _(msgid) getLocalized(msgid)

namespace {
struct FilePtrCloser {
    void operator()(FILE* fp) const { fclose(fp); }
};
} // namespace

using file_ptr = std::unique_ptr<FILE*, FilePtrCloser>;

sdl2::RWops_ptr openReadOnlyRWops(const std::filesystem::path& path) {
    const auto normal = path.lexically_normal();

    auto* const rwops = SDL_RWFromFile(normal.u8string().c_str(), "r");

    if (!rwops)
        THROW(sdl_error, "Opening file '%s' failed: %s!", normal.u8string().c_str(), SDL_GetError());

    return sdl2::RWops_ptr {rwops};
}

TextManager::TextManager(std::string_view language) {
    const auto locale_directory = getDuneLegacyDataDir() / "locale";

    auto languages = getFileNamesList(locale_directory, std::string {language} + ".po", true, FileListOrder_Name_Asc);

    const auto language_file = languages.empty() ? std::filesystem::path {"English.en.po"} : languages.front();

    const auto language_path = locale_directory / language_file;
    sdl2::log_info("Loading localization from '%s'...", language_path.u8string().c_str());
    auto rwops      = openReadOnlyRWops(language_path.u8string());
    localizedString = loadPOFile(rwops.get(), language_file.u8string());
}

TextManager::~TextManager() = default;

void TextManager::loadData() {
    const auto ext = _("LanguageFileExtension");

    addOrigDuneText("TEXTH." + ext, true);
    addOrigDuneText("TEXTA." + ext, true);
    addOrigDuneText("TEXTO." + ext, true);
    addOrigDuneText("DUNE." + ext);
    addOrigDuneText("MESSAGE." + ext);

    // load all mentat texts
    mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)] = std::make_unique<MentatTextFile>(pFileManager->openFile("MENTATH." + ext).get());
    mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)]  = std::make_unique<MentatTextFile>(pFileManager->openFile("MENTATA." + ext).get());
    mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ORDOS)]     = std::make_unique<MentatTextFile>(pFileManager->openFile("MENTATO." + ext).get());
}

std::string TextManager::getBriefingText(unsigned int mission, unsigned int texttype, HOUSETYPE house) const {
    // clang-format off
    switch(house) {
        case HOUSETYPE::HOUSE_HARKONNEN: {
            switch(texttype) {
                case MISSION_DESCRIPTION: {
                    switch(mission) {
                        case 0: return _("@TEXTH.ENG|0#Description of House Harkonnen");       break;
                        case 1: return _("@TEXTH.ENG|4#Description of Harkonnen mission 1");   break;
                        case 2: return _("@TEXTH.ENG|8#Description of Harkonnen mission 2");   break;
                        case 3: return _("@TEXTH.ENG|12#Description of Harkonnen mission 3");  break;
                        case 4: return _("@TEXTH.ENG|16#Description of Harkonnen mission 4");  break;
                        case 5: return _("@TEXTH.ENG|20#Description of Harkonnen mission 5");  break;
                        case 6: return _("@TEXTH.ENG|24#Description of Harkonnen mission 6");  break;
                        case 7: return _("@TEXTH.ENG|28#Description of Harkonnen mission 7");  break;
                        case 8: return _("@TEXTH.ENG|32#Description of Harkonnen mission 8");  break;
                        case 9: return _("@TEXTH.ENG|36#Description of Harkonnen mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                case MISSION_WIN: {
                    switch(mission) {
                        case 1: return _("@TEXTH.ENG|5#Winning text of Harkonnen mission 1");   break;
                        case 2: return _("@TEXTH.ENG|9#Winning text of Harkonnen mission 2");   break;
                        case 3: return _("@TEXTH.ENG|13#Winning text of Harkonnen mission 3");  break;
                        case 4: return _("@TEXTH.ENG|17#Winning text of Harkonnen mission 4");  break;
                        case 5: return _("@TEXTH.ENG|21#Winning text of Harkonnen mission 5");  break;
                        case 6: return _("@TEXTH.ENG|25#Winning text of Harkonnen mission 6");  break;
                        case 7: return _("@TEXTH.ENG|29#Winning text of Harkonnen mission 7");  break;
                        case 8: return _("@TEXTH.ENG|33#Winning text of Harkonnen mission 8");  break;
                        case 9: return _("@TEXTH.ENG|37#Winning text of Harkonnen mission 9");  break;
                        default: return "";                                                     break;
                    }
                } break;

                case MISSION_LOSE: {
                    switch(mission) {
                        case 1: return _("@TEXTH.ENG|6#Losing text of Harkonnen mission 1");   break;
                        case 2: return _("@TEXTH.ENG|10#Losing text of Harkonnen mission 2");  break;
                        case 3: return _("@TEXTH.ENG|14#Losing text of Harkonnen mission 3");  break;
                        case 4: return _("@TEXTH.ENG|18#Losing text of Harkonnen mission 4");  break;
                        case 5: return _("@TEXTH.ENG|22#Losing text of Harkonnen mission 5");  break;
                        case 6: return _("@TEXTH.ENG|26#Losing text of Harkonnen mission 6");  break;
                        case 7: return _("@TEXTH.ENG|30#Losing text of Harkonnen mission 7");  break;
                        case 8: return _("@TEXTH.ENG|34#Losing text of Harkonnen mission 8");  break;
                        case 9: return _("@TEXTH.ENG|38#Losing text of Harkonnen mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                case MISSION_ADVICE: {
                    switch(mission) {
                        case 1: return _("@TEXTH.ENG|7#Advice text for Harkonnen mission 1");   break;
                        case 2: return _("@TEXTH.ENG|11#Advice text for Harkonnen mission 2");  break;
                        case 3: return _("@TEXTH.ENG|15#Advice text for Harkonnen mission 3");  break;
                        case 4: return _("@TEXTH.ENG|19#Advice text for Harkonnen mission 4");  break;
                        case 5: return _("@TEXTH.ENG|23#Advice text for Harkonnen mission 5");  break;
                        case 6: return _("@TEXTH.ENG|27#Advice text for Harkonnen mission 6");  break;
                        case 7: return _("@TEXTH.ENG|31#Advice text for Harkonnen mission 7");  break;
                        case 8: return _("@TEXTH.ENG|35#Advice text for Harkonnen mission 8");  break;
                        case 9: return _("@TEXTH.ENG|39#Advice text for Harkonnen mission 9");  break;
                        default: return "";                                                     break;
                    }
                } break;

                default: {
                    return "";
                } break;
            }
        } break;

        case HOUSETYPE::HOUSE_ATREIDES: {
            switch(texttype) {
                case MISSION_DESCRIPTION: {
                    switch(mission) {
                        case 0: return _("@TEXTA.ENG|0#Description of House Atreides");       break;
                        case 1: return _("@TEXTA.ENG|4#Description of Atreides mission 1");   break;
                        case 2: return _("@TEXTA.ENG|8#Description of Atreides mission 2");   break;
                        case 3: return _("@TEXTA.ENG|12#Description of Atreides mission 3");  break;
                        case 4: return _("@TEXTA.ENG|16#Description of Atreides mission 4");  break;
                        case 5: return _("@TEXTA.ENG|20#Description of Atreides mission 5");  break;
                        case 6: return _("@TEXTA.ENG|24#Description of Atreides mission 6");  break;
                        case 7: return _("@TEXTA.ENG|28#Description of Atreides mission 7");  break;
                        case 8: return _("@TEXTA.ENG|32#Description of Atreides mission 8");  break;
                        case 9: return _("@TEXTA.ENG|36#Description of Atreides mission 9");  break;
                        default: return "";                                                   break;
                    }
                } break;

                case MISSION_WIN: {
                    switch(mission) {
                        case 1: return _("@TEXTA.ENG|5#Winning text of Atreides mission 1");   break;
                        case 2: return _("@TEXTA.ENG|9#Winning text of Atreides mission 2");   break;
                        case 3: return _("@TEXTA.ENG|13#Winning text of Atreides mission 3");  break;
                        case 4: return _("@TEXTA.ENG|17#Winning text of Atreides mission 4");  break;
                        case 5: return _("@TEXTA.ENG|21#Winning text of Atreides mission 5");  break;
                        case 6: return _("@TEXTA.ENG|25#Winning text of Atreides mission 6");  break;
                        case 7: return _("@TEXTA.ENG|29#Winning text of Atreides mission 7");  break;
                        case 8: return _("@TEXTA.ENG|33#Winning text of Atreides mission 8");  break;
                        case 9: return _("@TEXTA.ENG|37#Winning text of Atreides mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                case MISSION_LOSE: {
                    switch(mission) {
                        case 1: return _("@TEXTA.ENG|6#Losing text of Atreides mission 1");   break;
                        case 2: return _("@TEXTA.ENG|10#Losing text of Atreides mission 2");  break;
                        case 3: return _("@TEXTA.ENG|14#Losing text of Atreides mission 3");  break;
                        case 4: return _("@TEXTA.ENG|18#Losing text of Atreides mission 4");  break;
                        case 5: return _("@TEXTA.ENG|22#Losing text of Atreides mission 5");  break;
                        case 6: return _("@TEXTA.ENG|26#Losing text of Atreides mission 6");  break;
                        case 7: return _("@TEXTA.ENG|30#Losing text of Atreides mission 7");  break;
                        case 8: return _("@TEXTA.ENG|34#Losing text of Atreides mission 8");  break;
                        case 9: return _("@TEXTA.ENG|38#Losing text of Atreides mission 9");  break;
                        default: return "";                                                   break;
                    }
                } break;

                case MISSION_ADVICE: {
                    switch(mission) {
                        case 1: return _("@TEXTA.ENG|7#Advice text for Atreides mission 1");   break;
                        case 2: return _("@TEXTA.ENG|11#Advice text for Atreides mission 2");  break;
                        case 3: return _("@TEXTA.ENG|15#Advice text for Atreides mission 3");  break;
                        case 4: return _("@TEXTA.ENG|19#Advice text for Atreides mission 4");  break;
                        case 5: return _("@TEXTA.ENG|23#Advice text for Atreides mission 5");  break;
                        case 6: return _("@TEXTA.ENG|27#Advice text for Atreides mission 6");  break;
                        case 7: return _("@TEXTA.ENG|31#Advice text for Atreides mission 7");  break;
                        case 8: return _("@TEXTA.ENG|35#Advice text for Atreides mission 8");  break;
                        case 9: return _("@TEXTA.ENG|39#Advice text for Atreides mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                default: {
                    return "";
                } break;
            }
        } break;

        case HOUSETYPE::HOUSE_ORDOS: {
            switch(texttype) {
                case MISSION_DESCRIPTION: {
                    switch(mission) {
                        case 0: return _("@TEXTO.ENG|0#Description of House Ordos");       break;
                        case 1: return _("@TEXTO.ENG|4#Description of Ordos mission 1");   break;
                        case 2: return _("@TEXTO.ENG|8#Description of Ordos mission 2");   break;
                        case 3: return _("@TEXTO.ENG|12#Description of Ordos mission 3");  break;
                        case 4: return _("@TEXTO.ENG|16#Description of Ordos mission 4");  break;
                        case 5: return _("@TEXTO.ENG|20#Description of Ordos mission 5");  break;
                        case 6: return _("@TEXTO.ENG|24#Description of Ordos mission 6");  break;
                        case 7: return _("@TEXTO.ENG|28#Description of Ordos mission 7");  break;
                        case 8: return _("@TEXTO.ENG|32#Description of Ordos mission 8");  break;
                        case 9: return _("@TEXTO.ENG|36#Description of Ordos mission 9");  break;
                        default: return "";                                                break;
                    }
                } break;

                case MISSION_WIN: {
                    switch(mission) {
                        case 1: return _("@TEXTO.ENG|5#Winning text of Ordos mission 1");   break;
                        case 2: return _("@TEXTO.ENG|9#Winning text of Ordos mission 2");   break;
                        case 3: return _("@TEXTO.ENG|13#Winning text of Ordos mission 3");  break;
                        case 4: return _("@TEXTO.ENG|17#Winning text of Ordos mission 4");  break;
                        case 5: return _("@TEXTO.ENG|21#Winning text of Ordos mission 5");  break;
                        case 6: return _("@TEXTO.ENG|25#Winning text of Ordos mission 6");  break;
                        case 7: return _("@TEXTO.ENG|29#Winning text of Ordos mission 7");  break;
                        case 8: return _("@TEXTO.ENG|33#Winning text of Ordos mission 8");  break;
                        case 9: return _("@TEXTO.ENG|37#Winning text of Ordos mission 9");  break;
                        default: return "";                                                 break;
                    }
                } break;

                case MISSION_LOSE: {
                    switch(mission) {
                        case 1: return _("@TEXTO.ENG|6#Losing text of Ordos mission 1");   break;
                        case 2: return _("@TEXTO.ENG|10#Losing text of Ordos mission 2");  break;
                        case 3: return _("@TEXTO.ENG|14#Losing text of Ordos mission 3");  break;
                        case 4: return _("@TEXTO.ENG|18#Losing text of Ordos mission 4");  break;
                        case 5: return _("@TEXTO.ENG|22#Losing text of Ordos mission 5");  break;
                        case 6: return _("@TEXTO.ENG|26#Losing text of Ordos mission 6");  break;
                        case 7: return _("@TEXTO.ENG|30#Losing text of Ordos mission 7");  break;
                        case 8: return _("@TEXTO.ENG|34#Losing text of Ordos mission 8");  break;
                        case 9: return _("@TEXTO.ENG|38#Losing text of Ordos mission 9");  break;
                        default: return "";                                                break;
                    }
                } break;

                case MISSION_ADVICE: {
                    switch(mission) {
                        case 1: return _("@TEXTO.ENG|7#Advice text for Ordos mission 1");   break;
                        case 2: return _("@TEXTO.ENG|11#Advice text for Ordos mission 2");  break;
                        case 3: return _("@TEXTO.ENG|15#Advice text for Ordos mission 3");  break;
                        case 4: return _("@TEXTO.ENG|19#Advice text for Ordos mission 4");  break;
                        case 5: return _("@TEXTO.ENG|23#Advice text for Ordos mission 5");  break;
                        case 6: return _("@TEXTO.ENG|27#Advice text for Ordos mission 6");  break;
                        case 7: return _("@TEXTO.ENG|31#Advice text for Ordos mission 7");  break;
                        case 8: return _("@TEXTO.ENG|35#Advice text for Ordos mission 8");  break;
                        case 9: return _("@TEXTO.ENG|39#Advice text for Ordos mission 9");  break;
                        default: return "";                                                 break;
                    }
                } break;

                default: {
                    return "";
                } break;
            }
        } break;

        case HOUSETYPE::HOUSE_FREMEN: {
            switch(texttype) {
                case MISSION_DESCRIPTION: {
                    switch(mission) {
                        case 0: return _("House Fremen\nArrakis, the home planet of House Fremen, is one giant sand desert. Perfectly adapted to the warm, dry climate, the Fremen have huge advantage in defending their planet.");       break;
                        case 1: return _("@TEXTA.ENG|4|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 1");   break;
                        case 2: return _("@TEXTA.ENG|8|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 2");   break;
                        case 3: return _("@TEXTA.ENG|12|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 3");  break;
                        case 4: return _("@TEXTA.ENG|16|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 4");  break;
                        case 5: return _("@TEXTA.ENG|20|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 5");  break;
                        case 6: return _("@TEXTA.ENG|24|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 6");  break;
                        case 7: return _("@TEXTA.ENG|28|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 7");  break;
                        case 8: return _("@TEXTA.ENG|32|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Description of Fremen mission 8");  break;
                        case 9: return _("@TEXTA.ENG|36|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary|remaining Ordos and Harkonnen troops->remaining Ordos, Mercenary and Harkonnen troops#Description of Fremen mission 9");  break;
                        default: return "";                                                   break;
                    }
                } break;

                case MISSION_WIN: {
                    switch(mission) {
                        case 1: return _("@TEXTA.ENG|5|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 1");   break;
                        case 2: return _("@TEXTA.ENG|9|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 2");   break;
                        case 3: return _("@TEXTA.ENG|13|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 3");  break;
                        case 4: return _("@TEXTA.ENG|17|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 4");  break;
                        case 5: return _("@TEXTA.ENG|21|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 5");  break;
                        case 6: return _("@TEXTA.ENG|25|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 6");  break;
                        case 7: return _("@TEXTA.ENG|29|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 7");  break;
                        case 8: return _("@TEXTA.ENG|33|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 8");  break;
                        case 9: return _("@TEXTA.ENG|37|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Winning text of Fremen mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                case MISSION_LOSE: {
                    switch(mission) {
                        case 1: return _("@TEXTA.ENG|6|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 1");   break;
                        case 2: return _("@TEXTA.ENG|10|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 2");  break;
                        case 3: return _("@TEXTA.ENG|14|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 3");  break;
                        case 4: return _("@TEXTA.ENG|18|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 4");  break;
                        case 5: return _("@TEXTA.ENG|22|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 5");  break;
                        case 6: return _("@TEXTA.ENG|26|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 6");  break;
                        case 7: return _("@TEXTA.ENG|30|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 7");  break;
                        case 8: return _("@TEXTA.ENG|34|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 8");  break;
                        case 9: return _("@TEXTA.ENG|38|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Losing text of Fremen mission 9");  break;
                        default: return "";                                                   break;
                    }
                } break;

                case MISSION_ADVICE: {
                    switch(mission) {
                        case 1: return _("@TEXTA.ENG|7|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 1");   break;
                        case 2: return _("@TEXTA.ENG|11|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 2");  break;
                        case 3: return _("@TEXTA.ENG|15|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 3");  break;
                        case 4: return _("@TEXTA.ENG|19|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 4");  break;
                        case 5: return _("@TEXTA.ENG|23|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 5");  break;
                        case 6: return _("@TEXTA.ENG|27|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 6");  break;
                        case 7: return _("@TEXTA.ENG|31|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 7");  break;
                        case 8: return _("@TEXTA.ENG|35|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 8");  break;
                        case 9: return _("@TEXTA.ENG|39|Atreides->Fremen|Ordos->Sardaukar|Harkonnen->Mercenary#Advice text for Fremen mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                default: {
                    return "";
                } break;
            }
        } break;

        case HOUSETYPE::HOUSE_SARDAUKAR: {
            switch(texttype) {
                case MISSION_DESCRIPTION: {
                    switch(mission) {
                        case 0: return _("House Sardaukar\nOriginating from an forgotten planet, House Sardaukar has moved from planet to planet since then. Conquering foreign planets has specialized their combat skills which got them a reputation of being cruel.");  break;
                        case 1: return _("@TEXTH.ENG|4|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Description of Sardaukar mission 1");   break;
                        case 2: return _("@TEXTH.ENG|8|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Description of Sardaukar mission 2");   break;
                        case 3: return _("@TEXTH.ENG|12|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Description of Sardaukar mission 3");  break;
                        case 4: return _("@TEXTH.ENG|16|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Description of Sardaukar mission 4");  break;
                        case 5: return _("@TEXTH.ENG|20|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar|the Emperor was->the Atreides were#Description of Sardaukar mission 5");  break;
                        case 6: return _("@TEXTH.ENG|24|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Description of Sardaukar mission 6");  break;
                        case 7: return _("@TEXTH.ENG|28|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Description of Sardaukar mission 7");  break;
                        case 8: return _("@TEXTH.ENG|32|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Description of Sardaukar mission 8");  break;
                        case 9: return _("@TEXTH.ENG|36|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar|the Emperor's->their leader's#Description of Sardaukar mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                case MISSION_WIN: {
                    switch(mission) {
                        case 1: return _("@TEXTH.ENG|5|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 1");   break;
                        case 2: return _("@TEXTH.ENG|9|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 2");   break;
                        case 3: return _("@TEXTH.ENG|13|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 3");  break;
                        case 4: return _("@TEXTH.ENG|17|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 4");  break;
                        case 5: return _("@TEXTH.ENG|21|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 5");  break;
                        case 6: return _("@TEXTH.ENG|25|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 6");  break;
                        case 7: return _("@TEXTH.ENG|29|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 7");  break;
                        case 8: return _("@TEXTH.ENG|33|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Winning text of Sardaukar mission 8");  break;
                        case 9: return _("@TEXTH.ENG|37|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar|the Emperor->the leader#Winning text of Sardaukar mission 9");  break;
                        default: return "";                                                     break;
                    }
                } break;

                case MISSION_LOSE: {
                    switch(mission) {
                        case 1: return _("@TEXTH.ENG|6|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 1");   break;
                        case 2: return _("@TEXTH.ENG|10|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 2");  break;
                        case 3: return _("@TEXTH.ENG|14|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 3");  break;
                        case 4: return _("@TEXTH.ENG|18|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 4");  break;
                        case 5: return _("@TEXTH.ENG|22|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 5");  break;
                        case 6: return _("@TEXTH.ENG|26|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 6");  break;
                        case 7: return _("@TEXTH.ENG|30|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 7");  break;
                        case 8: return _("@TEXTH.ENG|34|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 8");  break;
                        case 9: return _("@TEXTH.ENG|38|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Losing text of Sardaukar mission 9");  break;
                        default: return "";                                                    break;
                    }
                } break;

                case MISSION_ADVICE: {
                    switch(mission) {
                        case 1: return _("@TEXTH.ENG|7|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 1");   break;
                        case 2: return _("@TEXTH.ENG|11|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 2");  break;
                        case 3: return _("@TEXTH.ENG|15|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 3");  break;
                        case 4: return _("@TEXTH.ENG|19|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 4");  break;
                        case 5: return _("@TEXTH.ENG|23|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 5");  break;
                        case 6: return _("@TEXTH.ENG|27|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 6");  break;
                        case 7: return _("@TEXTH.ENG|31|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 7");  break;
                        case 8: return _("@TEXTH.ENG|35|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 8");  break;
                        case 9: return _("@TEXTH.ENG|39|Atreides->Mercenary|Ordos->Fremen|Harkonnen->Sardaukar#Advice text for Sardaukar mission 9");  break;
                        default: return "";                                                     break;
                    }
                } break;

                default: {
                    return "";
                } break;
            }
        } break;

        case HOUSETYPE::HOUSE_MERCENARY:
        default: {
            switch(texttype) {
                case MISSION_DESCRIPTION: {
                    switch(mission) {
                        case 0: return _("House Mercenary\nThe home planet of the Mercenary consists of huge oceans and large deserts. House Mercenary is one of the oldest and most influencial intergalactical trading organization. To strengthen their power they don't hesitate to use military force.");       break;
                        case 1: return _("@TEXTO.ENG|4|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 1");   break;
                        case 2: return _("@TEXTO.ENG|8|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 2");   break;
                        case 3: return _("@TEXTO.ENG|12|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 3");  break;
                        case 4: return _("@TEXTO.ENG|16|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 4");  break;
                        case 5: return _("@TEXTO.ENG|20|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 5");  break;
                        case 6: return _("@TEXTO.ENG|24|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 6");  break;
                        case 7: return _("@TEXTO.ENG|28|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 7");  break;
                        case 8: return _("@TEXTO.ENG|32|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 8");  break;
                        case 9: return _("@TEXTO.ENG|36|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Description of Mercenary mission 9");  break;
                        default: return "";                                                break;
                    }
                } break;

                case MISSION_WIN: {
                    switch(mission) {
                        case 1: return _("@TEXTO.ENG|5|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 1");   break;
                        case 2: return _("@TEXTO.ENG|9|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 2");   break;
                        case 3: return _("@TEXTO.ENG|13|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 3");  break;
                        case 4: return _("@TEXTO.ENG|17|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 4");  break;
                        case 5: return _("@TEXTO.ENG|21|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 5");  break;
                        case 6: return _("@TEXTO.ENG|25|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 6");  break;
                        case 7: return _("@TEXTO.ENG|29|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 7");  break;
                        case 8: return _("@TEXTO.ENG|33|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 8");  break;
                        case 9: return _("@TEXTO.ENG|37|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Winning text of Mercenary mission 9");  break;
                        default: return "";                                                 break;
                    }
                } break;

                case MISSION_LOSE: {
                    switch(mission) {
                        case 1: return _("@TEXTO.ENG|6|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 1");   break;
                        case 2: return _("@TEXTO.ENG|10|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 2");  break;
                        case 3: return _("@TEXTO.ENG|14|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 3");  break;
                        case 4: return _("@TEXTO.ENG|18|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 4");  break;
                        case 5: return _("@TEXTO.ENG|22|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 5");  break;
                        case 6: return _("@TEXTO.ENG|26|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 6");  break;
                        case 7: return _("@TEXTO.ENG|30|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 7");  break;
                        case 8: return _("@TEXTO.ENG|34|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 8");  break;
                        case 9: return _("@TEXTO.ENG|38|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Losing text of Mercenary mission 9");  break;
                        default: return "";                                                break;
                    }
                } break;

                case MISSION_ADVICE: {
                    switch(mission) {
                        case 1: return _("@TEXTO.ENG|7|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 1");   break;
                        case 2: return _("@TEXTO.ENG|11|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 2");  break;
                        case 3: return _("@TEXTO.ENG|15|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 3");  break;
                        case 4: return _("@TEXTO.ENG|19|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 4");  break;
                        case 5: return _("@TEXTO.ENG|23|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 5");  break;
                        case 6: return _("@TEXTO.ENG|27|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 6");  break;
                        case 7: return _("@TEXTO.ENG|31|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 7");  break;
                        case 8: return _("@TEXTO.ENG|35|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 8");  break;
                        case 9: return _("@TEXTO.ENG|39|Atreides->Sardaukar|Ordos->Mercenary|Harkonnen->Fremen#Advice text for Mercenary mission 9");  break;
                        default: return "";                                                 break;
                    }
                } break;

                default: {
                    return "";
                } break;
            }

        } break;

    }
    // clang-format on
}

std::vector<MentatTextFile::MentatEntry> TextManager::getAllMentatEntries(HOUSETYPE house, unsigned int techLevel) const {
    std::vector<MentatTextFile::MentatEntry> mentatEntries;

    switch (house) {
        case HOUSETYPE::HOUSE_HARKONNEN:
        case HOUSETYPE::HOUSE_SARDAUKAR:
        default: {
            for (unsigned int i = 0; i < mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)]->getNumEntries(); i++) {
                if (mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)]->getMentatEntry(i).techLevel <= techLevel) {
                    mentatEntries.push_back(mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_HARKONNEN)]->getMentatEntry(i));
                }
            }
        } break;

        case HOUSETYPE::HOUSE_ATREIDES:
        case HOUSETYPE::HOUSE_FREMEN: {
            for (unsigned int i = 0; i < mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)]->getNumEntries(); i++) {
                if (mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)]->getMentatEntry(i).techLevel <= techLevel) {
                    mentatEntries.push_back(mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ATREIDES)]->getMentatEntry(i));
                }
            }
        } break;

        case HOUSETYPE::HOUSE_ORDOS:
        case HOUSETYPE::HOUSE_MERCENARY: {
            for (unsigned int i = 0; i < mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ORDOS)]->getNumEntries(); i++) {
                if (mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ORDOS)]->getMentatEntry(i).techLevel <= techLevel) {
                    mentatEntries.push_back(mentatStrings[static_cast<int>(HOUSETYPE::HOUSE_ORDOS)]->getMentatEntry(i));
                }
            }
        } break;
    }

    return mentatEntries;
}

const std::string& TextManager::postProcessString(const std::string& unprocessedString) const {
    size_t commentStart = unprocessedString.find_first_of('#');
    if (commentStart == std::string::npos) {
        commentStart = unprocessedString.size();
    }

    std::string commandString = unprocessedString.substr(1, commentStart - 1);

    std::vector<std::string> commands = splitStringToStringVector(commandString, "\\|");

    int index = -1;
    if (commands.size() < 2 || !parseString(commands[1], index)) {
        return unprocessedString;
    }
    auto iter = origDuneText.find(commands[0]);

    if (iter != origDuneText.end()) {

        IndexedTextFile* pIndexedTextFile = iter->second.get();

        if (commands[0].compare(0, 5, "DUNE.") && (index >= 281) && pIndexedTextFile->getNumStrings() == 335) {

            // Dune II 1.0 has 2 titles less

            index = std::max((int)281, index - 2);
        }

        std::map<std::string, std::string> mapping;

        for (unsigned int i = 2; i < commands.size(); i++) {

            std::vector<std::string> parts = splitStringToStringVector(commands[i], "->");

            mapping[parts[0]] = parts[1];
        }

        localizedString[unprocessedString] = replaceAll(pIndexedTextFile->getString(index), mapping);

        return localizedString[unprocessedString];

    } else {

        return unprocessedString;
    }

    return unprocessedString;
}

void TextManager::addOrigDuneText(const std::string& filename, bool bDecode) {
    origDuneText[filename] = std::make_unique<IndexedTextFile>(pFileManager->openFile(filename).get(), bDecode);
}
