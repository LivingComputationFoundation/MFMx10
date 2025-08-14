#include "UClassLibrary.h"

namespace MFM {
  void UClassLibrary::takeEW(EW& ew) {
    MFM_API_ASSERT_NULL(mEW);
    mEW = &ew;
  }

  UClass * UClassLibrary::getCurrentEWUClass(u16 sitenum) const {
    MFM_API_ASSERT_ARG(sitenum <= 40u);
    MFM_API_ASSERT_STATE(mEW!=0);
    const P4Atom & a = mEW->getAtom(sitenum);
    u32 atype = a.GetType();
    if (atype >= mUClassCount) return 0;
    return mAllUClasses[atype];
  }

  UClass * UClassLibrary::getCurrentStorageUClass(u16 storageid) const {
    UCStgType ucst = getStgType(storageid);
    switch (ucst) {
    case UCStgType::UCST_EWS:
      return getCurrentEWUClass(/*SiteNumber*/ storageid);
    default: break;
    }
    return 0;
  }

  const URefDescriptor * UClassLibrary::getURefDescriptor(URef ur) const {
    UClass * uc = getCurrentStorageUClass(ur.getURefStgId());
    if (!uc) return 0;

    u16 urn = ur.getURefNumber();
    if (urn >= uc->mURefCount) return 0;

    return &uc->mAllURefs[urn];
  }

  UClass * UClassLibrary::getStaticUClass(URef ur) {
    return 0;
  }
  UClass * UClassLibrary::getEffectiveClass(URef ur) {
    return 0;
  }

  u32 UClassLibrary::getUClassIdIfAny(const char * name) {
    for (u32 i = 1u; i < mUClassCount; ++i) {
      UClass * uc = mAllUClasses[i];
      if (0==strcmp(name,uc->mName))
        return i;               // found it
    }
    return 0u;                  // no such uclass
  }

  u32 UClassLibrary::getIndexOfBaseNamed(const UClass &obj, const char * basename) const
  {
    MFM_API_ASSERT_NONNULL(basename);
    for (u32 i = 0u; i < obj.mAllBaseCount; ++i) {
      u32 bcid = obj.mAllBaseList[i];
      MFM_API_ASSERT_STATE(bcid < mUClassCount);
      UClass * bc = mAllUClasses[bcid];
      if (0==strcmp(basename,bc->mName))
        return i;               // found it
    }
    return U32_MAX;             // didn't find it
  }

  u32 UClassLibrary::getIndexOfDataMemberNamed(const UClass &obj, const char * membname) const
  {
    MFM_API_ASSERT_NONNULL(membname);

    for (u32 i = 0u; i < obj.mAllDataMemberCount; ++i) {
      const UClassDataMember & dm = obj.mAllDataMemberList[i];
      if (0==strcmp(membname,dm.mName))
        return i;               // found it
    }
    return U32_MAX;             // didn't find it
  }

#include "UCLGENERATEDCODE.inc"  
}
