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

#include <Menu/MentatMenu.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <mmath.h>

#include <algorithm>
#include <regex>

MentatMenu::MentatMenu(HOUSETYPE newHouse)
    : nextSpecialAnimation(dune::dune_clock::now()
                           + dune::as_dune_clock_duration(dune::globals::pGFXManager->random().rand(8000, 20000))),
      house(newHouse) {

    disableQuitting(true);

    // set up window
    const DuneTexture* pBackground = nullptr;

    auto* const gfx = dune::globals::pGFXManager.get();

    if (house == HOUSETYPE::HOUSE_INVALID) {
        pBackground = gfx->getUIGraphic(UI_MentatBackgroundBene);
    } else {
        pBackground = gfx->getUIGraphic(UI_MentatBackground, house);
    }

    setBackground(pBackground);

    MentatMenu::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    MentatMenu::setWindowWidget(&windowWidget);

    switch (house) {
        case HOUSETYPE::HOUSE_HARKONNEN: {
            auto* anim = gfx->getAnimation(Anim_HarkonnenEyes);
            eyesAnim.setAnimation(anim);
            windowWidget.addWidget(&eyesAnim, Point(64, 176), eyesAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_HarkonnenMouth);
            mouthAnim.setAnimation(anim);
            windowWidget.addWidget(&mouthAnim, Point(64, 208), mouthAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_HarkonnenShoulder);
            shoulderAnim.setAnimation(anim);
            // don't add shoulderAnim, draw it in DrawSpecificStuff
        } break;

        case HOUSETYPE::HOUSE_ATREIDES: {
            auto* anim = gfx->getAnimation(Anim_AtreidesEyes);
            eyesAnim.setAnimation(anim);
            windowWidget.addWidget(&eyesAnim, Point(80, 160), eyesAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_AtreidesMouth);
            mouthAnim.setAnimation(anim);
            windowWidget.addWidget(&mouthAnim, Point(80, 192), mouthAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_AtreidesBook);
            specialAnim.setAnimation(anim);
            windowWidget.addWidget(&specialAnim, Point(145, 305), specialAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_AtreidesShoulder);
            shoulderAnim.setAnimation(anim);
            // don't add shoulderAnim, draw it in DrawSpecificStuff
        } break;

        case HOUSETYPE::HOUSE_ORDOS: {
            auto* anim = gfx->getAnimation(Anim_OrdosEyes);
            eyesAnim.setAnimation(anim);
            windowWidget.addWidget(&eyesAnim, Point(32, 160), eyesAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_OrdosMouth);
            mouthAnim.setAnimation(anim);
            windowWidget.addWidget(&mouthAnim, Point(32, 192), mouthAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_OrdosRing);
            specialAnim.setAnimation(anim);
            specialAnim.getAnimation()->setCurrentFrameNumber(specialAnim.getAnimation()->getNumberOfFrames() - 1);
            windowWidget.addWidget(&specialAnim, Point(178, 289), specialAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_OrdosShoulder);
            shoulderAnim.setAnimation(anim);
            // don't add shoulderAnim, draw it in DrawSpecificStuff
        } break;

        case HOUSETYPE::HOUSE_FREMEN: {
            auto* anim = gfx->getAnimation(Anim_FremenEyes);
            eyesAnim.setAnimation(anim);
            windowWidget.addWidget(&eyesAnim, Point(80, 160), eyesAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_FremenMouth);
            mouthAnim.setAnimation(anim);
            windowWidget.addWidget(&mouthAnim, Point(80, 192), mouthAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_FremenBook);
            specialAnim.setAnimation(anim);
            windowWidget.addWidget(&specialAnim, Point(145, 305), specialAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_FremenShoulder);
            shoulderAnim.setAnimation(anim);
            // don't add shoulderAnim, draw it in DrawSpecificStuff
        } break;

        case HOUSETYPE::HOUSE_SARDAUKAR: {
            auto* anim = gfx->getAnimation(Anim_SardaukarEyes);
            eyesAnim.setAnimation(anim);
            windowWidget.addWidget(&eyesAnim, Point(64, 176), eyesAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_SardaukarMouth);
            mouthAnim.setAnimation(anim);
            windowWidget.addWidget(&mouthAnim, Point(64, 208), mouthAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_SardaukarShoulder);
            shoulderAnim.setAnimation(anim);
            // don't add shoulderAnim, draw it in DrawSpecificStuff
        } break;

        case HOUSETYPE::HOUSE_MERCENARY: {
            auto* anim = gfx->getAnimation(Anim_MercenaryEyes);
            eyesAnim.setAnimation(anim);
            windowWidget.addWidget(&eyesAnim, Point(32, 160), eyesAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_MercenaryMouth);
            mouthAnim.setAnimation(anim);
            windowWidget.addWidget(&mouthAnim, Point(32, 192), mouthAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_MercenaryRing);
            specialAnim.setAnimation(anim);
            specialAnim.getAnimation()->setCurrentFrameNumber(specialAnim.getAnimation()->getNumberOfFrames() - 1);
            windowWidget.addWidget(&specialAnim, Point(178, 289), specialAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_MercenaryShoulder);
            shoulderAnim.setAnimation(anim);
            // don't add shoulderAnim, draw it in DrawSpecificStuff
        } break;

        default: {
            // bene gese
            auto* anim = gfx->getAnimation(Anim_BeneEyes);
            eyesAnim.setAnimation(anim);
            windowWidget.addWidget(&eyesAnim, Point(128, 160), eyesAnim.getMinimumSize());

            anim = gfx->getAnimation(Anim_BeneMouth);
            mouthAnim.setAnimation(anim);
            windowWidget.addWidget(&mouthAnim, Point(112, 192), mouthAnim.getMinimumSize());
        } break;
    }

    textLabel.setTextColor(COLOR_WHITE, COLOR_TRANSPARENT);
    textLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_Left | Alignment_Top));
    textLabel.setVisible(false);
}

