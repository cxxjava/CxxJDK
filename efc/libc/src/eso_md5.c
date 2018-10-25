/**
 * @file  eso_md5.c
 * @brief ES RSA Data Security
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include "eso_md5.h"
#include "eso_libc.h"

#ifdef __STDC__
		 #define UL(x)		 x##U
#else
		 #define UL(x)		 x
#endif

// F, G, H and I are basic MD5 functions 
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
// ROTATE_LEFT rotates x left n bits 
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 
// Rotation is separate from addition to prevent recomputation 

#define FF(a, b, c, d, x, s, ac){(a) += F ((b), (c), (d)) + (x) + (es_uint32_t)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}
#define GG(a, b, c, d, x, s, ac){(a) += G ((b), (c), (d)) + (x) + (es_uint32_t)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}
#define HH(a, b, c, d, x, s, ac){(a) += H ((b), (c), (d)) + (x) + (es_uint32_t)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}
#define II(a, b, c, d, x, s, ac){(a) += I ((b), (c), (d)) + (x) + (es_uint32_t)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21



// forward declaration 
  
static void Transform(es_uint32_t *buf,es_uint32_t * in);

static const es_uint8_t PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};




// The routine MD5Init initializes the message-digest context
//   mdContext. All fields are set to zero.
 
void eso_md5_init(es_md5_ctx_t* mdContext)
{
  mdContext->i[0] = mdContext->i[1] = (es_uint32_t)0;

  // Load magic initialization constants.
   
  mdContext->buf[0] = (es_uint32_t)0x67452301;
  mdContext->buf[1] = (es_uint32_t)0xefcdab89;
  mdContext->buf[2] = (es_uint32_t)0x98badcfe;
  mdContext->buf[3] = (es_uint32_t)0x10325476;
}

/* The routine MD5Update updates the message-digest context to
   account for the presence of each of the characters inBuf[0..inLen-1]
   in the message whose digest is being computed.
 */
                    /* context      input block         length of input block*/
