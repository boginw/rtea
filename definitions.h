//
// Created by hamburger on 9/30/17.
//

#include <stdbool.h>

#ifndef RTEA_DEFINITIONS_H
#define RTEA_DEFINITIONS_H

#endif //RTEA_DEFINITIONS_H

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
    int frequeny;
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
	unsigned int par2_3_length[2];
	unsigned int bigValues[2];
	unsigned int globalGain[2];
	unsigned char scalefacCompress[2];
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
} Granule;

typedef struct FrameSideInfo{
	int main_data_begin;
	char private;
	int scfsi;
	Granule granule0;
	Granule granule1;
} FrameSideInfo;

typedef struct Frame{
    FrameHeader header;
	FrameSideInfo sideInfo;
	// FrameBody body;
	// bool (*isMono)(const struct Frame);
} Frame;

const static int BitrateTable[2][4][16] = {
        // MPEG-2
        {
                // Layer 0
                { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                // Layer 3
                { -1, 8, 16, 24, 32, 64, 80, 56, 64, 128, 160, 112, 128, 256, 320, -1 },
                // Layer 2
                { -1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1 },
                // Layer 1
                { -1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1 }
        },
        // MPEG-1
        {
                // Layer 0
                { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                // Layer 3
                { -1, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1 },
                // Layer 2
                { -1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1 },
                // Layer 1
                { -1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1 }
        }
};
const static int FrequencyTable[2][4] = {
        { 22050, 24000, 16000 },
        { 44100, 48000, 32000 }
};

const static int ScaleFactorGroupTable[4][5] = {
	{  1, 2, 3, 4, 5 },
	{  6, 7, 8, 9,10 },
	{ 11,12,13,14,15 },
	{ 16,17,18,19,20 }
};

extern int debug;