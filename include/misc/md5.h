/**
 * \file md5.h
 *
 *  Copyright (C) 2006-2010, Paul Bakker <polarssl_maintainer at polarssl.org>
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef POLARSSL_MD5_H
#define POLARSSL_MD5_H

#include <cstdint>

/**
 * \brief          MD5 context structure
 */
struct md5_context {
    uint32_t total[2];  /*!< number of bytes processed  */
    uint32_t state[4];  /*!< intermediate digest state  */
    uint8_t buffer[64]; /*!< data block being processed */

    uint8_t ipad[64]; /*!< HMAC: inner padding        */
    uint8_t opad[64]; /*!< HMAC: outer padding        */
};

/**
 * \brief          MD5 context setup
 *
 * \param ctx      context to be initialized
 */
void md5_starts(md5_context* ctx);

/**
 * \brief          MD5 process buffer
 *
 * \param ctx      MD5 context
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 */
void md5_update(md5_context* ctx, const uint8_t* input, size_t ilen);

/**
 * \brief          MD5 final digest
 *
 * \param ctx      MD5 context
 * \param output   MD5 checksum result
 */
void md5_finish(md5_context* ctx, uint8_t output[16]);

/**
 * \brief          Output = MD5( input buffer )
 *
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 * \param output   MD5 checksum result
 */
void md5(const uint8_t* input, size_t ilen, uint8_t output[16]);

/**
 * \brief          Output = MD5( file contents )
 *
 * \param path     input file name
 * \param output   MD5 checksum result
 *
 * \return         0 if successful, 1 if fopen failed,
 *                 or 2 if fread failed
 */
int md5_file(const char* path, uint8_t output[16]);

/**
 * \brief          MD5 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void md5_hmac_starts(md5_context* ctx, const uint8_t* key, size_t keylen);

/**
 * \brief          MD5 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md5_hmac_update(md5_context* ctx, const uint8_t* input, size_t ilen);

/**
 * \brief          MD5 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD5 HMAC checksum result
 */
void md5_hmac_finish(md5_context* ctx, uint8_t output[16]);

/**
 * \brief          MD5 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void md5_hmac_reset(md5_context* ctx);

/**
 * \brief          Output = HMAC-MD5( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD5 result
 */
void md5_hmac(const uint8_t* key, size_t keylen, const uint8_t* input, size_t ilen, uint8_t output[16]);

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int md5_self_test(int verbose);

#endif /* md5.h */
