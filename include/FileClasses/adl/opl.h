/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * opl.h - OPL base class, by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_OPL
#define H_ADPLUG_OPL

class Copl
{
 public:
  typedef enum {
    TYPE_OPL2, TYPE_OPL3, TYPE_DUAL_OPL2
  } ChipType;

  Copl() noexcept
    : currChip(0), currType(TYPE_OPL2)
    {
    }

  Copl(const Copl &) = delete;
  Copl(Copl &&) = delete;
  Copl& operator=(const Copl &) = delete;
  Copl& operator=(Copl &&) = delete;

  virtual ~Copl() = default;

  virtual void write(int reg, int val) = 0; // combined register select + data write
  virtual void setchip(int n) noexcept      // select OPL chip
    {
      if(n < 2)
    currChip = n;
    }

  virtual int getchip() const noexcept            // returns current OPL chip
    {
      return currChip;
    }

  virtual void init(void) = 0;          // reinitialize OPL chip(s)

  // return this OPL chip's type
  ChipType gettype() const noexcept
    {
      return currType;
    }

  // Emulation only: fill buffer
  virtual void update(short *buf, int samples) {}

 protected:
  int       currChip;       // currently selected OPL chip number
  ChipType  currType;       // this OPL chip's type
};

#endif
