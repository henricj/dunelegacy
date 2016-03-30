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

#ifndef LABEL_H
#define LABEL_H

#include "Widget.h"
#include "GUIStyle.h"
#include <misc/draw_util.h>
#include <SDL.h>
#include <string>
#include <list>

/// a class for a text label
class Label : public Widget {
public:
	/// default constructor
	Label() : Widget() {
        fontID = FONT_STD12;
		textcolor = COLOR_DEFAULT;
		textshadowcolor = COLOR_DEFAULT;
		backgroundcolor = 0;
		alignment = (Alignment_Enum) (Alignment_Left | Alignment_VCenter);
		pTexture = NULL;
		enableResizing(true,true);
	}

	/// destructor
	virtual ~Label() {
		if(pTexture != NULL) {
			SDL_DestroyTexture(pTexture);
			pTexture = NULL;
		}
	}

    /**
		Sets a font for this label. Default font of a label is FONT_STD12
		\param	fontID	    the ID of the new font
	*/
	virtual inline void setTextFont(int fontID) {
	    this->fontID = fontID;
	    resize(getSize().x, getSize().y);
	}

    /**
		Gets the font of this label. Default font of a label is FONT_STD12
		\return the font ID of this label
	*/
	virtual inline int getTextFont() const {
	   return fontID;
	}

