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

#ifdef __cplusplus
extern "C" {
#endif

void R__zipBLAST(int cxlevel, int *srcsize, char *src, int *tgtsize, char *tgt, int *irep);

void R__unzipBLAST(int *srcsize, unsigned char *src, int *tgtsize, unsigned char *tgt, int *irep);

#ifdef __cplusplus
}
#endif

#endif // ROOT_ZipAccelogic
