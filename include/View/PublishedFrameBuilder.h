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

#ifndef VIEW_PUBLISHEDFRAMEBUILDER_H
#define VIEW_PUBLISHEDFRAMEBUILDER_H

#include <View/PublishedFrame.h>

class Game;

/// Builds one PublishedFrame per simulation tick from live simulation state.
///
/// Holds a single reusable PublishedFrame buffer: reset() and reserve APIs
/// from PublishedFrame/PlayerView are used to avoid per-tick allocations.
/// Call buildFrame() once per tick after the simulation update completes.
class PublishedFrameBuilder {
public:
    PublishedFrameBuilder()  = default;
    ~PublishedFrameBuilder() = default;

    PublishedFrameBuilder(const PublishedFrameBuilder&)            = delete;
    PublishedFrameBuilder& operator=(const PublishedFrameBuilder&) = delete;
    PublishedFrameBuilder(PublishedFrameBuilder&&)                 = delete;
    PublishedFrameBuilder& operator=(PublishedFrameBuilder&&)      = delete;

    /// Rebuild the frame buffer from current simulation state.
    ///
    /// For each active house, applies per-team visibility filtering across all
    /// live units and structures, then stamps the current tick. Uses reset()
    /// on all inner vectors so no capacity is released between ticks.
    void buildFrame(const Game& game);

    /// Returns the most recently built frame.
    /// Valid after the first call to buildFrame(); the reference is stable for
    /// the lifetime of this builder.
    [[nodiscard]] const PublishedFrame& getLatestFrame() const noexcept { return frame_; }

private:
    PublishedFrame frame_;
};

#endif // VIEW_PUBLISHEDFRAMEBUILDER_H
