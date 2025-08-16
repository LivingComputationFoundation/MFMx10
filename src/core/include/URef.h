#ifndef UREF_H /* -*- C++ -*- */
#define UREF_H

#include "itype.h"

namespace MFM {

  enum class UCStgType : u8 {
    UCST_EWS,  //< (atom in) event window site
    UCST_ARS,  //< atom register temps
    UCST_ILL,  //< illegal stg type
    UCST_PHY   //< physics-defined global temps
  };

  inline UCStgType getStgType(u32 stgid) {
    if (stgid <= 40u) return UCStgType::UCST_EWS; // 0..40
    if (stgid <= 62u) return UCStgType::UCST_ARS; // 41..62
    if (stgid <= 63u || stgid >= 127) return UCStgType::UCST_ILL; // 63 and 127+
    return UCStgType::UCST_PHY;                      // 64..126
  }

  struct URef {
    u8 mStgId;
    u8 mRefNum;

    u16 getURefRaw() const { return (((u16) mStgId)<<8u) | mRefNum; }
    u8 getURefStgId() const { return mStgId; }
    u8 getURefNumber() const { return mRefNum; }

    URef getDMURef(u32 dmidx) const ;

    URef getBCURef(u32 bcidx) const ;

    void setURefStgId(u8 stg) {
      mStgId = stg;
    }
    void setURefNumber(u8 num) {
      mRefNum = num;
    }
  };

} // end namespace MFM

#endif /* UREF_H */

