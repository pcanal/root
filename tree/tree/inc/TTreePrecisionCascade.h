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
   Bool_t      fOwnsFile = kFALSE;   ///<! Indicates whether this object owns the TFile

public:

   TTreePrecisionCascade() = default;

   TTreePrecisionCascade(TTree &tree, UInt_t cascadeLevel);

   TBranchPrecisionCascade *GetBranchPrecisionCascade(const char *fullname);

   ~TTreePrecisionCascade();

   /// Register the directory in which this object is stored
   ///
   /// \param[in] dir Pointer to the TDirectory
   /// \param[in] own Indicates whether this own should take ownership of the TFile containing the directory.
   void SetDirectory(TDirectory *dir, Bool_t own) {
      fDirectory = dir;
      fOwnsFile = own;
   }

   /// Return false if the PrecisionCascade is not of the expected level for the given TTree.
   Bool_t Verify(TTree& tree, UInt_t level) const;

   void Print(Option_t *option="") const override;

   const THashList &GetBranches() const { return fBranches; }

   void RecursiveRemove(TObject *obj) override;

   ClassDefOverride(TTreePrecisionCascade, 3); // TBranchPrecisionCascade holder
};

} // Details
} // ROOT

#endif // ROOT_TTreePrecisionCascade