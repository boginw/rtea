#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "definitions.h"
#include "ID3.h"
#include "util.h"
#include "LinkedList.h"
#ifndef RTEA_TABLES_H
#include "tables.h"
#endif //RTEA_TABLES_H

FrameHeader parseHeader(FILE *fp);
void jumpToNextFrame(FILE *fp, Frame *curFrame);
FrameSideInfo parseSideInfo(FILE *fp, FrameHeader fh);
Granule parseGranuleSideInfo(bool isMono, const unsigned char *buffer, size_t *cBit);
void parseMainData(FILE *fp, Frame *fr);
Frame nextFrame(FILE *fp);
void checkSupported(Frame fr);
int frameSize(Frame *frame);
unsigned char *readMainDataBytes(FILE *fp, long start, size_t size, size_t offset, long skipSize);
unsigned int calcPart2Length(Granule *g, int ch);
void sharedScaleFac(unsigned char *buffer, int i, int gr, int ch, Frame *fr, size_t *cBit);
void readHuffman(unsigned char *buffer, size_t *cBit, Frame *fr, int gr, int ch);
int huffmanDecode(unsigned char *buffer, size_t *cBit, Frame *fr, unsigned int tableNum,
                  int32_t *x, int32_t *y, int32_t *v, int32_t *w);

int debug;

// TODO: move this
hufftables g_huffman_main[34] = {
    {NULL, 0, 0},  /* Table  0 */
    {g_huffman_table_1, 7, 0},  /* Table  1 */
    {g_huffman_table_2, 17, 0},  /* Table  2 */
    {g_huffman_table_3, 17, 0},  /* Table  3 */
    {NULL, 0, 0},  /* Table  4 */
    {g_huffman_table_5, 31, 0},  /* Table  5 */
    {g_huffman_table_6, 31, 0},  /* Table  6 */
    {g_huffman_table_7, 71, 0},  /* Table  7 */
    {g_huffman_table_8, 71, 0},  /* Table  8 */
    {g_huffman_table_9, 71, 0},  /* Table  9 */
    {g_huffman_table_10, 127, 0},  /* Table 10 */
    {g_huffman_table_11, 127, 0},  /* Table 11 */
    {g_huffman_table_12, 127, 0},  /* Table 12 */
    {g_huffman_table_13, 511, 0},  /* Table 13 */
    {NULL, 0, 0},  /* Table 14 */
    {g_huffman_table_15, 511, 0},  /* Table 15 */
    {g_huffman_table_16, 511, 1},  /* Table 16 */
    {g_huffman_table_16, 511, 2},  /* Table 17 */
    {g_huffman_table_16, 511, 3},  /* Table 18 */
    {g_huffman_table_16, 511, 4},  /* Table 19 */
    {g_huffman_table_16, 511, 6},  /* Table 20 */
    {g_huffman_table_16, 511, 8},  /* Table 21 */
    {g_huffman_table_16, 511, 10},  /* Table 22 */
    {g_huffman_table_16, 511, 13},  /* Table 23 */
    {g_huffman_table_24, 512, 4},  /* Table 24 */
    {g_huffman_table_24, 512, 5},  /* Table 25 */
    {g_huffman_table_24, 512, 6},  /* Table 26 */
    {g_huffman_table_24, 512, 7},  /* Table 27 */
    {g_huffman_table_24, 512, 8},  /* Table 28 */
    {g_huffman_table_24, 512, 9},  /* Table 29 */
    {g_huffman_table_24, 512, 11},  /* Table 30 */
    {g_huffman_table_24, 512, 13},  /* Table 31 */
    {g_huffman_table_32, 31, 0},  /* Table 32 (not seen in the standard) */
    {g_huffman_table_33, 31, 0},  /* Table 33 (not seen in the standard) */
};

