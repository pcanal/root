// @(#)root/tree:$Id$
// Author: Philippe Canal 06/01/22

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/** \class ROOT::Detail::TTreePrecisionCascade
\ingroup tree

An Auxiliary objects holding the TBaskets information for the
supplemental parts of the precision cascades.

**/

#include "TBranchPrecisionCascade.h"
#include "TTreePrecisionCascade.h"
#include "TTree.h"
#include "TProcessID.h"
#include <iostream>
#include "TDirectory.h"
#include "TFile.h"

namespace ROOT {
namespace Detail {

TTreePrecisionCascade::TTreePrecisionCascade(TTree &tree, UInt_t level) :
   TNamed(tree.GetName(), "Precision Cascade"), fCascadeLevel(level)
{
   fBranches.SetOwner(kTRUE);
   if (!tree.TestBit(kIsReferenced)) {
      TProcessID::AssignID(&tree);
   }
   fTreeUniqueID = tree.GetUniqueID();
   fName.Form("%s_pc%d", tree.GetName(),level);
}

TTreePrecisionCascade::~TTreePrecisionCascade()
{
   if (fOwnsFile && fDirectory)
      delete fDirectory->GetFile();
}

TBranchPrecisionCascade *TTreePrecisionCascade::GetBranchPrecisionCascade(const char *fullname)
{
   return static_cast<TBranchPrecisionCascade*>(fBranches.FindObject(fullname));
}

/// Return false if the PrecisionCascade is not of the expected level for the given TTree.
Bool_t TTreePrecisionCascade::Verify(TTree& tree, UInt_t level) const {
   return tree.GetUniqueID() == fTreeUniqueID && level == fCascadeLevel;
}

/// Print the object's information.
void TTreePrecisionCascade::Print(Option_t * /* option = "" */) const
{
   std::cout << "TTreePrecisionCascade: " << GetName() << "\tTTree uniqueID: " << fTreeUniqueID << "\tlevel: " << fCascadeLevel << std::endl;
}

/// Remove reference to a deleted object
void TTreePrecisionCascade::RecursiveRemove(TObject *obj)
{
   if (obj == fDirectory || (fDirectory && obj == fDirectory->GetFile()))
      fDirectory = nullptr;
}

} // Details
} // ROOT