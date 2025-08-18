#ifndef UCLASSLIBRARY_H /* -*- C++ -*- */
#define UCLASSLIBRARY_H

#include "itype.h"
#include "URefInfo.h"
#include "UClass.h"
#include "EW.h"
#include "Logger.h"

namespace MFM {

  class UClassLibrary {
  public:
    UClassLibrary() ;

    UClass * getUClass(UClassId ucid) const ;

    UClass * getCurrentEWUClass(u16 storageid) const ;

    UClass * getCurrentStorageUClass(u16 storageid) const ;

    const URefDescriptor * getURefDescriptor(URef ur) const ;
    const URefDescriptor getURefDescriptorOrFail(URef ur) const {
      const URefDescriptor * purd = getURefDescriptor(ur);
      MFM_API_ASSERT_NONNULL(purd);
      return *purd;
    }

    template <u32 BITS>
    BitVector<BITS> & getURefAccessOrFail(URef ur, URefDescriptor & urd) const ;

    UClass * getStaticUClass(URef ur) ;
    UClass * getEffectiveClass(URef ur) ;

    u32 readBitsRaw32(URef ur) const ;
    u64 readBitsRaw64(URef ur) const ;
    void writeBitsRaw32(URef ur, u32 newval) const ;
    void writeBitsRaw64(URef ur, u64 newval) const ;

    ByteSink& reportURef(URef uref, Logger & to) {
      ByteSink * bs = to.GetByteSink();
      MFM_API_ASSERT_NONNULL(bs);
      to.VreportPrefix(Logger::MESSAGE);
      reportURef(uref, *bs);
      to.VreportSuffix();
      return *bs;
    }

    ByteSink& reportURef(URef uref, ByteSink & to) ;

    URef getStgURef(u32 stgid) {
      URef ret;
      ret.setURefStgId(stgid);
      ret.setURefNumber(0u);
      return ret;
    }

    //AbstractBitVector * getABV(u32 stgid) const ;

    URef selectURef(URef ur, u32 idx) const ;

    void takeEW(EW& ew) ;

    EW& getEW() {
      MFM_API_ASSERT_NONNULL(mEW);
      return *mEW;
    }

    u32 getUClassIdIfAny(const char * name) ;

    u32 getUClassCount() const {
      return mUClassCount;
    }

    u32 getIndexOfBaseNamed(const UClass & obj, const char * basename) const ;

    u32 getIndexOfDataMemberNamed(const UClass & obj, const char * membname) const ;


  private:
    EW * mEW;
    u32 mUClassCount;
    UClass ** mAllUClasses;
  };

  extern UClassLibrary theUCL;

} // end namespace MFM



#endif /* UCLASSLIBRARY_H */

