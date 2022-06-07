// @(#)root/tree:$Id$
// Author: Philippe Canal 06/01/22

/*************************************************************************
 * Copyright (C) 1995-2022, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TBranchPrecisionCascade
#define ROOT_TBranchPrecisionCascade

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TBranchPrecisionCascade                                              //
//                                                                      //
// Auxiliary objects holding the TBaskets information for the           //
// supplemental parts of the precision cascades                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include "THashList.h"
#include "TNamed.h"

class TBranch;

namespace ROOT {
namespace Detail {

class TBranchPrecisionCascade : public TNamed
{
   Int_t       fMaxBaskets = 0;             ///< Maximum number of Baskets so far
   Long64_t   *fBasketBytes = nullptr;      ///<[fMaxBaskets] Length of baskets on file
   Long64_t   *fBasketSeek = nullptr;       ///<[fMaxBaskets] Addresses of baskets on file

public:
   TBranchPrecisionCascade() = default;
   TBranchPrecisionCascade(TBranch &br);
   ~TBranchPrecisionCascade();

   Int_t StoreCascade(Int_t basketnumber, Long64_t nbytes, char *buffer);

   ClassDef(TBranchPrecisionCascade, 3);
};

} // Details
} // ROOT

#endif // ROOT_TBranchPrecisionCascade