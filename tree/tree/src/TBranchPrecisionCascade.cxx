// @(#)root/tree:$Id$
// Author: Philippe Canal 06/01/22

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/** \class ROOT::Detail::TBranchPrecisionCascade
\ingroup tree

An Auxiliary objects holding the TBaskets information for the 
supplemental parts of the precision cascades for a specific branch.

**/

#include "TBranchPrecisionCascade.h"
#include "TBranch.h"

namespace ROOT {
namespace Detail {

////////////////////////////////////////////////////////////////////////////////
/// Usual Constructor

TBranchPrecisionCascade::TBranchPrecisionCascade(TBranch &br) :
   TNamed(br.GetFullName().Data(), "Precision Cascade"), fMaxBaskets(br.GetMaxBaskets())
{
   if (fMaxBaskets) {
      fBasketBytes = new Long64_t[fMaxBaskets];
      fBasketSeek  = new Long64_t[fMaxBaskets];
   }

   for (Int_t i = 0; i < fMaxBaskets; ++i) {
      fBasketBytes[i] = 0;
      fBasketSeek[i] = 0;
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Usual Destructor

TBranchPrecisionCascade::~TBranchPrecisionCascade()
{
   delete [] fBasketBytes;
   delete [] fBasketSeek;
}

////////////////////////////////////////////////////////////////////////////////
/// Store a precision cascade buffer and record its location.

Int_t TBranchPrecisionCascade::StoreCascade(Int_t basketnumber, Long64_t nbytes, char *buffer)
{
   return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Retrieve a precision cascade buffer

char *TBranchPrecisionCascade::RetrieveCascade(Int_t basketnumber)
{
   return nullptr;
}

} // Details
} // ROOT