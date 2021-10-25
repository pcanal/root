// @(#)root/accelogic:$Id$
// Author: Philippe Canal, October 2021

/*************************************************************************
 * Copyright (C) 1995-2021, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "ZipAccelogic.h"
#include "AccelogicBLAST/AccelogicBLAST.hh"
#include <cstdio>
#include <cstdint>
#include <cstring>

static const int kHeaderSize = 9;

void R__zipBLAST(int cxlevel, int *srcsize, char *src, int *tgtsize, char *tgt, int *irep)
{
   *irep = 0;

   if (*tgtsize <= 0) {
      return;
   }

   if (*srcsize > 0xffffff || *srcsize < 0) {
      return;
   }

   size_t out_size;

   if (cxlevel <= 71 && (*srcsize % 4 == 0)) {
      size_t float_size = *srcsize / 4;
      // Use "absSense".  We shift the request config from [1,71] to [-60, 10]
      auto absSensLevel = cxlevel - 61;
      // Note: We need to check the source really start of a float boundary.
      // Note: We need to upgrade blast to avoid the memcpy (which is IN ADDITION to an internal copy already!!!)
      char *staging = nullptr;
      out_size = blast1_compress<true>(absSensLevel, (float*)src, float_size, staging);
      if ( (out_size + kHeaderSize) > (size_t)*tgtsize ) {
         delete [] staging;
         return;
      }
      memcpy(tgt + kHeaderSize, staging, out_size);
      delete [] staging;
      *irep = out_size + kHeaderSize;
   } else {
      // Call the other engines
      return;
   }

   tgt[0] = 'B';  /* Signature of Accelogic BLAST */
   tgt[1] = 'L';
   tgt[2] = cxlevel;

   unsigned in_size   = (unsigned) (*srcsize);

   tgt[3] = (char)(out_size & 0xff);
   tgt[4] = (char)((out_size >> 8) & 0xff);
   tgt[5] = (char)((out_size >> 16) & 0xff);

   tgt[6] = (char)(in_size & 0xff);         /* decompressed size */
   tgt[7] = (char)((in_size >> 8) & 0xff);
   tgt[8] = (char)((in_size >> 16) & 0xff);

   *irep = out_size + kHeaderSize;
}

void R__unzipBLAST(int *srcsize, unsigned char *src, int *tgtsize, unsigned char *tgt, int *irep)
{
   *irep = 0;

   auto cxlevel = src[2];

   if (cxlevel <= 71) {
      // Use "absSense".  We shift the request config from [1,71] to [-60, 10]
      auto absSensLevel = cxlevel - 61;
      // Note: We need to check the destination really start of a float boundary.
      float *staging = nullptr;
      size_t float_size = blast1_decompress<true>(absSensLevel, (char*)(&src[kHeaderSize]), *srcsize, staging);
      size_t out_size = float_size * 4;
      // Note: We need to upgrade blast to avoid the memcpy
      if ( out_size > (size_t)*tgtsize ) {
         delete [] staging;
         return;
      }
      memcpy(tgt, staging, out_size);
      delete [] staging;
      *irep = out_size;

   } else {
      // Need to handle the other engine
      return;
   }
}
