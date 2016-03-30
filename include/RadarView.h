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

#ifndef RADARVIEW_H
#define RADARVIEW_H

#include <RadarViewBase.h>

#include <DataTypes.h>

#include <SDL.h>


/// This class manages the mini map at the top right corner of the screen
class RadarView : public RadarViewBase
{
public:
	/**
        Constructor
	*/
	RadarView();

	/**
        Destructor
	*/
    virtual ~RadarView();

    /**
        Get the map size in x direction
        \return map width
    */
    virtual int getMapSizeX() const;

    /**
        Get the map size in y direction
        \return map height
    */
    virtual int getMapSizeY() const;


	/**
		Draws the radar to screen. This method is called before drawOverlay().
		\param	screen	Surface to draw on
		\param	Position	Position to draw the radar to
	*/
	virtual void draw(SDL_Surface* screen, Point position);


	/**
        This method updates the radar. It should be called every game tick
	*/
    void update();


    /**
        This method sets the radar mode directly. To show the static animation use switchRadarMode().
        \param bStatus  true = switches the radar on, false = switches the radar off
	*/
	void setRadarMode(bool bStatus) {
        if(bStatus == true) {
            currentRadarMode = Mode_RadarOn;
            animFrame = 0;
            animCounter = NUM_STATIC_FRAME_TIME;
        } else {
            currentRadarMode = Mode_RadarOff;
            animFrame = NUM_STATIC_FRAMES - 1;
            animCounter = NUM_STATIC_FRAME_TIME;
        }
    }

    /**
        This method switches the radar on or off
        \param bOn  true = switches the radar on, false = switches the radar off
	*/
	void switchRadarMode(bool bOn);

private:

    void updateRadarSurface(int mapSizeX, int mapSizeY, int scale, int offsetX, int offsetY);

	RadarViewMode currentRadarMode;         ///< the current mode of the radar

	int animFrame;                          ///< the current animation frame

	int animCounter;                        ///< this counter is for counting the ticks one animation frame is shown

    SDL_Surface* radarSurface;              ///< contains the image to be drawn when the radar is active
    SDL_Texture* radarTexture;              ///< streaming texture to be used when the radar is active
	SDL_Texture* radarStaticAnimation;      ///< holds the animation graphic for radar static

};

#endif // RADARVIEW_H
