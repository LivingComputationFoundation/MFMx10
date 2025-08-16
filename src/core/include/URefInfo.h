#ifndef UREFINFO_H /* -*- C++ -*- */
#define UREFINFO_H

#include "itype.h"
#include "URef.h"

namespace MFM {

  struct URefDescriptor {
    u8 mStgType;
    u8 mStaticType;
    u8 mEffectiveSelf;
    u8 mPosition;
    u8 mSize;
    u8 mFirstTransIdx;
    u8 mTransCount;
  };

  class URefInfo {
  public:
    virtual URefDescriptor getDescriptor(const URef ur) = 0;
    
  };

} // end namespace MFM

#endif /* UREFINFO_H */

