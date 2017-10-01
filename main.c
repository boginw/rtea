#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include "definitions.h"
#include "util.h"

bool isID3(FILE *fp);
ID3 parseID3(FILE *fp);
FrameHeader parseHeader(FILE *fp);
bool jumpToNextFrame(FILE *fp, FrameHeader curFrame);

int debug;

int main(int argc, char *argv[]) {
    char *filename;
    FILE *fp;
    FrameHeader fr;
    filename = getArgs(argc, argv);

    if(fileExists(filename)){
        fp = fopen(filename, "r");
    } else{
        fprintf(stderr, "File does not exist\n");
        exit(1);
    }

    while(!feof(fp)){
        if(isID3(fp)){
            parseID3(fp);
        } else {
            fr = parseHeader(fp);

            if(!jumpToNextFrame(fp, fr)){
                break;
            }

            if(debug){
                printHeader(fr);
            }
        }
    }

    fclose(fp);
    free(filename);
    return 0;
}

FrameHeader parseHeader(FILE *fp){
    FrameHeader fr;
    unsigned char buffer[4];
    unsigned char layerHolder;
    // Read the frame header
    fread(buffer, 1, 4, fp);

    // Check frame sync
    // TODO: support MPEG-2.5
    if(((buffer[0] ^ 0xFF) > 0) || (((buffer[1] >> 4) ^ 0xF) > 0)){
        int pos = (int) ftell(fp);
        rewind(fp);
        pos = (int) (ftell(fp) - pos);
        fprintf(stderr, "Unknown format: %2x %2x at pos %x\n", buffer[0], buffer[1], pos);
        exit(2);
    }

    fr.version = (MPEGVersion) getBit(buffer[1], 3);
    fr.layer = (Layer) ((buffer[1] & 0x6) >> 1);
    fr.protection = getBit(buffer[1], 0);

    fr.bitrate = BitrateTable[fr.version][fr.layer][buffer[2] >> 4];
    fr.frequeny = FrequencyTable[fr.version][(buffer[2] & 0xC) >> 2];
    fr.padding = getBit(buffer[2], 1);
    fr.private = getBit(buffer[2], 0);

    fr.mode = (ChannelMode) (buffer[3] >> 6);
    fr.MSStereo = getBit(buffer[3], 5);
    fr.intensityStereo = getBit(buffer[3], 4);
    fr.copyright = getBit(buffer[3], 3);
    fr.home = getBit(buffer[3],2);
    fr.emphasis = buffer[3] & 0x3;

    return fr;
}

bool jumpToNextFrame(FILE *fp, FrameHeader curFrame){
    int toAdd = ((144000 * curFrame.bitrate) / curFrame.frequeny) + curFrame.padding;
    int curPos = (int) ftell(fp);
    fseek(fp, curPos + toAdd - 5, SEEK_SET);

    // Check if file has ended
    if(feof(fp)){
        return false;
    } else {
        fgetc(fp);
    }

    return true;
}


ID3 parseID3(FILE *fp){
    ID3 ret;
    unsigned char buffer[10];

    // Read the ID3 header
    fread(buffer, 1, 10, fp);

    // Set the ID3 header ID3Version
    ret.version = (ID3Version) {
            .major = (int)buffer[3],
            .minor = (int)buffer[4]
    };

    // Fetch flags
    ret.flags = (ID3_Flags) buffer[5];

    // Size of the ID3 frame
    ret.size = *(unsigned int*) (&buffer[6]);

    fseek(fp, ftell(fp) + ret.size, SEEK_SET);
    return ret;
}

bool isID3(FILE *fp){
    int before = (int) ftell(fp);
    unsigned char buffer[3];
    fread(buffer, 1, 3, fp);
    fseek(fp, before, SEEK_SET);
    return strcmp((const char *) buffer, "ID3") == 0;
}