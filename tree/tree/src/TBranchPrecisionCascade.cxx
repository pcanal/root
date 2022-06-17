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

#include "TBasketPC.h"
#include "TBranchPrecisionCascade.h"
#include "TBranch.h"
#include "TStorage.h"
#include "TTree.h"
#include "TFile.h"

namespace ROOT {
namespace Detail {

////////////////////////////////////////////////////////////////////////////////
/// Usual Constructor

TBranchPrecisionCascade::TBranchPrecisionCascade(UInt_t cascadelevel, TBranch &br) :
   TNamed(br.GetFullName().Data(), "Precision Cascade"), fCascadeLevel(cascadelevel),
   fMaxBaskets(br.GetMaxBaskets())
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
/// Return (and create if need be) the request TBasketPC
///
/// Note: Currently there is no caching of the basket, so we always reuse
/// a single instance.

TBasketPC *TBranchPrecisionCascade::GetBasketPC(TTree &tree, UInt_t /* basketnumber */)
{
   if (!fBasket)
      fBasket = tree.CreateBasketPC(*this);
   return fBasket;
}

////////////////////////////////////////////////////////////////////////////////
/// Store a precision cascade buffer and record its location.

Int_t TBranchPrecisionCascade::StoreCascade(TTree &tree, Int_t basketnumber, Long64_t nbytes, Int_t uncompressedSize)
{
   if (basketnumber > fMaxBaskets) {
      Int_t newsize = TMath::Max(10,Int_t(1.5*basketnumber));
      fBasketBytes  = (Long64_t*)TStorage::ReAlloc(fBasketBytes, newsize*sizeof(Long64_t), fMaxBaskets*sizeof(Long64_t));
      fBasketSeek   = (Long64_t*)TStorage::ReAlloc(fBasketSeek,
                                                   newsize*sizeof(Long64_t),fMaxBaskets*sizeof(Long64_t));
      fMaxBaskets   = newsize;
   }
   TBasketPC *basket = GetBasketPC(tree, basketnumber);
   if (!basket)
      return 0;

   Int_t nout = basket->WriteCascade(nbytes, uncompressedSize, basketnumber);

   if (nout < 0) {
      auto dir = basket->GetMotherDir();
      auto file = dir ? dir->GetFile() : (TFile*)nullptr;
      const char *name = file ? file->GetName() : "memory";
      Error("StoreCascade", "Precision cascade element was not properly stored in %s", name);
   }
   fBasketBytes[basketnumber] = basket->GetNbytes();
   fBasketSeek[basketnumber] = basket->GetSeekKey();
   // FIXME: Need to update some statistics in the TTree or the in TTreePrecisionCascade.

   return nout;
}

////////////////////////////////////////////////////////////////////////////////
/// Retrieve a precision cascade buffer

char *TBranchPrecisionCascade::RetrieveCascade(TTree &tree, Int_t basketnumber)
{
   if (basketnumber > fMaxBaskets)
      return nullptr;
   TBasketPC *basket = GetBasketPC(tree, basketnumber);
   if (!basket)
      return nullptr;
   if (basket->ReadCascade(fBasketSeek[basketnumber], fBasketBytes[basketnumber]))
      return nullptr;

   // Do we need to also return fBasketBytes[fBasketSeek] ?
   return basket->GetBuffer();
}

} // Details
} // ROOT