//
// Created by hamburger on 9/30/17.
//

#include <stdbool.h>
#include <stdint.h>

#ifndef RTEA_DEFINITIONS_H
#define RTEA_DEFINITIONS_H

#endif //RTEA_DEFINITIONS_H

typedef struct { /* Scale factor band indices,for long and short windows */
	unsigned l[23];
	unsigned s[14];
} t_sf_band_indices;

typedef struct hufftables{
	const unsigned short * hufftable;
	uint16_t treelen;
	uint8_t linbits;
} hufftables;

typedef enum Layer{
    layer1 = 3,
    layer2 = 2,
    layer3 = 1
} Layer;

typedef enum MPEGVersion{
    MPEG2 = 0,
    MPEG1 = 1
} MPEGVersion;

typedef enum ChannelMode{
    STEREO  = 0,
    JSTEREO = 1,
    DUAL    = 2,
    SINGLE  = 3
} ChannelMode;

typedef struct FrameHeader {
    MPEGVersion version;
    bool protection;
    Layer layer;
    int bitrate;
    int frequency;
    bool private;
    bool padding;
    ChannelMode mode;
    bool intensityStereo;
    bool MSStereo;
    bool copyright;
    bool home;
    int emphasis;
	int crc;
} FrameHeader;

typedef struct Granule{
	unsigned int part2_3_length[2];
	unsigned int bigValues[2];
	unsigned int globalGain[2];
	unsigned char scalefacCompress[2];
	unsigned int slen[2][2];
	unsigned char windowSwitchingFlag[2];
	unsigned char blockType[2];
	unsigned int tableSelect[2][3];
	unsigned char mixedBlockflag[2];
	unsigned int subblockGain[2][3];
	unsigned char region0Count[2];
	unsigned char region1Count[2];
	unsigned char preflag[2];
	unsigned char scalefacScale[2];
	unsigned char count1TableSelect[2];
	unsigned int part2Length[2];
	unsigned int count1[2];
} Granule;

typedef struct FrameSideInfo{
	unsigned int main_data_begin;
	unsigned char private;
	bool scfsi[2][4];
	Granule granule[2];
} FrameSideInfo;

typedef struct{
	size_t length;
	unsigned int scalefac_l[2][2][21];    /* 0-4 bits */
	unsigned int scalefac_s[2][2][12][3]; /* 0-4 bits */
	float is[2][2][576]; // Huffman coded freq. lines
} FrameMainData;

typedef struct Frame{
	long start;
    FrameHeader header;
	FrameSideInfo sideInfo;
	FrameMainData mainData;
} Frame;

extern int debug;