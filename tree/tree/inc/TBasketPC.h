// @(#)root/tree:$Id$
// Author: Philippe Canal 06/01/22

/*************************************************************************
 * Copyright (C) 1995-2022, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TBasketPC
#define ROOT_TBasketPC

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TBasketPC [TBasketPrecisionCascasde]                                 //
//                                                                      //
// TBasket equivalent for the precision cascade elements                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include "TKey.h"

class TDirectory;

namespace ROOT {
namespace Detail {
   class TBranchPrecisionCascade;
}
}

// The name of the class is uncompressed stored as part of each key header of each
// record, so let's keep it as short as possible.
class TBasketPC : public TKey
{
   ROOT::Detail::TBranchPrecisionCascade *fBranchPrecisionCascade = nullptr;
   
public:
   TBasketPC(TDirectory *dir, ROOT::Detail::TBranchPrecisionCascade &owner);

   Int_t ReadCascade(Long64_t pos, Long64_t len);

   Int_t WriteCascade(Int_t n, Char_t *cascade, Int_t uncompressedSize, Int_t basketnumber);

   ClassDefOverride(TBasketPC, 3);
};

#endif // ROOT_TBasketPC