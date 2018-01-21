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

#ifndef DECODE_H
#define DECODE_H

/// A variant of memcpy that can handle overlapping memory areas.
/**
    Copies memory areas that may overlap byte by byte from small memory
    addresses to big memory addresses. Thus, already copied bytes can be
    copied again.
    \param  dst destination
    \param  src source
    \param  cnt length in bytes
*/
inline void memcpy_overlap(unsigned char *dst, const unsigned char *src, unsigned cnt);


/// Decompresses format40 compressed images/data.
/** Decompresses format40 compressed images/data specified by image_in to image_out.
    \param  image_in    format80 compressed data
    \param  image_out   pointer to output uncompressed data
    \return written bytes to image_out
 */
int decode40(const unsigned char *image_in, unsigned char *image_out);


/// Decompresses format80 compressed images/data.
/** Decompresses format80 compressed images/data specified by image_in to image_out. The checksum is also calculated and
    compared with the parameter checksum.
    \param  image_in    format80 compressed data
    \param  image_out   pointer to output uncompressed data
    \param  checksum    checksum for this file
    \return 0 if checksum is correct<br> -1 if checksum is incorrect
 */
int decode80(const unsigned char *image_in, unsigned char *image_out,unsigned checksum);

#endif // DECODE_H