int main(int argc, char *argv[]) {
  char *filename;
  long fileSize;
  FILE *framePointer;
  Frame fr;
  Frame lr;
  filename = getArgs(argc, argv);
  LinkedList *frameList = linkedList();

  if (fileExists(filename)) {
    framePointer = fopen(filename, "r");
    fseek(framePointer, 0L, SEEK_END);
    fileSize = ftell(framePointer);
    rewind(framePointer);
  } else {
    fprintf(stderr, "File does not exist\n");
    exit(1);
  }

  while (ftell(framePointer) < fileSize) {
    if (isID3(framePointer)) {
      parseID3Header(framePointer);
    } else {
      fr = nextFrame(framePointer);
      frameList->add(frameList, fr);
      lr = fr;
    }
  }

  printf("%d frames found\n", frameList->count(frameList));

  if (debug && frameList->count(frameList) > 0) {
    printHeader(frameList->get(frameList, 0).header);
  }
  fclose(framePointer);
  freeList(frameList);
  return 0;
}

char* decodeLayer3(Frame frame) {

  int granule, channel;
  int channels = frame.header.mode == SINGLE ? 1 : 2;

  for (granule = 0; granule < 2; granule++) {
    for (channel = 0; channel < channels; ++channel) {
      exit(1); // TODO: WIP
    }
  }

  return malloc(2);
}

/**
 * Gets the next frame given a file pointer
 * @param fp File pointer
 * @return The next frame
 */
Frame nextFrame(FILE *fp) {
  Frame fr;

  // store starting point in file for later
  fr.start = ftell(fp);
  fr.header = parseHeader(fp);

  // Check if we support the header
  checkSupported(fr);

  fr.sideInfo = parseSideInfo(fp, fr.header);
  parseMainData(fp, &fr);

  // Jump so we can read next frame
  jumpToNextFrame(fp, &fr);

  return fr;
}

void parseMainData(FILE *fp, Frame *fr) {
  int nch = fr->header.mode == SINGLE ? 1 : 2;
  int totalSize = frameSize(fr);
  int bitsize;
  unsigned char *mdb; // Main data bytes
  int skipSize = 4 + (nch == 1 ? 17 : 32); // header & side info
  size_t cBit = 0;
  // if crc protection, add that to the skip size
  if (fr->header.protection == 0) {
    skipSize += 2;
  }

  totalSize -= skipSize;
  fr->mainData.length = (size_t) totalSize;

  mdb = readMainDataBytes(
      fp, fr->start, (totalSize + fr->sideInfo.main_data_begin),
      (size_t) fr->sideInfo.main_data_begin, skipSize
  );

  // Foreach granule
  for (int gr = 0; gr < 2; gr++) {
    // Foreach channel
    for (int ch = 0; ch < nch; ch++) {
      Granule g = fr->sideInfo.granule[gr];
      if (g.windowSwitchingFlag[ch] == 1 && g.blockType[ch] == 2) {
        if (g.mixedBlockflag[ch]) {
          for (int sfb = 0; sfb < 8; sfb++) {
            fr->mainData.scalefac_l[gr][ch][sfb] =
                nextBits(mdb, &cBit, g.slen[ch][0]);
          }

          for (int sfb = 3; sfb < 12; sfb++) {
            // We choose slen1 for bands between 3 and 5 and slen2 for the rest
            bitsize = (sfb < 6) ? g.slen[ch][0] : g.slen[ch][1];

            for (int window = 0; window < 3; window++) {
              fr->mainData.scalefac_s[gr][ch][sfb][window] =
                  nextBits(mdb, &cBit, bitsize);
            }
          }
        } else {
          for (int sfb = 0; sfb < 12; sfb++) {
            // We choose slen1 for bands between 3 and 5 and slen2 for the rest
            bitsize = (sfb < 6) ? g.slen[ch][0] : g.slen[ch][1];

            for (int window = 0; window < 3; window++) {
              fr->mainData.scalefac_s[gr][ch][sfb][window] =
                  nextBits(mdb, &cBit, bitsize);
            }
          }
        }
      } else { /* blockType == 0 if windowSwitchingFlag == 0 */
        for (int i = 0; i < 4; ++i) {
          sharedScaleFac(mdb, i, gr, ch, fr, &cBit);
        }
      }
      readHuffman(mdb, &cBit, fr, gr, ch);
    }
  }

  free(mdb);
}

// TODO: clean this mess
/**
 * Basically https://github.com/technosaurus/PDMP3/blob/master/pdmp3.c
 * @param buffer
 * @param cBit
 * @param fr
 * @param gr
 * @param ch
 */