MentatMenu::~MentatMenu() = default;

void MentatMenu::setText(const std::string& text) {
    using namespace std::chrono_literals;

    const std::regex rgx(R"([^\.\!\?]*[\.\!\?]\s?)");
    mentatTexts = std::vector<std::string>(std::sregex_token_iterator(text.begin(), text.end(), rgx),
                                           std::sregex_token_iterator());
    if (mentatTexts.empty()) {
        mentatTexts.push_back(text);
    }

    mouthAnim.getAnimation()->setNumLoops(mentatTexts[0].empty() ? 0
                                                                 : static_cast<int>(mentatTexts[0].length() / 25) + 1);
    textLabel.setText(mentatTexts[0]);
    textLabel.setVisible(true);
    textLabel.resize(620, 240);

    currentMentatTextIndex = 0;
    nextMentatTextSwitch =
        dune::dune_clock::now() + dune::as_dune_clock_duration(mentatTexts[0].length()) * 75 + 1000ms;
}

void MentatMenu::update() {
    using namespace std::chrono_literals;

    // speedup blink of the eye
    eyesAnim.getAnimation()->setFrameRate(eyesAnim.getAnimation()->getCurrentFrameNumber() == MentatEyesClosed ? 4.0
                                                                                                               : 0.5);

    if (dune::dune_clock::now() > nextMentatTextSwitch) {
        currentMentatTextIndex++;

        std::string text;
        if (currentMentatTextIndex >= static_cast<int>(mentatTexts.size())) {
            onMentatTextFinished();
            nextMentatTextSwitch = dune::dune_clock::time_point::max();
        } else {
            text = mentatTexts[currentMentatTextIndex];
            if (text.empty()) {
                onMentatTextFinished();
                nextMentatTextSwitch   = dune::dune_clock::time_point::max();
                currentMentatTextIndex = mentatTexts.size();
            } else {
                nextMentatTextSwitch =
                    dune::dune_clock::now() + dune::as_dune_clock_duration(text.length()) * 75 + 1000ms;
            }
        }

        mouthAnim.getAnimation()->setNumLoops(text.empty() ? 0 : text.length() / 25 + 1);

        textLabel.setText(text);
        textLabel.setVisible(true);
        textLabel.resize(620, 240);
    }

    auto* const gfx = dune::globals::pGFXManager.get();

    if (specialAnim.getAnimation() != nullptr && specialAnim.getAnimation()->isFinished()) {
        if (nextSpecialAnimation < dune::dune_clock::now()) {
            specialAnim.getAnimation()->setNumLoops(1);
            nextSpecialAnimation =
                dune::dune_clock::now() + dune::as_dune_clock_duration(gfx->random().rand(8000, 20000));
        }
    }

    const Point mouse(dune::globals::drawnMouseX - getPosition().x, dune::globals::drawnMouseY - getPosition().y);
    const bool bPressed = SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT);

    const Point eyesPos     = windowWidget.getWidgetPosition(&eyesAnim);
    const Point& eyesSize   = eyesAnim.getSize();
    const Point eyesCenter  = eyesPos + eyesSize / 2;
    const Point mouseEyePos = mouse - eyesCenter;

    eyesAnim.getAnimation()->resetFrameOverride();

    if (mouseEyePos.x >= -eyesSize.x / 2 - 30 && mouseEyePos.x <= -eyesSize.x / 2
        && mouseEyePos.y >= -eyesSize.y / 2 - 20 && mouseEyePos.y <= eyesSize.y / 2) {
        eyesAnim.getAnimation()->setFrameOverride(MentatEyesLeft);
    } else if (mouseEyePos.x <= eyesSize.x / 2 + 30 && mouseEyePos.x >= eyesSize.x / 2
               && mouseEyePos.y >= -eyesSize.y / 2 - 20 && mouseEyePos.y <= eyesSize.y / 2) {
        eyesAnim.getAnimation()->setFrameOverride(MentatEyesRight);
    } else if (abs(mouseEyePos.x) < eyesSize.x && mouseEyePos.y >= -eyesSize.y / 2 - 20
               && mouseEyePos.y <= eyesSize.y / 2) {
        eyesAnim.getAnimation()->setFrameOverride(MentatEyesNormal);
    } else if (abs(mouseEyePos.x) < eyesSize.x && mouseEyePos.y > eyesSize.y / 2
               && mouseEyePos.y <= eyesSize.y / 2 + 15) {
        eyesAnim.getAnimation()->setFrameOverride(MentatEyesDown);
    }

    if (bPressed && abs(mouseEyePos.x) <= eyesSize.x / 2 && abs(mouseEyePos.y) <= eyesSize.y / 2) {
        eyesAnim.getAnimation()->setFrameOverride(MentatEyesClosed);
    }

    const Point mouthPos      = windowWidget.getWidgetPosition(&mouthAnim);
    const Point& mouthSize    = mouthAnim.getSize();
    const Point mouthCenter   = mouthPos + mouthSize / 2;
    const Point mouseMouthPos = mouse - mouthCenter;

    if (bPressed) {
        if (abs(mouseMouthPos.x) <= mouthSize.x / 2 && abs(mouseMouthPos.y) <= mouthSize.y / 2) {
            if (mouthAnim.getAnimation()->getCurrentFrameOverride() == INVALID_FRAME) {
                mouthAnim.getAnimation()->setFrameOverride(
                    gfx->random().getRandOf(MentatMouthOpen1, MentatMouthOpen2, MentatMouthOpen3, MentatMouthOpen4));
            }
        } else {
            mouthAnim.getAnimation()->resetFrameOverride();
        }
    } else {
        mouthAnim.getAnimation()->resetFrameOverride();
    }
}

