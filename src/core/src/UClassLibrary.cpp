#include "UClassLibrary.h"
#include "EW.h"
#include "BitVector.h"

namespace MFM {
  AbstractBitVector * UClassLibrary::getABV(u32 stgid) const {
    AbstractBitVector *abv = 0;
    switch (getStgType(stgid)) {
    case UCStgType::UCST_EWS:
      {
        P4Atom & p4 = theEW.getAtom(stgid);
        abv = &p4.GetBits();
        break;
      }
    default:     FAIL(INCOMPLETE_CODE);
    }
    return abv;
  }

  AbstractBitVector & UClassLibrary::getURefAccessOrFail(URef ur, URefDescriptor & urd) const {
    const URefDescriptor * purd = getURefDescriptor(ur);
    MFM_API_ASSERT_NONNULL(purd);
    AbstractBitVector * pabv = getABV(ur.getURefStgId());
    MFM_API_ASSERT_NONNULL(pabv);
    urd = *purd;
    return *pabv;
  }

  u32 UClassLibrary::readBitsRaw32(URef ur) const {
    URefDescriptor urd;
    AbstractBitVector & abv = getURefAccessOrFail(ur, urd);
    return abv.read32(urd.mPosition,urd.mSize);
  }

  u64 UClassLibrary::readBitsRaw64(URef ur) const {
    URefDescriptor urd;
    AbstractBitVector & abv = getURefAccessOrFail(ur, urd);
    return abv.read64(urd.mPosition,urd.mSize);
  }

  void UClassLibrary::writeBitsRaw32(URef ur, u32 newval) const {
    URefDescriptor urd;
    AbstractBitVector & abv = getURefAccessOrFail(ur, urd);
    abv.write32(urd.mPosition,urd.mSize,newval);
  }

  void UClassLibrary::writeBitsRaw64(URef ur, u64 newval) const {
    URefDescriptor urd;
    AbstractBitVector & abv = getURefAccessOrFail(ur, urd);
    abv.write64(urd.mPosition,urd.mSize,newval);
  }

  ByteSink& UClassLibrary::reportURef(URef ur, ByteSink& bs) {
    const URefDescriptor * urd = getURefDescriptor(ur);
    if (urd) {
      const char * name = "?";
      UClass * stguc = getCurrentStorageUClass(ur.getURefStgId());
      if (stguc) {
        if (ur.getURefNumber() < stguc->mURefCount)
          name = (stguc->mURefNames)[ur.getURefNumber()];
      }

      bs.Printf("UR%d/%d = '%s' stg %d, stat %d, eff %d, pos %d, siz %d, 1sl %d, #sl %d",
                ur.getURefStgId(), ur.getURefNumber(),
                name,
                urd->mStgType,
                urd->mStaticType,
                urd->mEffectiveSelf,
                urd->mPosition,
                urd->mSize,
                urd->mFirstTransIdx,
                urd->mTransCount);
    } else
      bs.Printf("NO URD?");
    return bs;
  }

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

  URef UClassLibrary::selectURef(URef ur, u32 idx) const {
    u8 stgid = ur.getURefStgId();
    UClass * stg = getCurrentStorageUClass(stgid);
    MFM_API_ASSERT_NONNULL(stg);
    u8 refnum = ur.getURefNumber();
    MFM_API_ASSERT_STATE(refnum < stg->mURefCount);
    URefDescriptor urd = stg->mAllURefs[refnum];
    MFM_API_ASSERT_ARG(idx < urd.mTransCount);
    u32 transidx = urd.mFirstTransIdx + idx;
    MFM_API_ASSERT_STATE(transidx < stg->mURefTransitionCount);
    u32 newrn = stg->mURefTransitions[transidx];
    //LOG.Message("ZONG stg %d rn %d 1st %d newrn %d",stgid,refnum,urd.mFirstTransIdx,newrn);
    ur.setURefNumber(newrn);
    return ur;
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