void readHuffman(unsigned char *buffer, size_t *cBit, Frame *fr, int gr, int ch) {
  int32_t x, y, v, w;
  unsigned int sfreq, isPos, region1Start,
      region2Start, bitPosEnd, l, tableNum;
  Granule *g = &(fr->sideInfo.granule[gr]);

  if (g->part2_3_length[ch] == 0) {
    for (isPos = 0; isPos < 576; isPos++) {
      fr->mainData.is[gr][ch][isPos] = 0.0f;
    }
    return;
  }

  // Calculate bitPosEnd which is the index of the last bit for this part.
  bitPosEnd = (unsigned int) ((*cBit) + g->part2_3_length[ch] - 1);
  // Determine region boundaries

  if ((fr->sideInfo.granule[gr].windowSwitchingFlag[ch] == 1) &&
      (fr->sideInfo.granule[gr].blockType[ch] == 2)) {
    region1Start = 36;  // sfb[9/3]*3=36
    region2Start = 576; // No Region2 for short block case.
  } else {
    sfreq = (unsigned int) fr->header.frequency;
    region1Start = g_sf_band_indices[sfreq]
        .l[fr->sideInfo.granule[gr].region0Count[ch] + 1];
    region2Start = g_sf_band_indices[sfreq]
        .l[fr->sideInfo.granule[gr].region0Count[ch] + 2];
  }

  /* Read big_values using tables according to region_x_start */
  for (isPos = 0; isPos < fr->sideInfo.granule[gr].bigValues[ch] * 2; isPos++) {
    if (isPos < region1Start) {
      tableNum = fr->sideInfo.granule[gr].tableSelect[ch][0];
    } else if (isPos < region2Start) {
      tableNum = fr->sideInfo.granule[gr].tableSelect[ch][1];
    } else {
      tableNum = fr->sideInfo.granule[gr].tableSelect[ch][2];
    }
    /* Get next Huffman coded words */
    huffmanDecode(buffer, cBit, fr, tableNum, &x, &y, &v, &w);
    fr->mainData.is[gr][ch][isPos++] = x;
    fr->mainData.is[gr][ch][isPos] = y;
  }

  /* Read small values until is_pos = 576 or we run out of huffman data */
  tableNum = fr->sideInfo.granule[gr].count1TableSelect[ch] + 32;
  for (isPos = fr->sideInfo.granule[gr].bigValues[ch] * 2;
       (isPos <= 572) && (*cBit <= bitPosEnd); isPos++) {
    /* Get next Huffman coded words */
    huffmanDecode(buffer, cBit, fr, tableNum, &x, &y, &v, &w);

    fr->mainData.is[gr][ch][isPos++] = v;
    if (isPos >= 576) {
      break;
    }

    fr->mainData.is[gr][ch][isPos++] = w;
    if (isPos >= 576) {
      break;
    }

    fr->mainData.is[gr][ch][isPos++] = x;
    if (isPos >= 576) {
      break;
    }

    fr->mainData.is[gr][ch][isPos] = y;
  }

  // Remove last words read
  if (*cBit > (bitPosEnd + 1)) {
    isPos -= 4;
  }

  fr->sideInfo.granule[gr].count1[ch] = isPos;

  // Zero out the last part if necessary
  for (/* is_pos comes from last for-loop */; isPos < 576; isPos++) {
    fr->mainData.is[gr][ch][isPos] = 0.0f;
  }
}