bool MentatMenu::doInput(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        showNextMentatText();
    }

    return parent::doInput(event);
}

void MentatMenu::drawSpecificStuff() {
    Point shoulderPos;
    switch (house) {
        case HOUSETYPE::HOUSE_HARKONNEN:
        case HOUSETYPE::HOUSE_SARDAUKAR: {
            shoulderPos = Point(256, 209) + getPosition();
        } break;

        case HOUSETYPE::HOUSE_ATREIDES:
        case HOUSETYPE::HOUSE_FREMEN:
        case HOUSETYPE::HOUSE_ORDOS:
        case HOUSETYPE::HOUSE_MERCENARY:
        default: {
            shoulderPos = Point(256, 257) + getPosition();
        } break;
    }

    shoulderAnim.draw(shoulderPos);
    textLabel.draw(Point(10, 5) + getPosition());
}

int MentatMenu::getMissionSpecificAnim(int missionnumber) {

    static constexpr int missionnumber2AnimID[] = {
        Anim_ConstructionYard, Anim_Harvester,    Anim_Radar, Anim_Quad,   Anim_Tank,
        Anim_RepairYard,       Anim_HeavyFactory, Anim_IX,    Anim_Palace, Anim_Sardaukar};

    if (missionnumber < 0 || missionnumber > 9) {
        return missionnumber2AnimID[0];
    }
    return missionnumber2AnimID[missionnumber];
}
