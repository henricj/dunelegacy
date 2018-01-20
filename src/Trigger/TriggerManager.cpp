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
    for(const std::shared_ptr<Trigger>& pTrigger : triggers) {
        saveTrigger(stream, pTrigger);
    }
}

void TriggerManager::load(InputStream& stream) {
    Uint32 numTriggers = stream.readUint32();

    for(Uint32 i=0;i<numTriggers;i++) {
        triggers.push_back(loadTrigger(stream));
    }
}

void TriggerManager::trigger(Uint32 CycleNumber)
{
    while((triggers.empty() == false) && (triggers.front()->getCycleNumber() == CycleNumber)) {
        std::shared_ptr<Trigger> pCurrentTrigger = triggers.front();
        triggers.pop_front();
        pCurrentTrigger->trigger();
    }
}

void TriggerManager::addTrigger(std::shared_ptr<Trigger> newTrigger)
{
    for(auto iter = triggers.begin(); iter != triggers.end(); ++iter) {
        if((*iter)->getCycleNumber() > newTrigger->getCycleNumber()) {
            triggers.insert(iter, newTrigger);
            return;
        }
    }

    triggers.push_back(newTrigger);
}

void TriggerManager::saveTrigger(OutputStream& stream, const std::shared_ptr<Trigger>& t) const
{
    if(dynamic_cast<ReinforcementTrigger*>(t.get())) {
        stream.writeUint32(TriggerManager::Type_ReinforcementTrigger);
        t->save(stream);
    } else if(dynamic_cast<TimeoutTrigger*>(t.get())) {
        stream.writeUint32(TriggerManager::Type_TimeoutTrigger);
        t->save(stream);
    }
}

std::shared_ptr<Trigger> TriggerManager::loadTrigger(InputStream& stream)
{
    Uint32 type = stream.readUint32();

    switch(type) {
        case TriggerManager::Type_ReinforcementTrigger: {
            return std::shared_ptr<Trigger>(new ReinforcementTrigger(stream));
        } break;

        case TriggerManager::Type_TimeoutTrigger: {
            return std::shared_ptr<Trigger>(new TimeoutTrigger(stream));
        } break;

        default: {
            THROW(std::runtime_error, "TriggerManager::loadTrigger(): Unknown trigger type!");
        }
    }
}
