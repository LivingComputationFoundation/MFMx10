/*                                              -*- mode:C++ -*-
  P4Atom.h Atom with built in error correcting
  Copyright (C) 2025 The Living Computation Foundation.  All rights reserved.
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file P4Atom.h Atom with built in error correcting
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef P4ATOM_H
#define P4ATOM_H

#include <stdio.h>
#include "itype.h"
#include "Point.h"
#include "BitField.h"
#include "Atom.h"
#include "Element.h"
#include "AtomConfig.h"
#include "Util.h"      /* For COMPILATION_REQUIREMENT */
#include "Parity2D_3x3.h"
//#include "UlamClassRegistry.h"

namespace MFM {

  class P4Atom; // FORWARD

  struct P4AtomConfig : public AtomConfig {
    typedef P4Atom ATOM_TYPE;
    enum { ATOM_CATEGORY = 4 };
    enum { BITS_PER_ATOM = 128 };
    enum { ATOM_TYPE_BITS = 9 };
    enum { ATOM_UNDEFINED_TYPE = 0x000 };
    enum { ATOM_EMPTY_TYPE = 0x002 }; // (UrSelf is uclassid #1)
    enum { ATOM_FIRST_STATE_BIT = 16 };
    enum { ATOM_LAST_STATE_BIT = BITS_PER_ATOM - 1 };
  };

  class P4Atom : public Atom< P4AtomConfig >
  {
    typedef P4AtomConfig AC;

  public:
    enum { ATOM_EMPTY_TYPE = AC::ATOM_EMPTY_TYPE };
    enum { ATOM_UNDEFINED_TYPE = AC::ATOM_UNDEFINED_TYPE };

    enum {
      BITS = AC::BITS_PER_ATOM,

      //////
      // P4 header configuration: Header is in low-bits end of the bitvector

      P4_ECC_BITS_POS = 0,
      P4_ECC_BITS_LEN = 7,

      P4_TYPE_BITS_POS = P4_ECC_BITS_POS + P4_ECC_BITS_LEN,
      P4_TYPE_BITS_LEN = 9,

      P4_FIXED_HEADER_POS = P4_ECC_BITS_POS,
      P4_FIXED_HEADER_LEN = P4_ECC_BITS_LEN + P4_TYPE_BITS_LEN,

      P4_STATE_BITS_POS = P4_FIXED_HEADER_POS + P4_FIXED_HEADER_LEN,
      P4_STATE_BITS_LEN = BITS - P4_STATE_BITS_POS,

      //////
      // Declarations required by the Atom contract
      ATOM_FIRST_STATE_BIT = P4_STATE_BITS_POS,

      //////
      // Other constants
      P4_TYPE_COUNT = 1<<P4_TYPE_BITS_LEN

    };

    typedef BitField<BitVector<BITS>,VD::U32,P4_FIXED_HEADER_LEN,P4_FIXED_HEADER_POS> AFFixedHeader;
    typedef BitField<BitVector<BITS>,VD::U32,P4_TYPE_BITS_LEN,P4_TYPE_BITS_POS> AFTypeBits;
    typedef BitField<BitVector<BITS>,VD::U32,P4_ECC_BITS_LEN,P4_ECC_BITS_POS> AFECCBits;

  protected:

    /* We really don't want to allow the public to change the type of a
       P4Atom, since the type doesn't mean much without the atomic
       header as well */

    void SetType(u32 type) {

      MFM_API_ASSERT_ARG(type < P4_TYPE_COUNT);

      // Generate ECC and store all in header
      AFFixedHeader::Write(this->m_bits,Parity2D_3x3::Add2DParity(type));
    }

  public:

    explicit P4Atom(u32 type = ATOM_EMPTY_TYPE, u32 z1 = 0, u32 z2 = 0, u32 stateBits = 0)
    {
      COMPILATION_REQUIREMENT< 32 <= BITS-1 >();

      MFM_API_ASSERT_ARG(z1 == 0 && z2 == 0);
      MFM_API_ASSERT(stateBits <= P4_STATE_BITS_LEN, OUT_OF_ROOM);

      SetType(type);
    }

