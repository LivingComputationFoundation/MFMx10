/* -*- mode:C++ -*- */

/*
  Parity2D_3x3.h Support for simple 2D parity management
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
   \file Parity2D_3x3.h Support for two-dimensional parity calculations
   \author David H. Ackley.
   \date (C) 2014 All rights reserved.
   \lgpl
*/
#ifndef PARITY2D_3x3_H
#define PARITY2D_3x3_H

#include "itype.h"

namespace MFM {

  /**
     Parity2D_3x3 is an all-static class for manipulating (one
     particular instance of) two-dimensional parity error detection
     and correction.  This class handles error detection and
     correction \em only of nine bit quantities being protected
     with seven additional bits.  In addition, this class assumes the
     seven ECC bits will be placed to the left of the nine data bits
     when they are joined into a 16-bit quantity.

     Redundancy at the level of seven bits protecting nine bits
     ensures that all single bit errors are correctable and all double
     bit errors are detectable.  Most all triple bit errors are
     detectable as well.

     This class uses a table lookup to accelerate the generation and
     checking of the parity bits.  In particular, \ref Compute2DParity
     and \ref Check2DParity are both likely inlined and fast enough to
     use with impunity virtually anywhere.  In addition, \ref
     CheckAndCorrect2DParity and \ref Remove2DParityFrom are fast \em
     if no errors are detected, but note only moderate effort has been
     made to accelerate \ref Correct2DParityIfPossible, which is the
     routine that actually performs the error correction to the degree
     possible.
   */
  class Parity2D_3x3
  {
  public:
    enum {
      W = 3,
      H = 3,
      DATA_BITS = W * H,
      ECC_BITS = W + H + 1,
      TABLE_SIZE = 1 << DATA_BITS,
      INDEX_MASK = TABLE_SIZE - 1,
      ECC_MASK = (1 << ECC_BITS) - 1
    };

    /**
       Compute the seven bit quantity representing the 2D parity of
       the bottom nine bits of \a dataBits.  Assume 0..8 represent the
       bottom nine bits of \a dataBits from least significant (0) to
       most significant (8) bit, and assume 9..f represent the bottom
       seven bits of the computed 2D parity, from least significant
       (9) to most significant (f) bit.  Then given this layout:

       <pre>
             8  7  6  b
             5  4  3  a
             2  1  0  9
             c  d  e  f
       </pre>

       this function returns the seven bits (corresponding to 9..f)
       such that each 4-element row and 4-element column has odd
       parity, and the overall group of 16 bits has even parity (but
       is never all zero).

       \sa Add2DParity

     */
    static u32 Compute2DParity(u32 dataBits) {
      return eccTable[dataBits&INDEX_MASK];
    }

    static u32 ComputeParitySlow(u32 dataBits);

    /**
       Return the 16 bit quantity formed by prefixing the seven bits
       of 2D parity to the nine bits of \a dataBits.

       \sa Compute2DParity
     */
    static u32 Add2DParity(u32 dataBits) {
      return (dataBits&INDEX_MASK) | (Compute2DParity(dataBits) << DATA_BITS);
    }

    /**
       Remove the seven parity bits from \a allBits if possible,
       storing the (possibly error-corrected) resulting nine bits in
       \a dataBits.

       \returns \b true if either the parity was correct or an error
       was detected and successfully corrected.  In either case \a
       dataBits will have been updated to the extracted data value.
       Otherwise returns \b false and \a dataBits remains untouched.

     */
    static bool Remove2DParity(u32 allBits, u32 & dataBits) {
      u32 corrected = CheckAndCorrect2DParity(allBits);
      if (corrected == 0)
        return false;
      dataBits = corrected&INDEX_MASK;
      return true;
    }

    /**
       Check the 2D parity of \a allBits, assuming it is laid out in
       bit number order from its most significant bit on the left to
       its least significant bit on the right, using the data bit and
       parity bit definitions given in the documentation for \ref
       Compute2DParity.

       \return true if allBits is a correctly formatted ECC+DATA
       value, with no detectable errors, or false otherwise

       \sa CheckAndCorrect2DParity
    */

    static bool Check2DParity(const u32 allBits) {
      u32 eccBits = (allBits>>DATA_BITS) & ECC_MASK;
      u32 dataBits = allBits&INDEX_MASK;
      return Compute2DParity(dataBits) == eccBits;
    }

    /**
       Check the 2D parity of \a allBits, assuming it is laid out in
       bit number order from its most significant bit on the left to
       its least significant bit on the right, using the data bit and
       parity bit definitions given in the documentation for \ref
       Compute2DParity.

       The value returned is equal to \a allBits if the parity checks,
       is 0 if the parity does not check but \a allBits is so damaged
       it cannot be repaired.  Otherwise the value returned is
       non-zero and not equal to \a allBits, and it represents the
       error-corrected value of \a allBits.

       \sa Compute2DParity

     */
    static u32 CheckAndCorrect2DParity(const u32 allBits) {
      u32 eccBits = (allBits>>DATA_BITS) & ECC_MASK;
      u32 dataBits = allBits&INDEX_MASK;
      if (Compute2DParity(dataBits) == eccBits)
        return allBits;
      return Correct2DParityIfPossible(allBits);
    }

    /**
       Attempt to error-correct \a allBits.  \b Note: Usually callers
       will prefer to call \ref CheckAndCorrect2DParity, because it's
       fast \em if the parity is correct, while this method is slow in
       all cases.

       The value returned is equal to \a allBits if the parity checks,
       is 0 if the parity does not check but \a allBits is so damaged
       it cannot be repaired.  Otherwise the value returned is
       non-zero and not equal to \a allBits, and it represents the
       error-corrected value of \a allBits.

       \sa CheckAndCorrect2DParity

     */
    static u32 Correct2DParityIfPossible(u32 allBits);

    static const u8 indices2D[H + 1][W + 1];

  private:
    static const u16 eccTable[TABLE_SIZE];
    static const u32 masks[H + 1 + W + 1];
  };
} /* namespace MFM */

#endif /*PARITY2D_3x3_H*/
