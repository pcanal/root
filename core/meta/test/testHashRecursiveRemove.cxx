#include "TClass.h"
#include "TInterpreter.h"
#include <memory>

#include "gtest/gtest.h"

class FirstOverload : public TObject
{
public:
   virtual ULong_t     Hash() const { return 1; }

   ClassDefInline(FirstOverload, 2);
};

const char* gCode = R"CODE(

   #include "TROOT.h"
   #include <iostream>

   class FirstOverload : public TObject
   {
   public:
      virtual ULong_t     Hash() const { return 1; }

      ClassDefInline(FirstOverload, 2);
   };

   class SecondOverload : public FirstOverload // Could also have used TNamed.
   {
   public:
      virtual ULong_t     Hash() const { return 2; }

      ClassDefInline(SecondOverload, 2);
   };

   class SecondNoHash : public FirstOverload // Could also have used TNamed.
   {
   public:

      ClassDefInline(SecondNoHash, 2);
   };

   class SecondAbstract : public FirstOverload // Could also have used TNamed.
   {
   public:
      virtual int Get() = 0;

      ClassDef(SecondAbstract, 2);
   };

   class Third : public SecondAbstract
   {
   public:
      int Get() override { return 0; };

      ClassDefInlineOverride(Third, 2);
   };

   class FirstOverloadCorrect : public TObject
   {
   public:
      ~FirstOverloadCorrect() {
         ROOT::CallRecursiveRemoveIfNeeded(*this);
      }
      virtual ULong_t     Hash() const { return 3; }

      ClassDefInline(FirstOverloadCorrect, 2);
   };

   class SecondCorrectAbstract : public FirstOverloadCorrect // Could also have used TNamed.
   {
   public:
      virtual int Get() = 0;

      ClassDef(SecondCorrectAbstract, 2);
   };

   class SecondCorrectAbstractHash : public FirstOverloadCorrect // Could also have used TNamed.
   {
   public:
      ~SecondCorrectAbstractHash() {
         ROOT::CallRecursiveRemoveIfNeeded(*this);
      }

      virtual ULong_t Hash() const { return 4; }
      virtual int     Get() = 0;

      ClassDef(SecondCorrectAbstractHash, 2);
   };

   class ThirdCorrect : public SecondCorrectAbstract
   {
   public:
      int Get() override { return 0; };

      ClassDefInlineOverride(ThirdCorrect, 2);
   };

   class SecondInCorrectAbstract : public FirstOverloadCorrect // Could also have used TNamed.
   {
   public:
      virtual ULong_t Hash() const { return 5; }
      virtual int     Get() = 0;

      ClassDef(SecondInCorrectAbstract, 2);
   };

   class ThirdInCorrect : public SecondInCorrectAbstract
   {
   public:
      int Get() override { return 0; };

      ClassDefInlineOverride(ThirdInCorrect, 2);
   };

)CODE";

const char *gErrorOutput = R"OUTPUT(Error in <ROOT::Internal::TCheckHashRecurveRemoveConsistency::CheckRecursiveRemove>: The class FirstOverload overrides TObject::Hash but does not call TROOT::RecursiveRemove in its destructor.
Error in <ROOT::Internal::TCheckHashRecurveRemoveConsistency::CheckRecursiveRemove>: The class SecondOverload overrides TObject::Hash but does not call TROOT::RecursiveRemove in its destructor.
Error in <ROOT::Internal::TCheckHashRecurveRemoveConsistency::CheckRecursiveRemove>: The class FirstOverload overrides TObject::Hash but does not call TROOT::RecursiveRemove in its destructor.
Error in <ROOT::Internal::TCheckHashRecurveRemoveConsistency::CheckRecursiveRemove>: The class FirstOverload overrides TObject::Hash but does not call TROOT::RecursiveRemove in its destructor.
Error in <ROOT::Internal::TCheckHashRecurveRemoveConsistency::CheckRecursiveRemove>: The class SecondInCorrectAbstract overrides TObject::Hash but does not call TROOT::RecursiveRemove in its destructor.
Error in <ROOT::Internal::TCheckHashRecurveRemoveConsistency::CheckRecursiveRemove>: The class WrongSetup overrides TObject::Hash but does not call TROOT::RecursiveRemove in its destructor.
)OUTPUT";


