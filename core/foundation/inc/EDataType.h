// Author: Rene Brun   04/02/95
// Extracted from TDataType.h

/*************************************************************************
 * Copyright (C) 1995-2021, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_EDataType
#define ROOT_EDataType

enum EDataType {
   kChar_t   = 1,  kUChar_t  = 11, kShort_t    = 2,  kUShort_t = 12,
   kInt_t    = 3,  kUInt_t   = 13, kLong_t     = 4,  kULong_t  = 14,
   kFloat_t  = 5,  kDouble_t =  8, kDouble32_t = 9,  kchar     = 10,
   kBool_t   = 18, kLong64_t = 16, kULong64_t  = 17, kOther_t  = -1,
   kNoType_t = 0,  kFloat16_t= 19,
   kCounter  =  6, kCharStar = 7,  kBits     = 15 /* for compatibility with TStreamerInfo */,
   kVoid_t   = 20,

   kDataTypeAliasUnsigned_t = 21,
   kDataTypeAliasSignedChar_t = 22,
   // could add "long int" etc
   kNumDataTypes,

   // Modifiers (See TStreamerInfo)
   kOffsetL = 20, /* Fixed size array */
   kOffsetP = 40  /* Variable size array or Pointer to object */
};

#endif // ROOT_EDataType