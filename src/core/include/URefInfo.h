#ifndef UREFINFO_H /* -*- C++ -*- */
#define UREFINFO_H

#include "itype.h"
#include "URef.h"

namespace MFM {

  struct URefDescriptor {
    u16 mStgType;
    u16 mStaticType;
    u16 mEffectiveSelf;
    u8 mPosition;
    u8 mSize;
  };

  class URefInfo {
  public:
    virtual URefDescriptor getDescriptor(const URef ur) = 0;
    
  };

} // end namespace MFM

#endif /* UREFINFO_H */

