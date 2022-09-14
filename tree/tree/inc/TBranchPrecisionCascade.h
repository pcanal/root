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

class TBasketPC;
class TBranch;
class TTree;

namespace ROOT {
namespace Detail {

class TBranchPrecisionCascade : public TNamed
{
   UInt_t      fCascadeLevel = 0;       ///< Position in the precision cascade.
   Int_t       fMaxBaskets   = 0;       ///< Maximum number of Baskets so far
   Long64_t   *fBasketBytes  = nullptr; ///<[fMaxBaskets] Length of baskets on file
   Long64_t   *fBasketSeek   = nullptr; ///<[fMaxBaskets] Addresses of baskets on file
   Long64_t    fTotBytes = 0;           ///< Estimate Total number of bytes in all leaves before compression
   Long64_t    fZipBytes     = 0;       ///< Total number of bytes in all leaves after compressionß

   TBasketPC  *fBasket = nullptr;   ///<! Current basket to read or write the Precision Cascade element.

public:
   TBranchPrecisionCascade() = default;
   TBranchPrecisionCascade(UInt_t cascadelevel, TBranch &br);
   ~TBranchPrecisionCascade();

   TBasketPC *GetBasketPC(TTree &tree, UInt_t /* basketnumber */);

   UInt_t GetCascadeLevel() const { return fCascadeLevel; }

   // Return the estimated total uncompressed size of the cascade buffers
   Long64_t  GetTotBytes() const { return fTotBytes; };

   // Return the total compressed size of the cascade buffers
   Long64_t  GetZipBytes() const { return fZipBytes; }

   Int_t StoreCascade(TTree &tree, Int_t basketnumber, Long64_t nbytes, Int_t uncompressedSize);

   char *RetrieveCascade(TTree &tree, Int_t basketnumber);

   void Print(Option_t *option="") const;

   ClassDef(TBranchPrecisionCascade, 4);
};

} // Details
} // ROOT

#endif // ROOT_TBranchPrecisionCascade