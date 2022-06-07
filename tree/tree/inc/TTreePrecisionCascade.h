// @(#)root/tree:$Id$
// Author: Philippe Canal 06/01/22

/*************************************************************************
 * Copyright (C) 1995-2022, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TTreePrecisionCascade
#define ROOT_TTreePrecisionCascade

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TTreePrecisionCascade                                                //
//                                                                      //
// Auxiliary objects holding the TBaskets information for the           //
// supplemental parts of the precision cascades                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "Rtypes.h"
#include "THashList.h"
#include "TNamed.h"

class TTree;
class TDirectory;

namespace ROOT {
namespace Detail {

class TBranchPrecisionCascade;

class TTreePrecisionCascade : public TNamed
{
private:
   THashList   fBranches;            ///< Collection of TBranchPrecisionCascade.
   UInt_t      fCascadeLevel = 0;    ///< Position in the precision cascade.
   UInt_t      fTreeUniqueID = 0;    ///< TTree unique identifier
   TDirectory *fDirectory = nullptr; ///<! Pointer to directory holding this tree cascade

public:

   TTreePrecisionCascade() = default;

   TTreePrecisionCascade(TTree &tree, UInt_t cascadeLevel);

   TBranchPrecisionCascade *GetBranchPrecisionCascade(const char *fullname);

   ~TTreePrecisionCascade() = default;

   ClassDefOverride(TTreePrecisionCascade, 3); // TBranchPrecisionCascade holder
};

} // Details
} // ROOT

#endif // ROOT_TTreePrecisionCascade