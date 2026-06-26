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

#ifndef VIEW_PUBLISHEDFRAMEHANDOFF_H
#define VIEW_PUBLISHEDFRAMEHANDOFF_H

#include <View/PublishedFrame.h>

/// Triple-buffered handoff for published frames: front (UI reading), queued (next ready),
/// write (engine populating).
///
/// Single-threaded semantics: beginWrite() returns reference to the write slot for engine
/// population. publishWrittenFrame() advances the buffer ring (write -> queued, queued -> front).
/// tryAcquireNextForUi() lets UI check if a new frame is available.
class PublishedFrameHandoff {
public:
    PublishedFrameHandoff() = default;

    /// Returns mutable reference to the write slot for engine to populate.
    /// The returned frame has been reset() for reuse.
    PublishedFrame& beginWrite() noexcept;

    /// Publishes the written frame, advancing ring: write -> queued, queued -> front, front -> write.
    /// Resets the newly-assigned write frame for next cycle.
    void publishWrittenFrame() noexcept;

    /// Attempts to acquire the next frame for UI consumption.
    /// Returns non-null if a new frame is available (front frame is updated to queued).
    /// Returns nullptr if no new frame is queued (front frame unchanged).
    const PublishedFrame* tryAcquireNextForUi() noexcept;

    /// Returns const reference to the current front frame (UI-facing read).
    const PublishedFrame& getCurrentFrame() const noexcept { return frames[static_cast<int>(slot_front)]; }

private:
    enum class SlotIndex { Front = 0, Queued = 1, Write = 2 };

    PublishedFrame frames[3];
    SlotIndex slot_front{SlotIndex::Front};
    SlotIndex slot_queued{SlotIndex::Queued};
    SlotIndex slot_write{SlotIndex::Write};
};

#endif // VIEW_PUBLISHEDFRAMEHANDOFF_H
