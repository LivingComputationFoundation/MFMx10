#include "URef.h"
#include "Fail.h"

namespace MFM {
  URef URef::getDMURef(u32 dmidx) const {
    FAIL(INCOMPLETE_CODE);
    return *this; // NOT REACHED
  }

  URef URef::getBCURef(u32 bcidx) const {
    FAIL(INCOMPLETE_CODE);
    return *this; // NOT REACHED
  }
}
