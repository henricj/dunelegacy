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

#ifndef ANIMATIONLABEL_H
#define ANIMATIONLABEL_H

#include <FileClasses/Animation.h>
#include <GUI/Widget.h>

/// A widget for showing an animation
class AnimationLabel final : public Widget {
public:
    /// default constructor
    AnimationLabel()
        : pAnim(nullptr) {
        enableResizing(false, false);
    }

    /// destructor
    ~AnimationLabel() override = default;

    /**
        Set the current animation that should be shown in this widget.
        \param newAnimation the new animation to show
    */
    void setAnimation(Animation* newAnimation) {
        pAnim = newAnimation;
    }

    /**
        This method returns the stored animation.
        \return the stored animation
    */
    [[nodiscard]] Animation* getAnimation() const { return pAnim; }

    /**
        Draws this widget to screen. This method is called before drawOverlay().
        \param  position    Position to draw the widget to
    */
    void draw(Point position) override {
        if (pAnim == nullptr) {
            return;
        }

        SDL_Texture* tex = pAnim->getFrameTexture();

        if (isVisible()) {
            const SDL_Rect dest = calcDrawingRect(tex, position.x, position.y);
            Dune_RenderCopy(renderer, tex, nullptr, &dest);
        }
    }

    /**
        Returns the minimum size of this animation label. The widget should not
        be resized to a size smaller than this.
        \return the minimum size of this animation label
    */
    [[nodiscard]] Point getMinimumSize() const override {
        SDL_Surface* surface = pAnim->getFrame();

        if (surface != nullptr) {
            return {surface->w, surface->h};
        }

        return {0, 0};
    }

private:
    Animation* pAnim;
};

#endif // ANIMATIONLABEL_H