// TODO: clean this mess
int huffmanDecode(unsigned char *buffer,
                  size_t *cBit,
                  Frame *fr,
                  unsigned int tableNum,
                  int32_t *x,
                  int32_t *y,
                  int32_t *v,
                  int32_t *w) {
  unsigned int point = 0, error = 1, bitsleft = 32, //=16??
  treelen = g_huffman_main[tableNum].treelen,
      linbits = g_huffman_main[tableNum].linbits;

  if (treelen == 0) { /* Check for empty tables */
    *x = *y = *v = *w = 0;
    return 1;
  }
  const unsigned short *htptr = g_huffman_main[tableNum].hufftable;
  do {   /* Start reading the Huffman code word,bit by bit */
    /* Check if we've matched a code word */
    if ((htptr[point] & 0xff00) == 0) {
      error = 0;
      *x = (htptr[point] >> 4) & 0xf;
      *y = htptr[point] & 0xf;
      break;
    }
    if (nextBits((const unsigned char *) buffer, cBit, 1)) { /* Go right in tree */
      while ((htptr[point] & 0xff) >= 250) {
        point += htptr[point] & 0xff;
      }
      point += htptr[point] & 0xff;
    } else { /* Go left in tree */
      while ((htptr[point] >> 8) >= 250) {
        point += htptr[point] >> 8;
      }
      point += htptr[point] >> 8;
    }
  } while ((--bitsleft > 0) && (point < treelen));
  if (tableNum > 31) {  /* Process sign encodings for quadruples tables. */
    *v = (*y >> 3) & 1;
    *w = (*y >> 2) & 1;
    *x = (*y >> 1) & 1;
    *y = *y & 1;

    if ((*v > 0) && (nextBits((const unsigned char *) buffer, cBit, 1) == 1)) {
      *v = -*v;
    }
    if ((*w > 0) && (nextBits((const unsigned char *) buffer, cBit, 1) == 1)) {
      *w = -*w;
    }
    if ((*x > 0) && (nextBits((const unsigned char *) buffer, cBit, 1) == 1)) {
      *x = -*x;
    }
    if ((*y > 0) && (nextBits((const unsigned char *) buffer, cBit, 1) == 1)) {
      *y = -*y;
    }
  } else {

    if ((linbits > 0) && (*x == 15)) {
      *x += nextBits((const unsigned char *) buffer, cBit, linbits);
    }// Get linbits
    if ((*x > 0) && (nextBits((const unsigned char *) buffer, cBit, 1) == 1)) {
      *x = -*x; // Get sign bit
    }
    if ((linbits > 0) && (*y == 15)) {
      *y += nextBits((const unsigned char *) buffer, cBit, linbits);
    }// Get linbits
    if ((*y > 0) && (nextBits((const unsigned char *) buffer, cBit, 1) == 1)) {
      *y = -*y;// Get sign bit
    }
  }
  return 0;
}

/**
 * Find scale factor bands for given group
 * @param buffer Main data bytes
 * @param i Index of the ScaleFactorGroupTable
 * @param gr Granule index
 * @param ch Channel index
 * @param fr Current frame
 * @param cBit Current bit
 */
void sharedScaleFac(unsigned char *buffer, int i, int gr, int ch, Frame *fr, size_t *cBit) {
  int sfb,
      from = ScaleFactorGroupTable[i][0],
      to = ScaleFactorGroupTable[i][1],
      si = (i < 2) ? 0 : 1;

  if (!fr->sideInfo.scfsi[ch][i] || gr == 0) {
    for (sfb = from; sfb < to; sfb++) {
      fr->mainData.scalefac_l[gr][ch][sfb] =
          nextBits(buffer, cBit, fr->sideInfo.granule[gr].slen[ch][si]);
    }
  } else if (fr->sideInfo.scfsi[ch][i] == 1 && gr == 1) {
    for (sfb = from; sfb < to; sfb++) {
      fr->mainData.scalefac_l[1][ch][sfb] = fr->mainData.scalefac_l[0][ch][sfb];
    }
  }
}

unsigned char *readMainDataBytes(FILE *fp, long start, size_t size, size_t offset, long skipSize) {
  unsigned char *mainDataBytes;
  int bytesLeft = (int) size;

  // Our array is set to the size, if there isn't space
  // for all elements, then ignore the frame
  if (size < offset) {
    fprintf(stderr, "Illegal size (pos %x)\n", (unsigned int) start);
    return NULL;
  }

  // This should not be allowed
  if (size > 1500) {
    fprintf(stderr, "Main data size is crazy large (size: %d)\n", (int) size);
  }

  // Allocate for main data
  mainDataBytes = malloc(size * sizeof(char));

  // If a negative offset is set read bytes from prev frame
  if (offset != 0) {
    fseek(fp, (long) (start - offset), SEEK_SET);
    fread(mainDataBytes, sizeof(char), (size_t) offset, fp);
    bytesLeft -= offset;
  }

  // Read remaining bytes from this frame
  fseek(fp, (start + skipSize), SEEK_SET);
  fread(&(mainDataBytes[size - bytesLeft]),
        sizeof(char), (size_t) bytesLeft, fp);

  return mainDataBytes;
}

