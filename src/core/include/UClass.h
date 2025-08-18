#ifndef UCLASS_H /* -*- C++ -*- */
#define UCLASS_H

#include "itype.h"
#include "URefInfo.h"
#include "Fail.h"

namespace MFM {

  typedef u16 UClassId;

  class UContext;   // FORWARD
  
  typedef void (*BehavePtr)(UContext &ucon, URef ur);

  struct UClassDataMember {
    const char * mName;
    const UClassId mOfUClassId;
    const UClassId mUClassId;
    const u8 mPosition;
    const u8 mSize;
  };

  struct UClass {
    BehavePtr mBehavePtr;
    const u16 * mAllBaseList;
    const char * mName;
    const u16 * mDirectBaseList;
    const UClassDataMember * mAllDataMemberList;
    const URefDescriptor * mAllURefs;
    const u8 * mURefTransitions;
    const char ** mURefNames;

    const UClassId mUClassId;
    const u16 mAllBaseCount;
    const u16 mDirectBaseCount;
    const u16 mAllDataMemberCount;
    const u16 mURefCount;
    const u8 mURefTransitionCount;

    UClass(const UClassId cid,
           const char * name,
           const u16 * directBases,
           const u16 directBaseCount,
           const u16 * allBases, // idx -> uclassid
           const u16 allBaseCount,
           const UClassDataMember * allMembers,
           const u16 allMemberCount,
           const URefDescriptor * allURefs,
           const char ** urefnames,
           const u16 urefcount,
           const u8 * urefTransitions,
           const u8 transcount)
      : mBehavePtr(0)
      , mAllBaseList(allBases)
      , mName(name)
      , mDirectBaseList(directBases)
      , mAllDataMemberList(allMembers)
      , mAllURefs(allURefs)
      , mURefTransitions(urefTransitions)
      , mURefNames(urefnames)

      , mUClassId(cid)
      , mAllBaseCount(allBaseCount)
      , mDirectBaseCount(directBaseCount)
      , mAllDataMemberCount(allMemberCount)
      , mURefCount(urefcount)
      , mURefTransitionCount(transcount)
    { }

    void registerBehave(BehavePtr bp) {
      MFM_API_ASSERT_NONNULL(bp);
      MFM_API_ASSERT_NULL(mBehavePtr);
      mBehavePtr = bp;
    }

    BehavePtr getBehavePtrIfAny() {
      return mBehavePtr;
    }

  };

} // end namespace MFM

#endif /* UCLASS_H */

