/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/** \classTBasketPC
\ingroup tree

TBasket equivalent for the precision cascade elements.

**/

#include "TBasketPC.h"
#include "TBranchPrecisionCascade.h"
#include "TBufferFile.h"
#include "TFile.h"
#include "TFileCacheRead.h"
#include "TROOT.h"

#include <mutex>

////////////////////////////////////////////////////////////////////////////////
/// Main Constructor
TBasketPC::TBasketPC(TDirectory *dir, ROOT::Detail::TBranchPrecisionCascade &owner)
   : TKey(dir), fBranchPrecisionCascade(&owner)
{
   SetName(owner.GetName());
   // SetTitle();
   fClassName   = "TBasketPC";
   fVersion    += 1000;
   fBuffer = nullptr;
   fBufferRef   = new TBufferFile(TBuffer::kWrite);
   if (fMotherDir) {
      TFile *file = fMotherDir->GetFile();
      fBufferRef->SetParent(file);
   }
   Streamer(*fBufferRef);
   fKeylen      = fBufferRef->Length();
   fObjlen      = 0;

   // branch->GetTree()->IncrementTotalBuffers(fBufferSize);
}

////////////////////////////////////////////////////////////////////////////////
/// Read the Precision Cascade element to its file.
/// \param[in] n Size of the compressed buffer (the number of bytes to write)
/// \param[in] cascade the data to write
/// \param[in] The estimated uncompressed size of the data (approximation)
/// \return The function returns 0 in case of success, 1 in case of error

Int_t TBasketPC::ReadCascade(Long64_t pos, Long64_t len)
{
   if (!fMotherDir)
      return -1;
   TFile *file = fMotherDir->GetFile();
   if (!file)
      return -1;

   TFileCacheRead *pf = nullptr;
   {
      // R__LOCKGUARD_IMT(gROOTMutex); // Lock for parallel TTree I/O
      // pf = fBranch->GetTree()->GetReadCache(file);
   }

   fBufferRef->SetReadMode();
   Int_t curBufferSize = fBufferRef->BufferSize();
   if (curBufferSize < len) {
      // Experience shows that giving 5% "wiggle-room" decreases churn.
      fBufferRef->Expand(Int_t(len*1.05));
   }
   fBufferRef->Reset();

   if (pf) {
      //TVirtualPerfStats* temp = gPerfStats;
      //if (fBranch->GetTree()->GetPerfStats() != 0) gPerfStats = fBranch->GetTree()->GetPerfStats();
      Int_t st = 0;
      {
         R__LOCKGUARD_IMT(gROOTMutex); // Lock for parallel TTree I/O
         st = pf->ReadBuffer(fBufferRef->Buffer(), pos, len);
      }
      if (st < 0) {
         return 1;
      } else if (st == 0) {
         R__LOCKGUARD_IMT(gROOTMutex);  // Lock for parallel TTree I/O
         // May or may not need to disable the cache.
         // pf->Disable();
         Int_t ret = file->ReadBuffer(fBufferRef->Buffer(), pos, len);
         // if (fc) fc->Enable();
         pf->AddNoCacheBytesRead(len);
         pf->AddNoCacheReadCalls(1);
         if (ret) {
            return 1;
         }
      }
      // gPerfStats = temp;
   } else {
      // Read from the file and unstream the header information.
      // TVirtualPerfStats* temp = gPerfStats;
      // if (fBranch->GetTree()->GetPerfStats() != 0) gPerfStats = fBranch->GetTree()->GetPerfStats();
      R__LOCKGUARD_IMT(gROOTMutex);  // Lock for parallel TTree I/O
      if (file->ReadBuffer(fBufferRef->Buffer(),pos,len)) {
         // gPerfStats = temp;
         return 1;
      }
      // else gPerfStats = temp;
   }

   fBuffer = fBufferRef->Buffer();
   Streamer(*fBufferRef);
   if (IsZombie()) {
      return 1;
   }

   return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Write the Precision Cascade element to its file assuming it is already
/// streamed inside the fBufferRef
/// \param[in] n Size of the compressed buffer (the number of bytes to write)
/// \param[in] The estimated uncompressed size of the data (approximation)
/// \param[in] Corresponding basketnumber in the main TTree/TBranch

Int_t TBasketPC::WriteCascade(Int_t n, Int_t uncompressedSize, Int_t basketnumber)
{
   TFile *file = fMotherDir->GetFile();
   if (!file)
      return 0;
   if (!file->IsWritable()) {
      return -1;
   }

   fBufferRef->SetWriteMode();

   // This mutex prevents multiple TBasket::WriteBuffer invocations from interacting
   // with the underlying TFile at once - TFile is assumed to *not* be thread-safe.
   //
   // The only parallelism we'd like to exploit (right now!) is the compression
   // step - everything else should be serialized at the TFile level.
#ifdef R__USE_IMT
   std::unique_lock<std::mutex> sentry(file->fWriteMutex);
#endif  // R__USE_IMT

   fObjlen = uncompressedSize;
   fCycle = basketnumber;

   fBuffer = fBufferRef->Buffer();
   Create(n, file);
    
   fBufferRef->SetBufferOffset(0);
   Streamer(*fBufferRef);  //write key itself again

   Int_t nBytes = WriteFileKeepBuffer(file);
   return nBytes>0 ? fKeylen + n : -1;
}

////////////////////////////////////////////////////////////////////////////////
/// Write the Precision Cascade element to its file.
/// \param[in] n Size of the compressed buffer (the number of bytes to write)
/// \param[in] cascade the data to write
/// \param[in] The estimated uncompressed size of the data (approximation)
/// \param[in] Corresponding basketnumber in the main TTree/TBranch

Int_t TBasketPC::WriteCascade(Int_t n, Char_t *cascade, Int_t uncompressedSize, Int_t basketnumber)
{
   fBufferRef->SetBufferOffset(fKeylen);
   fBufferRef->WriteFastArray(cascade, n);
   return WriteCascade(n, uncompressedSize, basketnumber);
}
////////////////////////////////////////////////////////////////////////////////
/// Stream a class object.

void TBasketPC::Streamer(TBuffer &b)
{
   TKey::Streamer(b);
}