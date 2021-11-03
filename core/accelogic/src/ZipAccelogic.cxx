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

static const int kHeaderSize = 10; // Regular ROOT header (9) plus one more for Blast.

union RealTypes {
   float *f;
   double *d;
   char *c;
};

union IntegerTypes {
   char *c;
   short *s;
   int *i;
   long long *l;
   unsigned char *uc;
   unsigned short *us;
   unsigned int *ui;
   unsigned long long *ul;
};


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

   int rawtype = datatype % EDataType::kOffsetL;

   int isfloat = (rawtype == EDataType::kFloat_t);
   bool isdouble = (rawtype == EDataType::kDouble_t);

   if (isfloat || isdouble) {
      // cxlevel <= 71
      const size_t elsize = isfloat ? sizeof(float) : sizeof(double);

      if ( (*srcsize % elsize) != 0 )
         return;
      size_t float_number = *srcsize / elsize;
      // Use "absSense".  We shift the request config from [1,71] to [-60, 10]
      auto absSensLevel = cxlevel - 61;
      // Note: We need to check the source really start of a float boundary.
      // Note: We need to upgrade blast to avoid the memcpy (which is IN ADDITION to an internal copy already!!!)
      char *staging = nullptr;
      RealTypes source;
      source.c = src;

      if (isfloat)
         out_size = blast1_compress<true>(absSensLevel, source.f, float_number, staging);
      else
         out_size = blast1_compress<true>(absSensLevel, source.d, float_number, staging);

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
      IntegerTypes source;
      source.c = src;

      if (cxlevel == 72) {
         out_size = blast2_compress<true>(source.c, *srcsize, staging);
      } else if (cxlevel == 73 && (*srcsize % sizeof(short) == 0)) {
         out_size = blast2_compress<true>(source.s, *srcsize, staging);
      } else if (cxlevel == 74 && (*srcsize % sizeof(int) == 0)) {
         out_size = blast2_compress<true>(source.i, *srcsize, staging);
      } else if (cxlevel == 75 && (*srcsize % sizeof(long long) == 0)) {
         out_size = blast2_compress<true>(source.l, *srcsize, staging);
      } else if (cxlevel == 76) {
         out_size = blast2_compress<true>(source.uc, *srcsize, staging);
      } else if (cxlevel == 77 && (*srcsize % sizeof(unsigned short) == 0)) {
         out_size = blast2_compress<true>(source.us, *srcsize, staging);
      } else if (cxlevel == 78 && (*srcsize % sizeof(unsigned int) == 0)) {
         out_size = blast2_compress<true>(source.ui, *srcsize, staging);
      } else if (cxlevel == 79 && (*srcsize % sizeof(unsigned long long) == 0)) {
         out_size = blast2_compress<true>(source.ul, *srcsize, staging);
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
   tgt[2] = datatype;

   unsigned in_size   = (unsigned) (*srcsize);

   tgt[3] = (char)(out_size & 0xff);
   tgt[4] = (char)((out_size >> 8) & 0xff);
   tgt[5] = (char)((out_size >> 16) & 0xff);

   tgt[6] = (char)(in_size & 0xff);         /* decompressed size */
   tgt[7] = (char)((in_size >> 8) & 0xff);
   tgt[8] = (char)((in_size >> 16) & 0xff);

   // Blast specific
   tgt[9] = datatype;

   *irep = out_size + kHeaderSize;
}

void R__unzipBLAST(int *srcsize, unsigned char *src, int *tgtsize, unsigned char *tgt, int *irep)
{
   *irep = 0;

   char* source = (char*)(&src[kHeaderSize]);
   size_t in_size = (*srcsize) - kHeaderSize;
   auto cxlevel = src[2];
   auto datatype = src[9];

   size_t out_size;

   int rawtype = datatype % EDataType::kOffsetL;

   int isfloat = (rawtype == EDataType::kFloat_t);
   bool isdouble = (rawtype == EDataType::kDouble_t);

   if (isfloat || isdouble) {
      // Use "absSense".  We shift the request config from [1,71] to [-60, 10]
      auto absSensLevel = cxlevel - 61;
      // Note: We need to check the destination really start of a float boundary.
      RealTypes staging;
      staging.c = nullptr;

      size_t float_size = isfloat ? blast1_decompress<true>(absSensLevel, source, in_size, staging.f)
                                  : blast1_decompress<true>(absSensLevel, source, in_size, staging.d);

      const size_t elsize = isfloat ? sizeof(float) : sizeof(double);
      out_size = float_size * elsize;
      // Note: We need to upgrade blast to avoid the memcpy
      if ( out_size > (size_t)*tgtsize ) {
         delete [] staging.c;
         return;
      }
      memcpy(tgt, staging.c, out_size);
      delete [] staging.c;
      *irep = out_size;
   } else if (cxlevel <= 79) {
      // Use "RLE.  cx level determines data type
      // Note: We need to check the destination really start of a short boundary.
      IntegerTypes staging;
      staging.c = nullptr;
      switch (cxlevel) {
         case (79) : out_size = blast2_decompress<true>(source, in_size, staging.ul); break;
         case (78) : out_size = blast2_decompress<true>(source, in_size, staging.ui); break;
         case (77) : out_size = blast2_decompress<true>(source, in_size, staging.us); break;
         case (76) : out_size = blast2_decompress<true>(source, in_size, staging.uc); break;
         case (75) : out_size = blast2_decompress<true>(source, in_size, staging.l); break;
         case (74) : out_size = blast2_decompress<true>(source, in_size, staging.i); break;
         case (73) : out_size = blast2_decompress<true>(source, in_size, staging.s); break;
         default   : out_size = blast2_decompress<true>(source, in_size, staging.c);
      }
      // Note: We need to upgrade blast to avoid the memcpy
      if ( out_size > (size_t)*tgtsize ) {
         delete [] staging.c;
         return;
      }
      memcpy(tgt, staging.c, out_size);
      delete [] staging.c;
      *irep = out_size;
   } else {
      // Need to handle the other engine
      return;
   }
}
