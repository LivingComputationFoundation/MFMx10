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
    u16 mData;

    static const u16 STGID_BITS = 7u;
    static const u16 UREFNUM_BITS = 9u;
    static const u16 STGID_MASK = ((1u<<STGID_BITS)-1u);
    static const u16 UREFNUM_MASK = ((1u<<UREFNUM_BITS)-1u);

    u16 getURefRaw() const { return mData; }
    u16 getURefStgId() const { return (mData>>UREFNUM_BITS)&STGID_MASK; }
    u16 getURefNumber() const { return mData&UREFNUM_MASK; }

    URef getDMURef(u32 dmidx) const ;

    URef getBCURef(u32 bcidx) const ;

    void setURefStgId(u16 stg) {
      if (unlikely(stg > STGID_MASK)) stg = STGID_MASK;
      mData = (stg<<UREFNUM_BITS)|getURefNumber();
    }
    void setURefNumber(u16 num) {
      if (unlikely(num > UREFNUM_MASK)) num = UREFNUM_MASK;
      mData = (mData&(STGID_MASK<<UREFNUM_BITS))|num;
    }
  };

} // end namespace MFM

#endif /* UREF_H */

