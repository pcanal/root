// @(#)root/accelogic:$Id$
// Author: Philippe Canal, October 2021

/*************************************************************************
 * Copyright (C) 1995-2021, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_ZipAccelogic
#define ROOT_ZipAccelogic

#include "EDataType.h"

void R__zipBLAST(int *cxlevels, int *srcsize, char *src, int **tgtsizes, char **tgts, int tgt_number, int *irep, EDataType datatype = EDataType::kNoType_t);

void R__unzipBLAST(int **srcsizes, unsigned char **srcs, int *tgtsize, unsigned char *tgt, int src_number, int *irep);

// The below two interfaces are to maintain backward compatibility until RZip.cxx is updated to use the above two interfaces

#ifdef __cplusplus
extern "C" {
#endif

void R__zipBLAST(int cxlevel, int *srcsize, char *src, int *tgtsize, char *tgt, int *irep, EDataType datatype = EDataType::kNoType_t);

void R__unzipBLAST(int *srcsize, unsigned char *src, int *tgtsize, unsigned char *tgt, int *irep);

#ifdef __cplusplus
}
#endif

#endif // ROOT_ZipAccelogic
