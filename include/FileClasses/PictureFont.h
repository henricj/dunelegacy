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

#ifndef PICTUREFONT_H
#define PICTUREFONT_H

#include "Font.h"

#include <vector>
#include <SDL.h>

/// A class for loading a font from a surface.
/**
	This class can read a font from a surface.
*/
class PictureFont : public Font
{
private:
	// Internal structure used for storing the character data
	struct FontCharacter
	{
		int	width;
		std::vector<char> data;
	};

public:
	PictureFont(SDL_Surface* pic, int freesrc);
	virtual ~PictureFont();

	void drawTextOnSurface(SDL_Surface* pSurface, std::string text, unsigned char baseColor = 128);

	int	getTextWidth(std::string text) const;

	/// Returns the number of pixels this font needs in y-direction.
	/**
		This methods returns the height of this font.
		\return Number of pixels needed
	*/
	inline int getTextHeight() const { return characterHeight; };

private:
	FontCharacter character[256];
	Uint8 characterHeight;
};

#endif //PICTUREFONT_H
