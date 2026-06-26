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

#include <View/PublishedFrameHandoff.h>

#include <utility>

PublishedFrame& PublishedFrameHandoff::beginWrite() noexcept {
    auto& write_frame = frames[static_cast<int>(slot_write)];
    write_frame.reset();
    return write_frame;
}

void PublishedFrameHandoff::publishWrittenFrame() noexcept {
    // Rotate: write -> queued -> front -> write (triple buffer ring)
    SlotIndex tmp = slot_write;
    slot_write    = slot_front;
    slot_front    = slot_queued;
    slot_queued   = tmp;

    // Reset the newly-assigned write frame for next cycle
    frames[static_cast<int>(slot_write)].reset();
}

const PublishedFrame* PublishedFrameHandoff::tryAcquireNextForUi() noexcept {
    // Check if queued frame is newer than front frame
    if (frames[static_cast<int>(slot_queued)].tick > frames[static_cast<int>(slot_front)].tick) {
        // Promote queued to front (swap indices)
        std::swap(slot_front, slot_queued);
        return &frames[static_cast<int>(slot_front)];
    }
    return nullptr;
}
