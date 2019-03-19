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

#include <Trigger/TriggerManager.h>
#include <Trigger/ReinforcementTrigger.h>
#include <Trigger/TimeoutTrigger.h>

#include <misc/exceptions.h>

TriggerManager::TriggerManager() = default;

TriggerManager::~TriggerManager() = default;

void TriggerManager::save(OutputStream& stream) const {
    stream.writeUint32(triggers.size());
    for(const auto& pTrigger : triggers) {
        saveTrigger(stream, pTrigger.get());
    }
}

void TriggerManager::load(InputStream& stream) {
    const auto numTriggers = stream.readUint32();

    for (auto i = 0u; i < numTriggers; i++) {
        triggers.push_back(loadTrigger(stream));
    }
}

void TriggerManager::trigger(Uint32 CycleNumber)
{
    if (triggers.empty()) return;

    auto clear = true;

    for (auto it = std::begin(triggers); it != std::end(triggers); ++it)
    {
        if ((*it)->getCycleNumber() != CycleNumber)
        {
            if (it != std::begin(triggers))
                triggers.erase(std::begin(triggers), it);

            clear = false;

            break;
        }

        active_trigger.push_back(std::move(*it));
    }

    if (clear)
        triggers.clear();

    for (auto& t : active_trigger)
        t->trigger();

    active_trigger.clear();
}

void TriggerManager::addTrigger(std::unique_ptr<Trigger> newTrigger)
{
    for(auto iter = triggers.begin(); iter != triggers.end(); ++iter) {
        if((*iter)->getCycleNumber() > newTrigger->getCycleNumber()) {
            triggers.insert(iter, std::move(newTrigger));
            return;
        }
    }

    triggers.push_back(std::move(newTrigger));
}

void TriggerManager::saveTrigger(OutputStream& stream, const Trigger* t)
{
    if(dynamic_cast<const ReinforcementTrigger*>(t)) {
        stream.writeUint32(TriggerManager::Type_ReinforcementTrigger);
        t->save(stream);
    } else if(dynamic_cast<const TimeoutTrigger*>(t)) {
        stream.writeUint32(TriggerManager::Type_TimeoutTrigger);
        t->save(stream);
    }
}

std::unique_ptr<Trigger> TriggerManager::loadTrigger(InputStream& stream) const
{
    const auto type = stream.readUint32();

    switch(type) {
        case TriggerManager::Type_ReinforcementTrigger: {
            return std::make_unique<ReinforcementTrigger>(stream);
        } break;

        case TriggerManager::Type_TimeoutTrigger: {
            return std::make_unique<TimeoutTrigger>(stream);
        } break;

    default:
        THROW(std::runtime_error, "TriggerManager::loadTrigger(): Unknown trigger type!");
    }
}
