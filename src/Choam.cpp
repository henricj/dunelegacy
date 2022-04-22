#include <Choam.h>

#include <Game.h>
#include <House.h>
#include <globals.h>

#include <FileClasses/TextManager.h>

#include <algorithm>

// change starport prices every minute
constexpr auto CHOAM_CHANGE_PRICETIME = MILLI2CYCLES(60 * 1000);

// change amount of available units every 30s
constexpr auto CHOAM_CHANGE_AMOUNT = MILLI2CYCLES(30 * 1000);

Choam::Choam(House* pHouse) : house(pHouse) { }

Choam::~Choam() = default;

void Choam::save(OutputStream& stream) const {
    stream.writeUint32(availableItems.size());
    for (const BuildItem& buildItem : availableItems) {
        buildItem.save(stream);
    }
}

void Choam::load(InputStream& stream) {
    const uint32_t num = stream.readUint32();
    for (uint32_t i = 0; i < num; i++) {
        BuildItem tmp;
        tmp.load(stream);
        availableItems.push_back(tmp);
    }
}

int Choam::getPrice(ItemID_enum itemID) const {
    for (const BuildItem& buildItem : availableItems) {
        if (buildItem.itemID == itemID) {
            return buildItem.price;
        }
    }

    return 0;
}

bool Choam::isCheap(ItemID_enum itemID) const {
    return (getPrice(itemID) < currentGame->objectData.data[itemID][static_cast<int>(house->getHouseID())].price
                                   * 1.3_fix); // A bit of logic to make starports better
}

int Choam::getNumAvailable(ItemID_enum itemID) const {
    for (const BuildItem& buildItem : availableItems) {
        if (buildItem.itemID == itemID) {
            return buildItem.num;
        }
    }

    return INVALID;
}

bool Choam::setNumAvailable(ItemID_enum itemID, int newValue) {
    for (BuildItem& buildItem : availableItems) {
        if (buildItem.itemID == itemID) {
            buildItem.num = newValue;
            return (buildItem.num > 0);
        }
    }

    return false;
}

void Choam::addItem(ItemID_enum itemID, int num) {
    BuildItem tmp;
    tmp.itemID = itemID;
    tmp.num    = num;
    availableItems.push_back(tmp);
}

void Choam::update(const GameContext& context) {
    if (availableItems.empty()) {
        return;
    }

    auto& game = context.game;

    if ((game.getGameCycleCount() % CHOAM_CHANGE_AMOUNT) == 0) {
        const int index           = game.randomGen.rand(0u, availableItems.size() - 1);
        availableItems[index].num = std::min(availableItems[index].num + 1, 10u);
    }

    if ((game.getGameCycleCount() % CHOAM_CHANGE_PRICETIME) == 0) {
        for (BuildItem& buildItem : availableItems) {
            int price = game.objectData.data[buildItem.itemID][static_cast<int>(house->getHouseID())].price;

            constexpr int min_mod = 2;
            constexpr int max_mod = 8;

            const int rand1 = game.randomGen.rand(min_mod, max_mod);
            const int rand2 = game.randomGen.rand(min_mod, max_mod);

            price = std::min((rand1 + rand2) * (price / 10), 999);

            buildItem.price = price;
        }

        if ((pLocalHouse == house) && (house->hasStarPort())) {
            game.addToNewsTicker(_("New Starport prices"));
        }
    }
}
