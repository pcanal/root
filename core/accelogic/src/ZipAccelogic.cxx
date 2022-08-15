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

static const int kHeaderSize = 11; // Regular ROOT header (9) plus two more for Blast.
static const int MAX_ZIG_BUFFERS = 64; // Maximum number of buffers for BLAST's ZIG method

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

void R__zipBLAST(unsigned char cxlevel, int *srcsize, char *src, int *tgtsize, char *tgt, int *irep, EDataType datatype)
{
   R__zipBLAST(&cxlevel,srcsize,src,tgtsize,&tgt,1,irep,datatype);
}

void R__zipBLAST(unsigned char *cxlevels, int *srcsize, char *src, int *tgtsize, char **tgts, int tgt_number, int *irep, EDataType datatype)
{
   memset(irep,0,tgt_number*sizeof(int)); // irep needs to point to an array of integers of size tgt_number (could just be a single integer)
   char *tgt = *tgts;

   for (int tgt_idx=0; tgt_idx<tgt_number; tgt_idx++) {
      if (tgtsize[tgt_idx] <= 0) {
         return;
      }
   }

   if (*srcsize > 0xffffff || *srcsize < 0) {
      return;
   }


   if (datatype == EDataType::kNoType_t) {
      Error("R__zipBLAST", "Accelogic BLAST connector requires a datatype (none was specified)\n");
      return;
   }
   if (datatype >= (EDataType::kOffsetP + EDataType::kVoid_t)) {
      Error("R__zipBLAST", "Accelogic BLAST connector requires a buffer with homegeonous numerical data types (datatype is %d)\n", datatype);
      return;
   }
   if (datatype >= EDataType::kOffsetP) {
      Error("R__zipBLAST", "Accelogic BLAST connector does not yet support variable size array (datatype is %d)\n", datatype);
      return;
   }

   int rawtype = datatype % EDataType::kOffsetL;

   int isfloat = (rawtype == EDataType::kFloat_t);
   bool isdouble = (rawtype == EDataType::kDouble_t);

   size_t out_sizes[MAX_ZIG_BUFFERS];

   if (isfloat || isdouble) {
      const size_t elsize = isfloat ? sizeof(float) : sizeof(double);

      if ( (*srcsize % elsize) != 0 )
         return;
      size_t float_number = *srcsize / elsize;
      // Use "absSens".
      int absSensLevels[MAX_ZIG_BUFFERS];
      // We shift the request config from [1,71] to [-60, 10]
      for (int tgt_idx=0; tgt_idx<tgt_number; tgt_idx++)
         absSensLevels[tgt_idx] = cxlevels[tgt_idx] - 61;
      // blast1_compress needs to know whether to keep the residual, and does not count
      // the residual among the target buffers. We use cxlevel=0 for final buffer to
      // indicate whether it will be the residual buffer.
      auto needresidual = (cxlevels[tgt_number-1] == 0);
      auto absSens_tgt_number = tgt_number - (needresidual ? 1 : 0);
      // Note: We need to check the source really start of a float boundary.
      // Note: We need to upgrade blast to avoid the memcpy (which is IN ADDITION to an internal copy already!!!)
      char *staging[MAX_ZIG_BUFFERS] = { nullptr };
      RealTypes source;
      source.c = src;

      if (isfloat)
         blast1_compress<true>(absSensLevels, source.f, float_number, staging, out_sizes, absSens_tgt_number, needresidual);
      else
         blast1_compress<true>(absSensLevels, source.d, float_number, staging, out_sizes, absSens_tgt_number, needresidual);

      auto excessive_size = false;
      for (int tgt_idx=0; tgt_idx<tgt_number && !excessive_size; tgt_idx++)
         excessive_size |= ( ( out_sizes[tgt_idx] + kHeaderSize) > (size_t)tgtsize[tgt_idx] );

      if (excessive_size) {
         for (int tgt_idx=0; tgt_idx<tgt_number; tgt_idx++)
           delete [] (staging[tgt_idx]);
         return;
      }

      for (int tgt_idx=0; tgt_idx<tgt_number; tgt_idx++) {
         memcpy(tgts[tgt_idx] + kHeaderSize, staging[tgt_idx], out_sizes[tgt_idx]);
         tgts[tgt_idx][2] = cxlevels[tgt_idx];
         delete [] (staging[tgt_idx]);
         // irep points to an array of all buffer sizes
         irep[tgt_idx] = out_sizes[tgt_idx] + kHeaderSize;
      }
   } else {
      // Use "RLE".
      // Note: We need to check the source really start of a boundary.
      // Note: We need to upgrade blast to avoid the memcpy (which is IN ADDITION to an internal copy already!!!)
      char *staging = nullptr;
      IntegerTypes source;
      source.c = src;

      size_t& out_size = out_sizes[0];

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
      tgt[2] = *cxlevels; // inconsequential with RLE, which has no levels
      delete [] staging;
      *irep = out_size + kHeaderSize;
   }

   unsigned in_size   = (unsigned) (*srcsize);
   for (int tgt_idx=0; tgt_idx<tgt_number; tgt_idx++) {
      tgt = tgts[tgt_idx];

      tgt[0] = 'B';  /* Signature of Accelogic BLAST */
      tgt[1] = 'L';
      // tgt[2] is set for each target buffer above

      // Include the 2 extra header byte into the out size.
      const size_t out_size = out_sizes[tgt_idx] + 2;
      tgt[3] = (char)((out_size) & 0xff);
      tgt[4] = (char)((out_size >> 8) & 0xff);
      tgt[5] = (char)((out_size >> 16) & 0xff);

      tgt[6] = (char)(in_size & 0xff);         /* decompressed size */
      tgt[7] = (char)((in_size >> 8) & 0xff);
      tgt[8] = (char)((in_size >> 16) & 0xff);

      // Blast specific
      tgt[9] = datatype;
      tgt[10] = tgt_number;
   }
}

