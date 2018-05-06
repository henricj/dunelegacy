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

#include <FileClasses/Decode.h>

#include <misc/exceptions.h>
#include <misc/SDL2pp.h>


void memcpy_overlap(unsigned char *dst, const unsigned char *src, unsigned cnt)
{
    /* Copies memory areas that may overlap byte by byte from small memory
     * addresses to big memory addresses. Thus, already copied bytes can be
     * copied again. */
    if (dst + cnt < src || src + cnt < dst) {
        memcpy(dst, src, cnt);
        return;
    }
    while (cnt--) {
        *dst = *src;
        dst++;
        src++;
    }
}


int decode40(const unsigned char *image_in, unsigned char *image_out)
{
    /*
    0 fill 00000000 c v
    1 copy 0ccccccc
    2 skip 10000000 c 0ccccccc
    3 copy 10000000 c 10cccccc
    4 fill 10000000 c 11cccccc v
    5 skip 1ccccccc
    */

    const unsigned char* readp = image_in;
    unsigned char* writep = image_out;
    Uint16 code;
    Uint16 count;
    while(1) {
        code = *readp++;
        if(~code & 0x80) {
            //bit 7 = 0
            if(!code) {
                //command 0 (00000000 c v): fill
                count = *readp++;
                code = *readp++;
                while (count--)
                    *writep++ ^= code;
            } else {
                //command 1 (0ccccccc): copy
                count = code;
                while (count--)
                    *writep++ ^= *readp++;
            }

        } else {
            //bit 7 = 1
            if(!(count = code & 0x7f)) {
                count =  SDL_SwapLE16(*((Uint16*)readp));
                readp += 2;
                code = count >> 8;
                if(~code & 0x80) {
                    //bit 7 = 0
                    //command 2 (10000000 c 0ccccccc): skip
                    if(!count) {
                        // end of image
                        break;
                    }
                    writep += count;
                } else {
                    //bit 7 = 1
                    count &= 0x3fff;
                    if(~code & 0x40) {
                        //bit 6 = 0
                        //command 3 (10000000 c 10cccccc): copy
                        while(count--) {
                            *writep++ ^= *readp++;
                        }
                    } else {
                        //bit 6 = 1
                        //command 4 (10000000 c 11cccccc v): fill
                        code = *readp++;
                        while(count--) {
                            *writep++ ^= code;
                        }
                    }
                }
            } else {
                //command 5 (1ccccccc): skip
                writep += count;
            }
        }
    }
    return (writep - image_out);
}

int decode80(const unsigned char *image_in, unsigned char *image_out, unsigned checksum)
{
    //
    // should decode all the format80 stuff ;-)
    //

    const unsigned char *readp = image_in;
    unsigned char *writep = image_out;

    unsigned int a = 0;
    unsigned int b = 0;
    unsigned int c = 0;
    unsigned int d = 0;
    unsigned int e = 0;
    unsigned int megacounta = 0;
    unsigned int megacountb = 0;
    unsigned int megacountc = 0;
    unsigned int megacountd = 0;
    unsigned int megacounte = 0;
    /*
       1 10cccccc
       2 0cccpppp p
       3 11cccccc p p
       4 11111110 c c v
       5 11111111 c c p p
     */

    while (1) {
        if ((*readp & 0xc0) == 0x80) {
            //
            // 10cccccc (1)
            //
            unsigned count = readp[0] & 0x3f;
            //SDL_Log("Cmd 1, count: %d", count);
            megacounta += count;
            if (!count) {
                break;
            }
            readp++;
            memcpy_overlap(writep, readp, count);
            readp += count;
            writep += count;
            a++;
        } else if ((*readp & 0x80) == 0x00) {
            //
            // 0cccpppp p (2)
            //
            unsigned count = ((readp[0] & 0x70) >> 4) + 3;
            unsigned short relpos = (((unsigned short) (readp[0] & 0xf)) << 8) | ((unsigned short) readp[1]);
            readp += 2;
            megacountb += count;
            memcpy_overlap(writep, writep - relpos, count);
            writep += count;
            b++;
        } else if (*readp == 0xff) {
            //
            // 11111111 c c p p (5)
            //
            unsigned short count = SDL_SwapLE16(*((unsigned short *) (readp + 1)));
            unsigned short pos = SDL_SwapLE16(*((unsigned short *) (readp + 3)));
            readp += 5;
            megacounte += count;
            memcpy_overlap(writep, image_out + pos, count);
            writep += count;
            e++;
        } else if (*readp == 0xfe) {
            //
            // 11111110 c c v(4)
            //
            unsigned short count = SDL_SwapLE16(*((unsigned short *) (readp + 1)));
            unsigned char color = readp[3];
            readp += 4;
            memset(writep, color, count);
            writep += count;
            megacountd += count;
            d++;
        } else if ((*readp & 0xc0) == 0xc0) {
            //
            // 11cccccc p p (3)
            //
            unsigned short count = (*readp & 0x3f) + 3;
            unsigned short pos = SDL_SwapLE16(*((unsigned short *) (readp + 1)));
            readp += 3;
            megacountc += count;
            memcpy_overlap(writep, image_out + pos, count);
            writep += count;
            c++;
        } else {
            THROW(std::invalid_argument, "Decode: File contains unknown format80 command");

        }
    }
    if (megacounta + megacountb + megacountc + megacountd + megacounte != checksum)
        return -1;

    return 0;
}