/**
 * Parses the sideinfo of frame
 * @param fp File pointer
 * @param fh Current frame header
 * @return FrameSideInfo of frame
 */
FrameSideInfo parseSideInfo(FILE *fp, FrameHeader fh) {
  FrameSideInfo result;
  bool isMono = fh.mode == SINGLE;
  size_t size = isMono ? 17 : 32;
  int nch = (!isMono + 1);
  unsigned char *buffer = calloc(sizeof(char), size);
  fread(buffer, 1, size, fp);
  size_t cBit = 0;

  result.main_data_begin = nextBits(buffer, &cBit, 9);

  result.private = (char) nextBits(buffer, &cBit, (isMono ? 5 : 3));

  for (int ch = 0; ch < nch; ch++) {
    for (int i = 0; i < 4; i++) {
      result.scfsi[ch][i] = (bool) nextBits(buffer, &cBit, 1);
    }
  }

  result.granule[0] = parseGranuleSideInfo(isMono, buffer, &cBit);
  result.granule[1] = parseGranuleSideInfo(isMono, buffer, &cBit);

  free(buffer);
  return result;
}

/**
 * Parses sideinfo from frame
 * @param isMono Whether if the frame is mon
 * @param buffer Buffer containing sideinfo bytes
 * @param cBit The current bit index
 * @return Granule struct
 */
Granule parseGranuleSideInfo(bool isMono, const unsigned char *buffer, size_t *cBit) {
  Granule result = {};
  int nch = isMono ? 1 : 2;
  int sfc;
  // Once per channel
  for (int ch = 0; ch < nch; ch++) {
    result.part2_3_length[ch] = nextBits(buffer, cBit, 12);
    result.bigValues[ch] = nextBits(buffer, cBit, 9);
    result.globalGain[ch] = nextBits(buffer, cBit, 8);
    sfc = nextBits(buffer, cBit, 4);
    result.slen[ch][0] = (unsigned char) ScalefactorSizes[sfc][0];
    result.slen[ch][1] = (unsigned char) ScalefactorSizes[sfc][1];
    result.windowSwitchingFlag[ch] = (unsigned char) nextBits(buffer, cBit, 1);

    if (result.windowSwitchingFlag[ch] == 1) {
      result.blockType[ch] = (unsigned char) nextBits(buffer, cBit, 2);
      result.mixedBlockflag[ch] = (unsigned char) nextBits(buffer, cBit, 1);

      for (int region = 0; region < 2; region++) {
        result.tableSelect[ch][region] = nextBits(buffer, cBit, 5);
      }

      for (int window = 0; window < 3; window++) {
        result.subblockGain[ch][window] = nextBits(buffer, cBit, 3);
      }

      if (result.blockType[ch] == 2 && result.mixedBlockflag[ch] == 0) {
        result.region0Count[ch] = 8; // Implicit
      } else {
        result.region0Count[ch] = 7; // Implicit
      }

      result.region1Count[ch] = (unsigned char) (20 - result.region0Count[ch]); // Implicit
    } else {
      for (int region = 0; region < 3; region++) {
        result.tableSelect[ch][region] = nextBits(buffer, cBit, 5);
      }

      result.region0Count[ch] = (unsigned char) nextBits(buffer, cBit, 4);
      result.region1Count[ch] = (unsigned char) nextBits(buffer, cBit, 3);
      result.blockType[ch] = 0; // Implicit
    }

    result.preflag[ch] = (unsigned char) nextBits(buffer, cBit, 1);
    result.scalefacScale[ch] = (unsigned char) nextBits(buffer, cBit, 1);
    result.count1TableSelect[ch] = (unsigned char) nextBits(buffer, cBit, 1);

    // Calculate part2Length
    result.part2Length[ch] = calcPart2Length(&result, ch);
  }

  return result;
}