void R__unzipBLAST(int *srcsize, unsigned char *src, int *tgtsize, unsigned char *tgt, int *irep)
{
  R__unzipBLAST(srcsize,&src,tgtsize,tgt,1,irep);
}

void R__unzipBLAST(int *srcsize, unsigned char **srcs, int *tgtsize, unsigned char *tgt, int src_number, int *irep)
{
   *irep = 0;
   unsigned char *src = *srcs;
   auto datatype = src[9];
   auto maximum_src_number = src[10];
   if (src_number > maximum_src_number) {
     Error("R__unzipBLAST", "too many sources (%d more than maximum %d\n",src_number,maximum_src_number);
     return;
   } else if (src_number < maximum_src_number && srcs[src_number-1][2] == 0) {
     // This is a simple check that full precision is only ever possible with all sources from compression.
     // However, it is possible that full precision was not saved at compression (last source cxlevel!=0),
     // so even using all saved sources may not return full precision.
     Error("R__unzipBLAST", "too few sources provided to obtain full precision (%d below minimum of %d sources)", src_number, maximum_src_number);
     return;
   }

   size_t out_size;

   int rawtype = datatype % EDataType::kOffsetL;

   int isfloat = (rawtype == EDataType::kFloat_t);
   bool isdouble = (rawtype == EDataType::kDouble_t);

   if (isfloat || isdouble) {
      // Use "absSense".  We shift the request config from [1,71] to [-60, 10]
      int absSensLevels[MAX_ZIG_BUFFERS];
      char* sources[MAX_ZIG_BUFFERS];
      size_t in_sizes[MAX_ZIG_BUFFERS];
      for (int src_idx=0; src_idx<src_number; src_idx++) {
         absSensLevels[src_idx] = srcs[src_idx][2] - 61;
         sources[src_idx] = (char*)(&srcs[src_idx][kHeaderSize]);
         // The 2 extra header byte are include in the source size.
         in_sizes[src_idx] = (size_t) (srcsize[src_idx] - kHeaderSize);
      }
      auto absSens_src_number = src_number - 1; // Needs to be 1 less than provided sources
      // Note: We need to check the destination really start of a float boundary.
      RealTypes staging;
      staging.c = nullptr;

      size_t float_size = isfloat ? blast1_decompress<true>(absSensLevels, sources, in_sizes, staging.f, absSens_src_number)
                                  : blast1_decompress<true>(absSensLevels, sources, in_sizes, staging.d, absSens_src_number);

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
   } else {
      char* source = (char*)(&src[kHeaderSize]);
      size_t in_size = (*srcsize) - kHeaderSize;

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
   }
}
