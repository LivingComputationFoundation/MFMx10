#include "Parity2D_3x3.h"
#include "Util.h"  /* For PARITY */

namespace MFM {
  /*
    The bit indices in the 2D 9+7 encoding, [row0..3][col0..3]
   */
  const u8 Parity2D_3x3::indices2D[H + 1][W + 1] = {
    //Col0, Col1, Col2, Col3, Col4
    { 0x8, 0x7, 0x6, 0xb }, // Row 0
    { 0x5, 0x4, 0x3, 0xa }, // Row 1
    { 0x2, 0x1, 0x0, 0x9 }, // Row 2
    { 0xc, 0xd, 0xe, 0xf }, // Row 3
  };

  u32 Parity2D_3x3::ComputeParitySlow(u32 dataBits) {
    u32 res = dataBits;

    // Odd parity on rows excluding last
    for (u32 r = 0; r < H; ++r) {
      u32 bit = 0;
      for (u32 c = 0; c < W; ++c) {
        bit ^= (res >> indices2D[r][c]) & 1;
      }
      if (!bit)
        res |= 1 << indices2D[r][W];
    }
    // Odd parity on cols including last
    for (u32 c = 0; c < W + 1; ++c) {
      u32 bit = 0;
      for (u32 r = 0; r < H; ++r) {
        bit ^= (res >> indices2D[r][c]) & 1;
      }
      if (!bit)
        res |= 1 << indices2D[H][c];
    }

    return res >> DATA_BITS;
  }
}

#ifndef WRITE_PARITY_TABLES
namespace MFM {

  // Helper functions for slow checking and table generation

  static u32 BitAt(u32 bitno) {
    return 1<<bitno;
  }

  u32 Parity2D_3x3::Correct2DParityIfPossible(u32 allBits)
  {
    s32 failed[2];  // [row0], [col1]
    failed[0] = failed[1] = -1;

    // Check all groups for errors, crap out if multiple failures
    // detected
    for (u32 m = 0; m < H + 1 + W + 1; ++m) {  // Check all masks
      u32 rc, idx;
      if (m < H + 1) {
        rc = 0;
        idx = m;
      } else {
        rc = 1;
        idx = m - (H + 1);
      }
      u32 bits = allBits & masks[m];
      if (PARITY(bits)==0) {        // Even parity is a failure
        if (failed[rc] >= 0)        // If so, already failed in direction
          return 0;                 // So we cannot correct this
        failed[rc] = idx;           // Else remember which idx failed
      }
    }

    if (failed[0] < 0 && failed[1] < 0) // Then no failures, you dork,
      return allBits;               // so why did you call me?
    if (failed[0] < 0 || failed[1] < 0) // If apparently just one failure
      return 0;                         // Must be multiple, we're dead

    // Here we have isolated the single failed bit!  Fix it!

    u32 failedIndex = indices2D[failed[0]][failed[1]];
    return allBits^BitAt(failedIndex);

  }

#include "Parity2D_3x3_tables.src"

}
#else /* -DWRITE_PARITY_TABLES */
// TO BUILD Parity2D_3x3_tables.src:
// touch Parity2D_3x3_tables.cpp;g++ -g -I ../include -DWRITE_PARITY_TABLES Parity2D_3x3.cpp -o Parity2D_3x3.elf;./Parity2D_3x3.elf > Parity2D_3x3_tables.src;rm -f Parity2D_3x3.elf
#include <stdio.h>
#include <stdlib.h>

namespace MFM {

  static void WriteMasksTable(FILE * output) {

    fprintf(output,"%s", "  const u32 Parity2D_3x3::masks[H + 1 + W + 1] =\n  {\n");
    for (u32 r = 0; r < Parity2D_3x3::H + 1; ++r) {
      u32 bits = 0;
      for (u32 c = 0; c < Parity2D_3x3::W + 1; ++c) {
        bits |= 1 << Parity2D_3x3::indices2D[r][c];
      }
      fprintf(output,"    0x%04x, // Row %d\n", bits, r);
    }
    for (u32 c = 0; c < Parity2D_3x3::W + 1; ++c) {
      u32 bits = 0;
      for (u32 r = 0; r < Parity2D_3x3::H + 1; ++r) {
        bits |= 1 << Parity2D_3x3::indices2D[r][c];
      }
      fprintf(output,"    0x%04x%s // Col %d\n", bits, c==Parity2D_3x3::W?" ":",", c);
    }
    fprintf(output,"%s", "  };\n\n");
  }
  static void WriteParityTables(FILE * output) {
    fprintf(output,"%s", "  //// GENERATED TABLES: DO NOT EDIT\n  // (See Parity2D_3x3.cpp for generation and use of this file)\n\n");
    WriteMasksTable(output);
    fprintf(output,"%s", "  const u16 Parity2D_3x3::eccTable[TABLE_SIZE] =\n  {");
    for (u32 dataBit = 0; dataBit < Parity2D_3x3::TABLE_SIZE; ++dataBit) {
      if (dataBit%16==0)
        fprintf(output,"\n    ");
      fprintf(output,"0x%02x", Parity2D_3x3::ComputeParitySlow(dataBit));
      if (dataBit!=Parity2D_3x3::TABLE_SIZE-1)
        fprintf(output,", ");
    }
    fprintf(output,"\n  };\n");
    fprintf(output,"%s", "  //// END OF GENERATED TABLES\n");
  }
}

int main() {
  MFM::WriteParityTables(stdout);
  return 0;
}
#endif
