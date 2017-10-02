#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"
#include "util.h"
#include "ID3.h"

FrameHeader parseHeader(FILE *fp);
bool jumpToNextFrame(FILE *fp, FrameHeader curFrame);

FrameSideInfo parseSideInfo(FILE *fp, FrameHeader fh);

int debug;

int main(int argc, char *argv[]) {
    char *filename;
    FILE *fp;
    Frame fr;
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
            fr.header = parseHeader(fp);
			fr.sideInfo = parseSideInfo(fp, fr.header);

            if(!jumpToNextFrame(fp, fr.header)){
                break;
            }

            if(debug){
                printHeader(fr.header);
            }
        }
    }

    fclose(fp);
    free(filename);
    return 0;
}

FrameSideInfo parseSideInfo(FILE *fp, FrameHeader fh) {
	FrameSideInfo result;
	bool isMono = fh.mode == SINGLE;
	size_t size = isMono ? 17 : 32;
	unsigned char *buffer = calloc(sizeof(char), size);
	fread(buffer, 1, size, fp);

	result.main_data_begin = (int) (buffer[0] << 8) & getBit(buffer[1], 7);

	if(isMono){
		result.private = (char)((buffer[1] >> 2) & 0x1F);
		result.scfsi = (((buffer[1] & 0x3) << 2) & (buffer[2] >> 6));
	} else {
		result.private = (char)((buffer[1] >> 4) & 0x7);
		result.scfsi = ((buffer[1] & 0x1F) << 4) & (buffer[2] >> 4);
	}

	printf("%x\n", buffer[1]);

	free(buffer);
	return result;
}

FrameHeader parseHeader(FILE *fp){
    FrameHeader fr;
    unsigned char buffer[16];

    // Read the frame header
    fread(buffer, 1, 4, fp);

	// Check frame sync
	// TODO: support MPEG-2.5
	if(((buffer[0] ^ 0xFF) > 0) || (((buffer[1] >> 4) ^ 0xF) > 0)){
		int pos = (int) ftell(fp);
		rewind(fp);
		pos = pos - ftell(fp);
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

	if(fr.protection){
		fseek(fp, ftell(fp) + 16, SEEK_SET);
		// TODO: verify CRC
		//fread(buffer, 1, 16, fp);
		//fr.crc = buffer[0];
	}
	return fr;
}

// This function should be deleted once all frames can be parsed
bool jumpToNextFrame(FILE *fp, FrameHeader curFrame){
    int toAdd = ((144000 * curFrame.bitrate) / curFrame.frequeny) + curFrame.padding;
    int curPos = (int) ftell(fp);

	if(curFrame.protection){
		toAdd -= 16;
	}

	if(curFrame.mode == SINGLE){
		toAdd -= 17;
	} else {
		toAdd -= 32;
	}

    fseek(fp, curPos + toAdd - 5, SEEK_SET);

    // Check if file has ended
    if(feof(fp)){
        return false;
    } else {
        fgetc(fp);
    }

    return true;
}
