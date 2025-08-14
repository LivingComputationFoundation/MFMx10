#ifndef UCLASS_H /* -*- C++ -*- */
#define UCLASS_H

#include "itype.h"
#include "URefInfo.h"

namespace MFM {

  struct UClassDataMember {
    const char * mName;
    const u16 mOfUClassId;
    const u16 mUClassId;
    const u8 mPosition;
    const u8 mSize;
  };
  struct UClass {
    const u16 mUClassId;
    const char * mName;
    const u16 * mDirectBaseList;
    const u16 mDirectBaseCount;
    const u16 * mAllBaseList;
    const u16 mAllBaseCount;
    const UClassDataMember * mAllDataMemberList;
    const u16 mAllDataMemberCount;
    const URefDescriptor * mAllURefs;
    const u16 mURefCount;
    UClass(const u16 cid,
           const char * name,
           const u16 * directBases,
           const u16 directBaseCount,
           const u16 * allBases, // idx -> uclassid
           const u16 allBaseCount,
           const UClassDataMember * allMembers,
           const u16 allMemberCount,
           const URefDescriptor * allURefs,
           const u16 urefcount)
      : mUClassId(cid)
      , mName(name)
      , mDirectBaseList(directBases)
      , mDirectBaseCount(directBaseCount)
      , mAllBaseList(allBases)
      , mAllBaseCount(allBaseCount)
      , mAllDataMemberList(allMembers)
      , mAllDataMemberCount(allMemberCount)
      , mAllURefs(allURefs)
      , mURefCount(urefcount)
    { }

  };

#if 0 // SEE URefInfo INSTEAD
  struct UCMemberInfo {
    u16 mMemberPosition;        // bit position in stg
    u16 mMemberSize;            // bit size in stg
    u16 mMemberClassId;         // concrete classid (effself)
  };
#endif

} // end namespace MFM

#endif /* UCLASS_H */