    P4Atom(const P4Atom& p4atomref)
    {
      this->m_bits = p4atomref.m_bits; // explicitly for c++11, since op= is explicit.
    }

    u32 GetTypeImpl() const {
      return AFTypeBits::Read(this->m_bits);
    }

    bool IsSaneImpl() const
    {
      u32 fixedHeader = AFFixedHeader::Read(this->m_bits);
      return Parity2D_3x3::Check2DParity(fixedHeader);
    }

    void SetEmptyImpl()
    {
      SetType(ATOM_EMPTY_TYPE);
    }

    void SetUndefinedImpl()
    {
      SetType(ATOM_UNDEFINED_TYPE);
    }

    bool HasBeenRepairedImpl()
    {
      u32 fixedHeader = AFFixedHeader::Read(this->m_bits);
      u32 repairedHeader =
        Parity2D_3x3::Correct2DParityIfPossible(fixedHeader);

      if (repairedHeader == 0) return false;

      if (fixedHeader != repairedHeader)
      {
        AFFixedHeader::Write(this->m_bits, repairedHeader);
      }

      return true;
    }

    u32 GetMaxStateSize(u32 type) const {
      return P4_STATE_BITS_LEN;
    }

    /**
     * Index of first bit that isn't a state bit.
     */
    u32 EndStateBit() const
    {
      return BITS;
    }

    void WriteStateBitsImpl(ByteSink& ostream) const
    {
      for(u32 i = P4_STATE_BITS_POS; i < P4_STATE_BITS_POS + P4_STATE_BITS_LEN; i++)
      {
	ostream.Printf("%d", this->m_bits.ReadBit(i) ? 1 : 0);
      }
    }

    void ReadStateBitsImpl(const char* stateStr)
    {
      for(u32 i = 0; i < P4_STATE_BITS_LEN; i++)
      {
	this->m_bits.WriteBit(P4_STATE_BITS_POS + i, stateStr[i] == '0' ? 0 : 1);
      }
    }

    void ReadStateBitsImpl(const BitVector<BITS> & bv)
    {
      for(u32 i = 0; i < P4_STATE_BITS_LEN; i++)
      {
        u32 idx = P4_STATE_BITS_POS + i;
	this->m_bits.WriteBit(idx, bv.ReadBit(idx));
      }
    }

    /**
     * Read stateWidth state bits starting at stateIndex, which counts
     * toward the right with 0 meaning the leftmost state bit.
     */
    u32 GetStateField(u32 stateIndex, u32 stateWidth) const
    {
      MFM_API_ASSERT_ARG(stateWidth <= P4_STATE_BITS_LEN);
      return this->m_bits.Read(P4_STATE_BITS_POS + stateIndex, stateWidth);
    }

    /**
     * Store value into stateWidth state bits starting at stateIndex,
     * which counts toward the right with 0 meaning the leftmost state
     * bit.
     */
    void SetStateField(u32 stateIndex, u32 stateWidth, u32 value)
    {
      MFM_API_ASSERT_ARG(stateWidth <= P4_STATE_BITS_LEN);
      return this->m_bits.Write(P4_STATE_BITS_POS + stateIndex, stateWidth, value);
    }

    void PrintBits(ByteSink & ostream) const
    { this->m_bits.Print(ostream); }

    void PrintImpl(ByteSink & ostream) const
    {
      u32 type = this->GetType();
      ostream.Printf("P4[%x/",type);
      u32 length = GetMaxStateSize(type);
      for (u32 i = 0; i < length; i += 4) {
        u32 nyb = this->GetStateField(i,4);
        ostream.Printf("%x",nyb);
      }
      ostream.Printf("]");
    }

    P4Atom& operator=(const P4Atom & rhs)
    {
      if (this == &rhs) return *this;

      this->m_bits = rhs.m_bits;

      return *this;
    }

  };
} /* namespace MFM */

#endif /*P4ATOM_H*/