/**
 * Calculates the length of part 2 of main data
 * @param g Granule to calculate upon
 * @param ch Channel selected
 * @return Length in bits
 */
unsigned int calcPart2Length(Granule *g, int ch) {
  if (g->blockType[ch] == 0 || g->blockType[ch] == 1 || g->blockType[ch] == 3) {
    return (11 * g->slen[ch][0] + 10 * g->slen[ch][1]);
  }

  if (g->blockType[ch] == 2 && g->mixedBlockflag == 0) {
    return (18 * g->slen[ch][0] + 18 * g->slen[ch][1]);
  }

  if (g->blockType[ch] == 2 && g->mixedBlockflag[ch] == 1) {
    return (17 * g->slen[ch][0] + 18 * g->slen[ch][1]);
  }

  fprintf(stderr, "Invalid blockType or mixedBlockFlag combination (%d, %d)?\n",
          g->blockType[ch], g->mixedBlockflag[ch]);
  exit(2);
}

/**
 * Parses the header of frame
 * @param fp File pointer
 * @return Header struct
 */
FrameHeader parseHeader(FILE *fp) {
  FrameHeader fr;
  unsigned char buffer[16];
  size_t cBit = 0;

  // Read the frame header
  fread(buffer, sizeof(char), 4, fp);

  // Check frame sync, checks if the first 12 bits are 1's
  // TODO: support MPEG-2.5
  if ((nextBits(buffer, &cBit, 12) ^ (unsigned) 0xFFF) > 0) {
    long pos = (int) ftell(fp);
    rewind(fp);
    pos = pos - ftell(fp);
    fprintf(stderr, "Unknown format: %2x %2x at pos %lx\n", buffer[0], buffer[1], pos);
    exit(2);
  }

  // Read all values from header
  fr.version = (MPEGVersion) nextBits(buffer, &cBit, 1);
  fr.layer = (Layer) nextBits(buffer, &cBit, 2);
  fr.protection = (bool) nextBits(buffer, &cBit, 1);

  fr.bitrate = BitrateTable[fr.version][fr.layer][nextBits(buffer, &cBit, 4)];
  fr.frequency = nextBits(buffer, &cBit, 2);
  fr.padding = (bool) nextBits(buffer, &cBit, 1);
  fr.private = (bool) nextBits(buffer, &cBit, 1);

  fr.mode = (ChannelMode) nextBits(buffer, &cBit, 2);
  fr.MSStereo = (bool) nextBits(buffer, &cBit, 1);
  fr.intensityStereo = (bool) nextBits(buffer, &cBit, 1);
  fr.copyright = (bool) nextBits(buffer, &cBit, 1);
  fr.home = (bool) nextBits(buffer, &cBit, 1);
  fr.emphasis = nextBits(buffer, &cBit, 2);

  if (fr.protection) {
    fseek(fp, ftell(fp) + 2, SEEK_SET);
    // TODO: verify CRC
    //fread(buffer, 1, 16, fp);
    //fr.crc = buffer[0];
  }
  return fr;
}

/**
 * Jumps to the next frame after the frame provided
 * @param fp File pointer
 * @param curFrame Current frame to jump past
 */
void jumpToNextFrame(FILE *fp, Frame *curFrame) {
  fseek(fp, curFrame->start + frameSize(curFrame), SEEK_SET);
}

/**
 * Calculates the frame size of a given frame
 * @param frame Frame to calculate
 * @return Size of frame
 */
int frameSize(Frame *frame) {
  int freq = FrequencyTable[frame->header.version][frame->header.frequency];
  if (!freq) {
    fprintf(stderr, "No frequency selected! (alleged pos: %x)\n", (unsigned int) frame->start);
    exit(2);
  }

  return ((144000 * frame->header.bitrate) / freq) +
      frame->header.padding;
}

void checkSupported(Frame fr) {
  if (fr.header.version != MPEG1) {
    fprintf(stderr,
            "Only MPEG1 is supported (expected %d, actual %d)\n",
            MPEG1,
            fr.header.version);
  }
  if (fr.header.layer != layer3) {
    fprintf(stderr,
            "Only layer3 is supported (expected %d, actual %d)\n",
            layer3,
            fr.header.layer);
  }
}