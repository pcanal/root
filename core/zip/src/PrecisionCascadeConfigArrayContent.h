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
   size_t fLen = 0;                ///< Number of expressed compression level.
   UChar_t  fLevels = 0;           ///< Actually a "Int_t fLevels[fLen]"
   /// If the last level is 0 then the user is requesting to keep the last cascade element (residuals)


   size_t SizeOf()
   {
      return SizeOf(fLen);
   }

   static size_t SizeOf(size_t len)
   {
   return sizeof(PrecisionCascadeConfigArrayContent) + sizeof(UChar_t) * (len - 1);
   }

   ///< The size of the returned array in fLen.
   UChar_t *GetLevels()
   {
      return &fLevels;
   }

   UChar_t GetLevel(size_t i) {
      return (&fLevels)[i];
   }
};

}} // ROOT::Internal
