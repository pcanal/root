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
   Short_t *s;
   Int_t *i;
   Long_t *l;
   Long64_t *ll;
   unsigned char *uc;
   UShort_t *us;
   UInt_t *ui;
   ULong_t *ul;
   ULong64_t *ull;
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

      switch(datatype) {
         case EDataType::kChar_t:
           out_size = blast2_compress<true>(source.c, *srcsize, staging);
           break;
         case EDataType::kUChar_t:
           out_size = blast2_compress<true>(source.uc, *srcsize, staging);
           break;
         case EDataType::kShort_t:
           out_size = blast2_compress<true>(source.s, *srcsize, staging);
           break;
         case EDataType::kUShort_t:
           out_size = blast2_compress<true>(source.us, *srcsize, staging);
           break;
         case EDataType::kInt_t:
           out_size = blast2_compress<true>(source.i, *srcsize, staging);
           break;
         case EDataType::kUInt_t:
           out_size = blast2_compress<true>(source.ui, *srcsize, staging);
           break;
         case EDataType::kLong_t:
           out_size = blast2_compress<true>(source.l, *srcsize, staging);
           break;
         case EDataType::kULong_t:
           out_size = blast2_compress<true>(source.ul, *srcsize, staging);
           break;
         case EDataType::kLong64_t:
           out_size = blast2_compress<true>(source.ll, *srcsize, staging);
           break;
         case EDataType::kULong64_t:
           out_size = blast2_compress<true>(source.ull, *srcsize, staging);
           break;
         default:
           // data type not supported.
           // We actually need to use gzip, for now just bail
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
     switch(datatype) {
         case EDataType::kChar_t:
           out_size = blast2_decompress<true>(source, in_size, staging.c);
           break;
         case EDataType::kUChar_t:
           out_size = blast2_decompress<true>(source, in_size, staging.uc);
           break;
         case EDataType::kShort_t:
           out_size = blast2_decompress<true>(source, in_size, staging.s);
           break;
         case EDataType::kUShort_t:
           out_size = blast2_decompress<true>(source, in_size, staging.us);
           break;
         case EDataType::kInt_t:
           out_size = blast2_decompress<true>(source, in_size, staging.i);
           break;
         case EDataType::kUInt_t:
           out_size = blast2_decompress<true>(source, in_size, staging.ui);
           break;
         case EDataType::kLong_t:
           out_size = blast2_decompress<true>(source, in_size, staging.l);
           break;
         case EDataType::kULong_t:
           out_size = blast2_decompress<true>(source, in_size, staging.ul);
           break;
         case EDataType::kLong64_t:
           out_size = blast2_decompress<true>(source, in_size, staging.ll);
           break;
         case EDataType::kULong64_t:
           out_size = blast2_decompress<true>(source, in_size, staging.ull);
           break;
         default:
           // data type not supported.
           // We actually need to use gzip, for now just bail
           out_size = blast2_decompress<true>(source, in_size, staging.c);
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