void eso_md5_update(es_md5_ctx_t *mdContext, const es_uint8_t *inBuf, es_uint32_t inLen)
{
  es_uint32_t in[16];
  es_int32_t mdi;
  es_uint32_t i, ii;

  // compute number of bytes mod 64 
  mdi = (es_int32_t)((mdContext->i[0] >> 3) & 0x3F);

  // update number of bits 
  if ((mdContext->i[0] + ((es_uint32_t)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((es_uint32_t)inLen << 3);
  mdContext->i[1] += ((es_uint32_t)inLen >> 29);

  while (inLen--) {
    // add new character to buffer, increment mdi 
    mdContext->in[mdi++] = *inBuf++;

    // transform if necessary 
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((es_uint32_t)mdContext->in[ii+3]) << 24) |
                (((es_uint32_t)mdContext->in[ii+2]) << 16) |
                (((es_uint32_t)mdContext->in[ii+1]) << 8) |
                ((es_uint32_t)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

// The routine MD5Final terminates the message-digest computation and
//   ends with the desired message digest in mdContext->digest[0...15].
void eso_md5_final(es_uint8_t hash[], es_md5_ctx_t *mdContext)
{
  es_uint32_t in[16];
  es_int32_t mdi;
  es_uint32_t i, ii;
  es_uint32_t padLen;

  // save number of bits 
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  // compute number of bytes mod 64 
  mdi = (es_int32_t)((mdContext->i[0] >> 3) & 0x3F);

  // pad out to 56 mod 64 
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  eso_md5_update (mdContext, PADDING, padLen);

  // append length in bits and transform 
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((es_uint32_t)mdContext->in[ii+3]) << 24) |
            (((es_uint32_t)mdContext->in[ii+2]) << 16) |
            (((es_uint32_t)mdContext->in[ii+1]) << 8) |
            ((es_uint32_t)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  // store buffer in digest 
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (es_uint8_t)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (es_uint8_t)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (es_uint8_t)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (es_uint8_t)((mdContext->buf[i] >> 24) & 0xFF);
  }
  eso_memcpy(hash, mdContext->digest, 16);
}

// Basic MD5 step. Transforms buf based on in.
 

 
static void Transform (es_uint32_t *buf,es_uint32_t * in)
{
  es_uint32_t a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  // Round 1 
  FF ( a, b, c, d, in[ 0], S11, UL(3614090360)); 
  FF ( d, a, b, c, in[ 1], S12, UL(3905402710)); 
  FF ( c, d, a, b, in[ 2], S13, UL( 606105819)); 
  FF ( b, c, d, a, in[ 3], S14, UL(3250441966)); 
  FF ( a, b, c, d, in[ 4], S11, UL(4118548399)); 
  FF ( d, a, b, c, in[ 5], S12, UL(1200080426)); 
  FF ( c, d, a, b, in[ 6], S13, UL(2821735955)); 
  FF ( b, c, d, a, in[ 7], S14, UL(4249261313)); 
  FF ( a, b, c, d, in[ 8], S11, UL(1770035416)); 
  FF ( d, a, b, c, in[ 9], S12, UL(2336552879)); 
  FF ( c, d, a, b, in[10], S13, UL(4294925233));
  FF ( b, c, d, a, in[11], S14, UL(2304563134)); 
  FF ( a, b, c, d, in[12], S11, UL(1804603682)); 
  FF ( d, a, b, c, in[13], S12, UL(4254626195)); 
  FF ( c, d, a, b, in[14], S13, UL(2792965006)); 
  FF ( b, c, d, a, in[15], S14, UL(1236535329)); 

  // Round 2 

  GG ( a, b, c, d, in[ 1], S21, UL(4129170786)); 
  GG ( d, a, b, c, in[ 6], S22, UL(3225465664)); 
  GG ( c, d, a, b, in[11], S23, UL( 643717713)); 
  GG ( b, c, d, a, in[ 0], S24, UL(3921069994)); 
  GG ( a, b, c, d, in[ 5], S21, UL(3593408605)); 
  GG ( d, a, b, c, in[10], S22, UL(  38016083)); 
  GG ( c, d, a, b, in[15], S23, UL(3634488961)); 
  GG ( b, c, d, a, in[ 4], S24, UL(3889429448)); 
  GG ( a, b, c, d, in[ 9], S21, UL( 568446438)); 
  GG ( d, a, b, c, in[14], S22, UL(3275163606)); 
  GG ( c, d, a, b, in[ 3], S23, UL(4107603335)); 
  GG ( b, c, d, a, in[ 8], S24, UL(1163531501)); 
  GG ( a, b, c, d, in[13], S21, UL(2850285829)); 
  GG ( d, a, b, c, in[ 2], S22, UL(4243563512)); 
  GG ( c, d, a, b, in[ 7], S23, UL(1735328473)); 
  GG ( b, c, d, a, in[12], S24, UL(2368359562)); 

  // Round 3 

  HH ( a, b, c, d, in[ 5], S31, UL(4294588738));
  HH ( d, a, b, c, in[ 8], S32, UL(2272392833)); 
  HH ( c, d, a, b, in[11], S33, UL(1839030562)); 
  HH ( b, c, d, a, in[14], S34, UL(4259657740)); 
  HH ( a, b, c, d, in[ 1], S31, UL(2763975236));
  HH ( d, a, b, c, in[ 4], S32, UL(1272893353));
  HH ( c, d, a, b, in[ 7], S33, UL(4139469664)); 
  HH ( b, c, d, a, in[10], S34, UL(3200236656)); 
  HH ( a, b, c, d, in[13], S31, UL( 681279174)); 
  HH ( d, a, b, c, in[ 0], S32, UL(3936430074)); 
  HH ( c, d, a, b, in[ 3], S33, UL(3572445317)); 
  HH ( b, c, d, a, in[ 6], S34, UL(  76029189)); 
  HH ( a, b, c, d, in[ 9], S31, UL(3654602809)); 
  HH ( d, a, b, c, in[12], S32, UL(3873151461)); 
  HH ( c, d, a, b, in[15], S33, UL( 530742520)); 
  HH ( b, c, d, a, in[ 2], S34, UL(3299628645)); 

  // Round 4 

  II ( a, b, c, d, in[ 0], S41, UL(4096336452)); 
  II ( d, a, b, c, in[ 7], S42, UL(1126891415)); 
  II ( c, d, a, b, in[14], S43, UL(2878612391)); 
  II ( b, c, d, a, in[ 5], S44, UL(4237533241));
  II ( a, b, c, d, in[12], S41, UL(1700485571)); 
  II ( d, a, b, c, in[ 3], S42, UL(2399980690));
  II ( c, d, a, b, in[10], S43, UL(4293915773)); 
  II ( b, c, d, a, in[ 1], S44, UL(2240044497)); 
  II ( a, b, c, d, in[ 8], S41, UL(1873313359)); 
  II ( d, a, b, c, in[15], S42, UL(4264355552)); 
  II ( c, d, a, b, in[ 6], S43, UL(2734768916)); 
  II ( b, c, d, a, in[13], S44, UL(1309151649)); 
  II ( a, b, c, d, in[ 4], S41, UL(4149444226)); 
  II ( d, a, b, c, in[11], S42, UL(3174756917)); 
  II ( c, d, a, b, in[ 2], S43, UL( 718787259)); 
  II ( b, c, d, a, in[ 9], S44, UL(3951481745)); 

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}
