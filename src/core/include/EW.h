#ifndef EW_H /* -*- C++ -*- */
#define EW_H

#include "itype.h"
#include "URef.h"
#include "P4Atom.h"

namespace MFM {

  struct EWSite {
    P4Atom mSiteAtom;
  };

  struct EW {
    EWSite mSites[42];
    P4Atom & getAtom(u32 site) {
      MFM_API_ASSERT_ARG(site < 42);
      return mSites[site].mSiteAtom;
    }
    void doEvent() ;
  };

  extern EW theEW;

} // end namespace MFM

#endif /* EW_H */

