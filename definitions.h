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
} FrameHeader;

typedef struct Frame{
    FrameHeader header;
};

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

extern int debug;