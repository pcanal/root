// @(#)root/base:$Id$
// Author: Philippe Canal 2019

/*************************************************************************
 * Copyright (C) 1995-2019, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TNotifyLink
#define ROOT_TNotifyLink

/** \class TNotifyLink
\ingroup Base

Use this class to create to be able to add more object to be
notified (via TObject::Notify).  Note that neither SetNotify
nor TNotifyLink takes ownership of its parameter(s).

eg.
```
auto notify = new TNotifyLink(object, fChain->GetNotify());
fChain->SetNotify(notify);
```
**/

template <class Type>
class TNotifyLink : public TObject {
private:
   Type    *fCurrent = nullptr;
   TObject *fNext = nullptr;

public:
   TNotifyLink(Type *current, TObject *next) : fCurrent(current), fNext(next) {}

   // Call Notify on the current and next object.
   Bool_t Notify() override
   {
      auto result = fCurrent ? fCurrent->Notify() : kTRUE;
      if (fNext) result &= fNext->Notify();
      return result;
   }

   ClassDefOverride(TNotifyLink, 0);
};

#endif // ROOT_TNotifyLink