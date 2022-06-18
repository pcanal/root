// @(#)root/zip:$Id$
// Author: David Dagenhart   May 2011

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "Compression.h"
#include <stdexcept>
#include "PrecisionCascadeConfigArrayContent.h"

namespace ROOT {

////////////////////////////////////////////////////////////////////////////////

  int CompressionSettings(RCompressionSetting::EAlgorithm::EValues algorithm,
                          int compressionLevel)
  {
    if (compressionLevel < 0) compressionLevel = 0;
    if (compressionLevel > 99) compressionLevel = 99;
    int algo = algorithm;
    if (algorithm >= ROOT::RCompressionSetting::EAlgorithm::kUndefined) algo = 0;
    return algo * 100 + compressionLevel;
  }

  int CompressionSettings(ROOT::ECompressionAlgorithm algorithm,
                          int compressionLevel)
  {
    if (compressionLevel < 0) compressionLevel = 0;
    if (compressionLevel > 99) compressionLevel = 99;
    int algo = algorithm;
    if (algorithm >= ROOT::ECompressionAlgorithm::kUndefinedCompressionAlgorithm) algo = 0;
    return algo * 100 + compressionLevel;
  }


  PrecisionCascadeCompressionConfig::PrecisionCascadeCompressionConfig(
      RCompressionSetting::EAlgorithm::EValues algo,
      const std::vector<Int_t> &levels, bool storeResidual /* = false */)
  {
    if (algo != RCompressionSetting::EAlgorithm::kBLAST) {
      std::string msg("Requestion compression algorithm does not support Precision Cascade: ");
      msg += algo;
      throw std::runtime_error(msg);
    }
    fIsPrecisionCascade = (levels.size() > 1 || storeResidual);
    fAlgorithm = algo;
    if (!levels.empty())
      fLevel = levels[0];
    if (fIsPrecisionCascade) {
      using ROOT::Internal::PrecisionCascadeConfigArrayContent;
      fNConfig = PrecisionCascadeConfigArrayContent::SizeOf(levels.size());
      fConfigArray = new char[fNConfig];
      auto content = reinterpret_cast<PrecisionCascadeConfigArrayContent*>(fConfigArray);
      content->fStoreResidual = storeResidual;
      content->fLen = levels.size();
      memcpy(&(content->fLevels), levels.data(), levels.size() * sizeof(Int_t));
    }
  }
}
