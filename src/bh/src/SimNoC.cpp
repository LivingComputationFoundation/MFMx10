#include "SimNoC.h"
#include "Random.h"
#include "UClassLibrary.h"
#include "UContext.h"
#include "UClassLibrary.h"
#include "MDist.h"
#include "EW.h"

namespace MFM {

  SimNoC theSimNoC;

  SimNoC::SimNoC() 
    : mTotalEvents(0u)
    , mDispatchableEvents(0u)
    , mDispatchedEvents(0u)
    , mCompletedEvents(0u)
  {
    mUContext.mUCL = &theUCL;
    mUContext.mEW = &theEW;
  }

  void SimNoC::reseed(UClassId ucid) {
    P4Atom emp; // Default is type empty
    for (u32 i = 0u; i < WORLD_WIDTH; ++i)
      for (u32 j = 0u; j < WORLD_HEIGHT; ++j)
        mWorld[i][j] = emp;
    P4Atom seed(ucid);
    mWorld[WORLD_WIDTH/2][WORLD_HEIGHT/2] = seed;
    LOG.Message("SimNoC: Seeded %d at [%d,%d]",
                ucid,
                WORLD_WIDTH/2,
                WORLD_HEIGHT/2);
  }

  void SimNoC::doEvent() {
    u32 x = mUContext.mRandom.Create(WORLD_WIDTH);
    u32 y = mUContext.mRandom.Create(WORLD_HEIGHT);
    doEventAt(x,y);
    //doEventAt(WORLD_WIDTH/2,WORLD_HEIGHT/2);
  }

  void SimNoC::doEventAt(u32 x, u32 y) {
    MFM_API_ASSERT_ARG(x<WORLD_WIDTH);
    MFM_API_ASSERT_ARG(y<WORLD_HEIGHT);

    ++mTotalEvents;
    P4Atom & c = mWorld[x][y];
    if (c.GetType() == P4Atom::ATOM_EMPTY_TYPE)
      return;
    ++mDispatchableEvents;
    UClassId ucid = c.GetType();
    UClass * uc = theUCL.getUClass(ucid);
    MFM_API_ASSERT_NONNULL(uc);
    BehavePtr bp = uc->getBehavePtrIfAny();
    if (!bp)
      return;
    ++mDispatchedEvents;
    {
      // Load the ew
      SPoint center((s32) x, (s32) y);
      const MDist<4> & md = MDist<4>::get();
      P4Atom inacc(P4Atom::ATOM_INACCESSIBLE_TYPE);
      for (u32 sn = 0u; sn < 41u; ++sn) {
        SPoint ewc = md.GetPoint(sn);
        SPoint tilec = center + ewc;
        if (tilec.GetX() < 0 || tilec.GetY() < 0 ||
            tilec.GetX() >= WORLD_WIDTH || tilec.GetY() >= WORLD_HEIGHT)
          theEW.getAtom(sn) = inacc;
        else
          theEW.getAtom(sn) = mWorld[tilec.GetX()][tilec.GetY()];
      }
    }
    URef ur = theUCL.getStgURef(0u);
    bp(mUContext,ur);
    {
      // UNLOAD THE EW
      SPoint center((s32) x, (s32) y);
      const MDist<4> & md = MDist<4>::get();
      for (u32 sn = 0u; sn < 41u; ++sn) {
        SPoint ewc = md.GetPoint(sn);
        SPoint tilec = center + ewc;
        if (tilec.GetX() < 0 || tilec.GetY() < 0 ||
            tilec.GetX() >= WORLD_WIDTH || tilec.GetY() >= WORLD_HEIGHT)
          continue;
        mWorld[tilec.GetX()][tilec.GetY()] = theEW.getAtom(sn);
      }
    }
    ++mCompletedEvents;
  }

  void SimNoC::dump(Logger & to) {
    ByteSink * bs = to.GetByteSink();
    MFM_API_ASSERT_NONNULL(bs);
    for (u32 y = 0u; y < WORLD_HEIGHT; ++y) {
      to.VreportPrefix(Logger::MESSAGE);
      bs->Printf("%c", (y == WORLD_HEIGHT/2) ? '>' : ' ');
      for (u32 x = 0u; x < WORLD_WIDTH; ++x) {
        UClassId ucid = mWorld[x][y].GetType();
        char ch = '?';          // assume unknown
        switch (ucid) {
        case 2: ch = '.'; break;   // empty
        default: ch = 'X'; break;  // occupied by something
        }
        bs->Printf("%c",ch);
      }
      to.VreportSuffix();
    }
  }
}
