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
#include <string>
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
      std::string msg("Requested compression algorithm does not support Precision Cascade: ");
      msg += algo;
      throw std::runtime_error(msg);
    }
    for(auto l : levels) {
      if (l <= 0) {
        std::string msg("Requested compression level is not supported for  Precision Cascade: ");
        msg += l;
        throw std::runtime_error(msg);
      }
    }
    fNPrecisionCascade = (levels.empty() ? 0 : levels.size() - 1) + storeResidual;
    fAlgorithm = algo;
    if (!levels.empty())
      fLevel = levels[0];
    if (fNPrecisionCascade) {
      using ROOT::Internal::PrecisionCascadeConfigArrayContent;
      fNConfig = PrecisionCascadeConfigArrayContent::SizeOf(levels.size() + storeResidual);
      fConfigArray = new char[fNConfig];
      auto content = reinterpret_cast<PrecisionCascadeConfigArrayContent*>(fConfigArray);
      content->fLen = levels.size() + storeResidual;
      UChar_t *levelArray = &(content->fLevels);
      for(size_t i = 0; i < levels.size(); ++i) {
        levelArray[i] = levels[i];
      }
      if (storeResidual)
        levelArray[levels.size()] = 0;
    }
  }
}