	/**
		Sets the text color for this label.
		\param	textcolor	    the color of the text (COLOR_DEFAULT = default color)
        \param	textshadowcolor	the color of the shadow of the text (COLOR_DEFAULT = default color)
        \param  backgroundcolor the color of the label background (COLOR_TRANSPARENT = transparent)
	*/
	virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) {
		this->textcolor = textcolor;
		this->textshadowcolor = textshadowcolor;
		this->backgroundcolor = backgroundcolor;
		resize(getSize().x, getSize().y);
	}

	/**
		Sets the alignment of the text in this label.
		\param alignment Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
	*/
	virtual inline void setAlignment(Alignment_Enum alignment) {
		this->alignment = alignment;
		resize(getSize().x, getSize().y);
	}

	/**
		Returns the alignment of the text in this label.
		\return Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
	*/
	virtual inline Alignment_Enum getAlignment() const {
		return alignment;
	}

	/**
		This method sets a new text for this label and resizes this label
		to fit this text.
		\param	Text The new text for this button
	*/
	virtual inline void setText(std::string text) {
		this->text = text;
		resizeAll();
	}

	/**
		Get the text of this label.
		\return the text of this button
	*/
	inline std::string getText() const { return text; };

	/**
		This method resized the label to width and height. This method should only
		called if the new size is a valid size for this label (See getMinumumSize).
		\param	width	the new width of this label
		\param	height	the new height of this label
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		if(pTexture != NULL) {
			SDL_DestroyTexture(pTexture);
			pTexture = NULL;
		}

		//split text into single lines at every '\n'
		size_t startpos = 0;
		size_t nextpos;
		std::list<std::string> hardLines;
		do {
			nextpos = text.find("\n",startpos);
			if(nextpos == std::string::npos) {
				hardLines.push_back(text.substr(startpos,text.length()-startpos));
			} else {
				hardLines.push_back(text.substr(startpos,nextpos-startpos));
				startpos = nextpos+1;
			}
		} while(nextpos != std::string::npos);

		std::list<std::string> textLines;

		std::list<std::string>::const_iterator iter;
		for(iter = hardLines.begin();iter != hardLines.end();++iter) {
			std::string tmpLine = *iter;

			if(tmpLine == "") {
                textLines.push_back(" ");
                continue;
			}

			bool bEndOfLine = false;
			size_t warppos = 0;
			size_t oldwarppos = 0;
			size_t lastwarp = 0;

			while(bEndOfLine == false) {
				while(true) {
					warppos = tmpLine.find(" ", oldwarppos);
					std::string tmp;
					if(warppos == std::string::npos) {
						tmp = tmpLine.substr(lastwarp,tmpLine.length()-lastwarp);
						warppos = tmpLine.length();
						bEndOfLine = true;
					} else {
						tmp = tmpLine.substr(lastwarp,warppos-lastwarp);
					}

					if( (int) GUIStyle::getInstance().getMinimumLabelSize(tmp, fontID).x - 4 > (int) width) {
						// this line would be too big => in oldwarppos is the last correct warp pos
						bEndOfLine = false;
						break;
					} else {
					    if(bEndOfLine == true) {
                            oldwarppos = warppos;
                            break;
					    } else {
                            oldwarppos = warppos + 1;
					    }
					}
				}

				if(oldwarppos == lastwarp) {
					// the width of this label is too small for the next word
					// split the word

					warppos = lastwarp;
					while(true) {
						std::string tmp = tmpLine.substr(lastwarp,warppos-lastwarp);
						if( (int) GUIStyle::getInstance().getMinimumLabelSize(tmp, fontID).x - 4 > (int) width) {
							// this line would be too big => in oldwarppos is the last correct warp pos
							break;
						} else {
							oldwarppos = warppos;
						}

						warppos++;

						if(warppos > tmpLine.length()) {
                            oldwarppos = tmpLine.length();
                            break;
						}
					}

					if(warppos != lastwarp) {
						textLines.push_back(tmpLine.substr(lastwarp,oldwarppos-lastwarp));
						lastwarp = oldwarppos;
					} else {
						// the width of this label is too small for the next character
						// create a dummy entry
						textLines.push_back(" ");
						lastwarp++;
						oldwarppos++;
					}
				} else {
					std::string tmpStr = tmpLine.substr(lastwarp,oldwarppos-lastwarp);
					textLines.push_back(tmpStr);
					lastwarp = oldwarppos;
				}
			}
		}


		pTexture = convertSurfaceToTexture(GUIStyle::getInstance().createLabelSurface(width,height,textLines,fontID,alignment,textcolor,textshadowcolor,backgroundcolor), true);
		Widget::resize(width,height);
	}

	/**
		Returns the minimum size of this label. The label should not
		resized to a size smaller than this.
		\return the minimum size of this label
	*/
	virtual Point getMinimumSize() const {
		Point p(0,0);

		//split text into single lines at every '\n'
		size_t startpos = 0;
		size_t nextpos;
		std::list<std::string> hardLines;
		do {
			nextpos = text.find("\n",startpos);
			if(nextpos == std::string::npos) {
				hardLines.push_back(text.substr(startpos,text.length()-startpos));
			} else {
				hardLines.push_back(text.substr(startpos,nextpos-startpos));
				startpos = nextpos+1;
			}
		} while(nextpos != std::string::npos);

		std::list<std::string>::const_iterator iter;
		for(iter = hardLines.begin();iter != hardLines.end();++iter) {
			std::string tmp = *iter;
			p.x = std::max(p.x, GUIStyle::getInstance().getMinimumLabelSize(tmp, fontID).x);
			p.y += GUIStyle::getInstance().getMinimumLabelSize(tmp, fontID).y;
		}
		return p;
	}

	/**
		Draws this label to screen.
		\param	screen	Surface to draw on
		\param	position	Position to draw the label to
	*/
	virtual void draw(SDL_Surface* screen, Point position) {
		if((isEnabled() == false) || (isVisible() == false) || (pTexture == NULL)) {
			return;
		}

        SDL_Rect dest = calcDrawingRect(pTexture, position.x + getSize().x/2, position.y + getSize().y/2, HAlign::Center, VAlign::Center);
		SDL_RenderCopy(renderer, pTexture, NULL, &dest);
	};

	/**
		This static method creates a dynamic label object with Text as the label text.
		The idea behind this method is to simply create a new text label on the fly and
		add it to a container. If the container gets destroyed also this label will be freed.
		\param	text	The label text
       \param	textcolor	    the color of the text (COLOR_DEFAULT = default color)
       \param	textshadowcolor	the color of the shadow of the text (COLOR_DEFAULT = default color)
       \param  backgroundcolor the color of the label background (COLOR_TRANSPARENT = transparent)
		\return	The new created label (will be automatically destroyed when it's parent widget is destroyed)
	*/
	static Label* create(std::string text, Uint32 textcolor = COLOR_DEFAULT, Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) {
		Label* label = new Label();
		label->setText(text);
		label->setTextColor(textcolor, textshadowcolor, backgroundcolor);
		label->pAllocated = true;
		return label;
	}

private:
    int fontID;                 ///< the ID of the font to use
	Uint32 textcolor;	        ///< the text color
	Uint32 textshadowcolor;     ///< the color of the shadow of the text
	Uint32 backgroundcolor;     ///< the color of the label background
	std::string text;			///< the text of this label
	SDL_Texture* pTexture;		///< the texture of this label
	Alignment_Enum alignment;	///< the alignment of this label
};

#endif // LABEL_H
