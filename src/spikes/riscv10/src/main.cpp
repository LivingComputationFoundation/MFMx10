#include "main.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace MFM {
  FileByteSink STDOUT(stdout);
  FileByteSink STDERR(stderr);
  P4Atom a;
  static const u8 ALEN = BitVector<128>::ARRAY_LENGTH;
  u32 aray[ALEN];

  static const u32 DREG_TYPE = 51u;
  static const u32 RES_TYPE = 52u;
  static const u32 DHS_TYPE = 53u;
  typedef BitField<BitVector<128>,VD::U32,23u,0u> DHS_VAL;

  static EW& ew = theEW;
  static UClassLibrary& ucl = theUCL;

  void littleCode(URef urmc,u32 TIMES) {
    for (u32 i = 0u; i < TIMES; ++i) {
      /*
      u32 v = ucl.readBitsRaw32(urmc);
      v++;
      ucl.writeBitsRaw32(urmc,v);
      */
      theSimNoC.doEvent();
      if (i % 10000u == 0u) {
        LOG.Message("tot %d occ %d dis %d suc %d",
                    theSimNoC.mTotalEvents,
                    theSimNoC.mDispatchableEvents,
                    theSimNoC.mDispatchedEvents,
                    theSimNoC.mCompletedEvents);
        theSimNoC.dump(LOG);
      }
    }
  }
  static void behave_DReg(UContext & ucon, URef ur) {
    EW & ew = *ucon.mEW;
    Random & random = ucon.mRandom;
    u32 sn = random.Between(1u,4u);
    P4Atom & an = ew.getAtom(sn);
    if (an.GetType() == P4Atom::ATOM_EMPTY_TYPE) {
      //LOG.Message("HARO! FROM DReg (%x)",&ew);
      an = ew.getAtom(0u);
    }
  }

  int MainDispatch(int argc, char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    LOG.Message("UClassCount = %d", ucl.getUClassCount());

    LOG.Message("sizeof(P4Atom) = %d", sizeof(P4Atom));
    LOG.Message("sizeof(SimNoC) = %d", sizeof(SimNoC));
    LOG.Message("sizeof(URef) = %d", sizeof(URef));
    LOG.Message("sizeof(BV96) = %d", sizeof(BV96));
    LOG.Message("sizeof(BV128) = %d", sizeof(BitVector<128>));
    LOG.Message("sizeof(BV8K) = %d", sizeof(BV8K));

    LOG.Message("STARTIN GUP");

    URef ur;

    LOG.Message("MADE EW");

    ucl.takeEW(ew);
    LOG.Message("TOOK EW 0x%x",&ucl.getEW());

    LOG.Message("MADE UCL - class count %d",ucl.getUClassCount());

    {
      u32 ucid = ucl.getUClassIdIfAny("DReg");
      P4Atom ee(ucid);
      ew.getAtom(0) = ee;
      UClass * uc = ucl.getCurrentStorageUClass(0u);
      uc->registerBehave(behave_DReg);
      ew.doEvent();

      // fire up the SimNoC
      theSimNoC.reseed(ucid);
    }

    UClass * ucew0 = ucl.getCurrentStorageUClass(0u);
    LOG.Message("MADE CALL");
    LOG.Message("ew[0].uclass = 0x%x",ucew0);
    if (ucew0 != 0)
      LOG.Message(" (of type '%s')",ucew0->mName);

    const char * ename = "EltWithAQFoo";
    u32 ucid = ucl.getUClassIdIfAny(ename);
    LOG.Message("ucid of %s = %d",ename,ucid);
    MFM_API_ASSERT_NONZERO(ucid);

    P4Atom ee(ucid);
    ew.getAtom(2) = ee;
    UClass * ucew1 = ucl.getCurrentStorageUClass(2u);
    LOG.Message("ew[2].uclass = 0x%x",ucew1);
    if (ucew1 != 0)
      LOG.Message(" (of type '%s')",ucew1->mName);
    
    const char * dmname = "mCounter";
    u32 dmidx = ucl.getIndexOfDataMemberNamed(*ucew1, dmname);
    LOG.Message(" %s.%s is at idx %d",ucew1->mName,dmname,dmidx);

    //// HOW DO I GET A UREF TO EltWithAQFoo.mCounter HERE?

    ur = ucl.getStgURef(2u);
    //ur.setURefNumber(92u);

    UClass * ewuc = ucl.getCurrentEWUClass(2);
    
    LOG.Message("FOLLOWIN GUP ew0x%x",ewuc);

    LOG.Message("UR 0x%04x stg#%d, urn#%d",
                ur.getURefRaw(),
                ur.getURefStgId(),
                ur.getURefNumber());

    ucl.reportURef(ur,LOG);

    // ur is currently EltWidthAQFoo*. To get to mCounter, our first
    // step is get to EltWidthAQFoo!, which is the 0th ur selection
    URef urbang = ucl.selectURef(ur,0u);
    ucl.reportURef(urbang,LOG);

    // Then we want the 1th selection off urbang to get to mCounter
    URef urmc = ucl.selectURef(urbang,1u);
    ucl.reportURef(urmc,LOG);

    // Let's try ot read mCounter
    u32 mval = ucl.readBitsRaw32(urmc);
    LOG.Message("GOTS %d",mval);

    // Let's increment it
    ++mval;
    ucl.writeBitsRaw32(urmc,mval);
    mval = ucl.readBitsRaw32(urmc);
    LOG.Message("PUTS %d",mval);

    // Let's do that a lot
    const u32 TIMES = 1000000u;
    littleCode(urmc,TIMES);
    LOG.Message("AFTER %d",TIMES);

    P4Atom dhs(DHS_TYPE);
    u32 dhsv = DHS_VAL::Read(dhs);
    LOG.Message("Orig 0x%x",dhsv);
    DHS_VAL::Write(dhs,0x224455);
    dhsv = DHS_VAL::Read(dhs);
    LOG.Message("Nows 0x%x",dhsv);
    
    {
      AtomSerializer<P4AtomConfig> as(dhs);
      LOG.Message(">dhs: %@",&as);
    }

    // // GENERATE A FOGGEN REFERENCE TO GETSTACKBOUND
    // LOG.Message("The stack bound is 0x%x.  Fascinating.\n",getStackBound());
    
    LOG.Message("wham10");
    typedef BitVector<128> MyBV;
    MyBV bv;
    for (u32 i = 0; i < MyBV::BITS/8; ++i) {
      bv.Write(i*8,8,i);
    }
    LOG.Message("wham11");
    bv.ToArray(aray);
    LOG.Message("wham12");
    for (u32 i = 0; i < ALEN; ++i) {
      LOG.Message("POCKO %d 0x%08x JOCKO",i,aray[i]);
    }
    
    bv.SetBit(3);
    u8 vv;
    vv = (u8) bv.Read(0,8);
    LOG.Message("BAHAHAHAHARO %d.",vv);
    bv.SetBit(4);
    vv = (u8) bv.Read(0,8);
    LOG.Message("BAHAHAHAHARO %d.",vv);
    bv.SetBit(5);
    vv = (u8) bv.Read(0,8);
    LOG.Message("BAHAHAHAHARO %d.",vv);
    bv.ClearBit(4);
    vv = (u8) bv.Read(0,8);
    LOG.Message("BAHAHAHAHARO %d.",vv);
    P4Atom dreg(DREG_TYPE);
    P4Atom res(RES_TYPE);
    P4Atom empty;
    LOG.Message("DREG %d",dreg.GetType());
    LOG.Message("RES %d",res.GetType());
    LOG.Message("EMP %d",empty.GetType());
    if (true) {
      a.SetStateField(8u,8u,17u);
      vv = (u8) a.GetStateField(8u,8u);
      a.SetStateField(8u,8u,27u);
      vv = (u8) a.GetStateField(8u,8u);
      for (u32 i = 0u; i < 28u; ++i) {
        a.SetStateField(i*4,4,i&0xf);
      }
      theEW.mSites[0].mSiteAtom = a;
      AtomSerializer<P4AtomConfig> as(theEW.mSites[0].mSiteAtom);
      LOG.Message(">ew0: %@",&as);
      vv = 0u;
    }
    return vv;
  }
}

int main(int argc, char** argv)
{
  unwind_protect({
      /*
      const char * file = (const char *) unwindProtect_errorEnvironment.file;
      int line = unwindProtect_errorEnvironment.lineno;
      int code = unwindProtect_errorEnvironment.thrown;
      const char * msg = MFMFailCodeReason(code);

      MFMPrintError(stderr,file,line,code);
      fprintf(stderr,"Failed out of main: %s\n",msg);
      exit(99);
      */
      return 98;
  },{
    return MFM::MainDispatch(argc,argv);
  });
}