void DeclareFailingClasses() {
   gInterpreter->Declare(gCode);
}

TEST(HashRecursiveRemove,RootClasses)
{
   EXPECT_TRUE(TClass::GetClass("TObject")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("TNamed")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("TH1")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("TH1F")->HasConsistentHashMember());

   EXPECT_TRUE(TClass::GetClass("TEnvRec")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("TDataType")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("TObjArray")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("TList")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("THashList")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("TClass")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("TInterpreter")->HasConsistentHashMember());
   //EXPECT_TRUE(TClass::GetClass("TCling")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("TMethod")->HasConsistentHashMember());
   //EXPECT_TRUE(TClass::GetClass("ROOT::Internal::TCheckHashRecurveRemoveConsistency")->HasConsistentHashMember());
}

TEST(HashRecursiveRemove,FailingClasses)
{
   DeclareFailingClasses();

   testing::internal::CaptureStderr();

   EXPECT_NE(nullptr,TClass::GetClass("FirstOverload"));
   EXPECT_FALSE(TClass::GetClass("FirstOverload")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("SecondOverload")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("SecondNoHash")->HasConsistentHashMember());

   EXPECT_FALSE(TClass::GetClass("SecondAbstract")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("Third")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("FirstOverloadCorrect")->HasConsistentHashMember());

   EXPECT_FALSE(TClass::GetClass("SecondCorrectAbstract")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("SecondCorrectAbstractHash")->HasConsistentHashMember());
   EXPECT_TRUE(TClass::GetClass("ThirdCorrect")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("SecondInCorrectAbstract")->HasConsistentHashMember());
   EXPECT_FALSE(TClass::GetClass("ThirdInCorrect")->HasConsistentHashMember());
   EXPECT_FALSE(WrongSetup::Class()->HasConsistentHashMember());

   std::string output = testing::internal::GetCapturedStderr();
   EXPECT_EQ(gErrorOutput,output);
}

#include "THashList.h"
#include "TROOT.h"

constexpr size_t kHowMany = 10000;

TEST(HashRecursiveRemove,SimpleDelete)
{
   THashList cont;

   for(size_t i = 0; i < kHowMany; ++i) {
      TNamed *n = new TNamed(TString::Format("n%ld",i),TString(""));
      n->SetBit(kMustCleanup);
      cont.Add(n);
   }
   cont.Delete();

   EXPECT_EQ(0, cont.GetSize());
}

TEST(HashRecursiveRemove,DeleteWithRecursiveRemove)
{
   THashList cont;
   TList todelete;

   for(size_t i = 0; i < kHowMany; ++i) {
      TNamed *n = new TNamed(TString::Format("n%ld",i),TString(""));
      n->SetBit(kMustCleanup);
      cont.Add(n);
      todelete.Add(n);
   }
   gROOT->GetListOfCleanups()->Add(&cont);

   for(auto o : todelete)
     delete o;

   todelete.Clear("nodelete");

   EXPECT_EQ(0, cont.GetSize());
}

TEST(HashRecursiveRemove,DeleteBadHashWithRecursiveRemove)
{
   THashList cont;
   TList todelete;

   for(size_t i = 0; i < kHowMany; ++i) {
      TObject *o;
      if (i%2) o = (TObject*)TClass::GetClass("FirstOverload")->New();
      else o = new WrongSetup;
      o->SetBit(kMustCleanup);
      cont.Add(o);
      todelete.Add(o);
   }
   gROOT->GetListOfCleanups()->Add(&cont);

   for(auto o : todelete) {
     delete o;
   }

   EXPECT_EQ(0, cont.GetSize());

   todelete.Clear("nodelete");

   // Avoid spurrious/redundant error messages in case of failure.
   cont.Clear("nodelete");
  }
