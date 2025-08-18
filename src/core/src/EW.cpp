#include "EW.h"
#include "Logger.h"
#include "UClassLibrary.h"
#include "UContext.h"

namespace MFM {
  EW theEW;

  void EW::doEvent() {
    LOG.Message("DOEVENT");
    //P4Atom & ctr = getAtom(0u);
    URef urctr = theUCL.getStgURef(0);
    UClass * ucp = theUCL.getCurrentEWUClass(0);
    MFM_API_ASSERT_NONNULL(ucp);
    BehavePtr bp = ucp->mBehavePtr;
    if (bp) {
      UContext ucon;
      ucon.mUCL = &theUCL;
      ucon.mEW = &theEW;
      LOG.Message("CALLINGO");
      (*bp)(ucon,urctr);
    }
    LOG.Message("DONGO");
  }

}
