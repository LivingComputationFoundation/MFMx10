#ifndef UCONTEXT_H /* -*- C++ -*- */
#define UCONTEXT_H

#include "itype.h"
#include "UClassLibrary.h"
#include "Random.h"

namespace MFM {

  struct UContext {
    UClassLibrary * mUCL;
    EW * mEW;
    Random mRandom;
    UContext()
      : mUCL(0)
      , mEW(0)
      , mRandom(0)
    { }
  };

} // end namespace MFM

#endif /* UCONTEXT_H */

