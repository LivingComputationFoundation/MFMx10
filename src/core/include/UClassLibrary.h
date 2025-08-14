#ifndef UCLASSLIBRARY_H /* -*- C++ -*- */
#define UCLASSLIBRARY_H

#include "itype.h"
#include "URefInfo.h"
#include "UClass.h"
#include "EW.h"

namespace MFM {

  class UClassLibrary {
  public:
    UClassLibrary() ;

    UClass * getCurrentEWUClass(u16 storageid) const ;

    UClass * getCurrentStorageUClass(u16 storageid) const ;

    const URefDescriptor * getURefDescriptor(URef ur) const ;

    UClass * getStaticUClass(URef ur) ;
    UClass * getEffectiveClass(URef ur) ;

    URef getStgURef(u32 stgid) {
      URef ret;
      ret.setURefStgId(stgid);
      ret.setURefNumber(0u);
      return ret;
    }

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

} // end namespace MFM

#endif /* UCLASSLIBRARY_H */

