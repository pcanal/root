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

TBranchPrecisionCascade *TTreePrecisionCascade::GetBranchPrecisionCascade(const char *fullname)
{
   return static_cast<TBranchPrecisionCascade*>(fBranches.FindObject(fullname));
}



} // Details
} // ROOT