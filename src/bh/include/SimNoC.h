#ifndef SIMNOC_H /* -*- C++ -*- */
#define SIMNOC_H

#include "itype.h"
#include "UClass.h"
#include "Fail.h"
#include "P4Atom.h"
#include "Logger.h"
#include "UContext.h"

namespace MFM {

  // Fake 'NoC' stub for dispatching event windows
  struct SimNoC {
#if 1
    enum {
      WORLD_WIDTH = 60,
      WORLD_HEIGHT = 40
    };
    P4Atom mWorld[WORLD_WIDTH][WORLD_HEIGHT];

    void reseed(UClassId ucid) ;

    u32 mTotalEvents;
    u32 mDispatchableEvents;
    u32 mDispatchedEvents;
    u32 mCompletedEvents;

    UContext mUContext;
    
    SimNoC() ;

    void dump(Logger & log) ;

    void doEvent() ;

    void doEventAt(u32 x, u32 y) ;
#endif
  };

  extern SimNoC theSimNoC;
  
} // end namespace MFM

#endif /* SIMNOC_H */

