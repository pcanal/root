// @(#)root/zip:$Id$
// Author: Philippe Canal June 2022

/*************************************************************************
 * Copyright (C) 1995-2022, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#pragma once

#include "RtypesCore.h"

namespace ROOT {
namespace Internal {

struct PrecisionCascadeConfigArrayContent
{
   Bool_t fStoreResidual = kFALSE; ///< Whether the user requested keeping the last cascade element
   size_t fLen = 0;                ///< Number of expressed compression level.
   Int_t  fLevels = 0;             ///< Actually a "Int_t fLevels[fLen]"

   size_t SizeOf()
   {
      return SizeOf(fLen);
   }

   static size_t SizeOf(size_t len)
   {
   return sizeof(PrecisionCascadeConfigArrayContent) + sizeof(Int_t) * (len - 1);
   }

   ///< The size of the returned array in fLen.
   Int_t *GetLevels()
   {
      return &fLevels;
   }

   Int_t GetLevel(size_t i) {
      return (&fLevels)[i];
   }
};

}} // ROOT::Internal
