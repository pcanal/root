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
#include "TError.h"

static const int kHeaderSize = 9;

void R__zipBLAST(int cxlevel, int *srcsize, char *src, int *tgtsize, char *tgt, int *irep, EDataType datatype)
{
   *irep = 0;

   if (*tgtsize <= 0) {
      return;
   }

   if (*srcsize > 0xffffff || *srcsize < 0) {
      return;
   }

   if (datatype >= (EDataType::kOffsetP + EDataType::kVoid_t)) {
      Error("R__zipBLAST", "Accelogic BLAST connector requires a buffer with homegeonous numericla data types (datatype is %d)\n", datatype);
      return;
   }
   if (datatype >= EDataType::kOffsetP) {
      Error("R__zipBLAST", "Accelogic BLAST connector does not yet support variable size array (datatype is %d)\n", datatype);
      return;
   }

   size_t out_size;

   if (cxlevel <= 71 && (*srcsize % sizeof(float) == 0)) {
      size_t float_size = *srcsize / sizeof(float);
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
   } else if (cxlevel <= 79) {
      // Use "RLE.  cx level determines data type
      // Note: We need to check the source really start of a boundary.
      // Note: We need to upgrade blast to avoid the memcpy (which is IN ADDITION to an internal copy already!!!)
      char *staging = nullptr;
      // out_size = blast2_compress<T,true>((T*)src, *srcsize, staging);
      if (cxlevel == 72) {
         out_size = blast2_compress<char,true>(src, *srcsize, staging);
      } else if (cxlevel == 73 && (*srcsize % sizeof(short) == 0)) {
         out_size = blast2_compress<short,true>((short*)src, *srcsize, staging);
      } else if (cxlevel == 74 && (*srcsize % sizeof(int) == 0)) {
         out_size = blast2_compress<int,true>((int*)src, *srcsize, staging);
      } else if (cxlevel == 75 && (*srcsize % sizeof(long long) == 0)) {
         out_size = blast2_compress<long long,true>((long long*)src, *srcsize, staging);
      } else if (cxlevel == 76) {
         out_size = blast2_compress<unsigned char,true>((unsigned char*) src, *srcsize, staging);
      } else if (cxlevel == 77 && (*srcsize % sizeof(unsigned short) == 0)) {
         out_size = blast2_compress<unsigned short,true>((unsigned short*)src, *srcsize, staging);
      } else if (cxlevel == 78 && (*srcsize % sizeof(unsigned int) == 0)) {
         out_size = blast2_compress<unsigned int,true>((unsigned int*)src, *srcsize, staging);
      } else if (cxlevel == 79 && (*srcsize % sizeof(unsigned long long) == 0)) {
         out_size = blast2_compress<unsigned long long,true>((unsigned long long*)src, *srcsize, staging);
      } else {
         // not proper length
         return;
      }
      if ( (out_size + kHeaderSize) > (size_t)*tgtsize ) {
         delete [] staging;
         return;
      }
      memcpy(tgt + kHeaderSize, staging, out_size);
      delete [] staging;
      *irep = out_size + kHeaderSize;
   } else {
      // Here we should call the other engines
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

   size_t out_size;

   if (cxlevel <= 71) {
      // Use "absSense".  We shift the request config from [1,71] to [-60, 10]
      auto absSensLevel = cxlevel - 61;
      // Note: We need to check the destination really start of a float boundary.
      float *staging = nullptr;
      size_t float_size = blast1_decompress<true>(absSensLevel, (char*)(&src[kHeaderSize]), *srcsize, staging);
      out_size = float_size * sizeof(float);
      // Note: We need to upgrade blast to avoid the memcpy
      if ( out_size > (size_t)*tgtsize ) {
         delete [] staging;
         return;
      }
      memcpy(tgt, staging, out_size);
      delete [] staging;
      *irep = out_size;
   } else if (cxlevel <= 79) {
      // Use "RLE.  cx level determines data type
      // Note: We need to check the destination really start of a short boundary.
      char *staging = nullptr;
      char*& stagingPtr = staging;
      switch (cxlevel) {
         case (79) : out_size = blast2_decompress<unsigned long long,true>((char*)(&src[kHeaderSize]), *srcsize, (unsigned long long*&) stagingPtr); break;
         case (78) : out_size = blast2_decompress<unsigned int,true>((char*)(&src[kHeaderSize]), *srcsize, (unsigned int*&) stagingPtr); break;
         case (77) : out_size = blast2_decompress<unsigned short,true>((char*)(&src[kHeaderSize]), *srcsize, (unsigned short*&) stagingPtr); break;
         case (76) : out_size = blast2_decompress<unsigned char,true>((char*)(&src[kHeaderSize]), *srcsize, (unsigned char*&) stagingPtr);
         case (75) : out_size = blast2_decompress<long long,true>((char*)(&src[kHeaderSize]), *srcsize, (long long*&) stagingPtr); break;
         case (74) : out_size = blast2_decompress<int,true>((char*)(&src[kHeaderSize]), *srcsize, (int*&) stagingPtr); break;
         case (73) : out_size = blast2_decompress<short,true>((char*)(&src[kHeaderSize]), *srcsize, (short*&) stagingPtr); break;
         default   : out_size = blast2_decompress<char,true>((char*)(&src[kHeaderSize]), *srcsize, staging);
      }
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
