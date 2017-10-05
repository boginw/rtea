#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"
#include "ID3.h"
#include "util.h"

FrameHeader parseHeader(FILE *fp);
bool jumpToNextFrame(FILE *fp, FrameHeader curFrame);
FrameSideInfo parseSideInfo(FILE *fp, FrameHeader fh);
Granule parseGranuleSideInfo(bool isMono, const unsigned char *buffer, size_t *cBit);

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
			parseID3Header(fp);
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
	size_t cBit = 0;

	result.main_data_begin = nextBits(buffer, &cBit, 9);

	result.private = (char) nextBits(buffer, &cBit, (isMono ? 5 : 3));
	result.scfsi = nextBits(buffer, &cBit, (isMono ? 4 : 8));

	result.granule0 = parseGranuleSideInfo(isMono, buffer, &cBit);
	result.granule1 = parseGranuleSideInfo(isMono, buffer, &cBit);
	free(buffer);
	return result;
}

Granule parseGranuleSideInfo(bool isMono, const unsigned char *buffer, size_t *cBit){
	Granule result = {};
	int nch = isMono ? 1 : 2;

	for (int ch = 0; ch < nch; ch++) {
		result.par2_3_length[ch] = nextBits(buffer, cBit, 12);
		result.bigValues[ch] = nextBits(buffer, cBit, 9);
		result.globalGain[ch] = nextBits(buffer, cBit, 8);
		result.scalefacCompress[ch] = (unsigned char) nextBits(buffer, cBit, 4);
		result.windowSwitchingFlag[ch] = (unsigned char) nextBits(buffer, cBit, 1);

		if(result.windowSwitchingFlag[ch] == 1){
			result.blockType[ch] = (unsigned char) nextBits(buffer, cBit, 2);
			result.mixedBlockflag[ch] = (unsigned char) nextBits(buffer, cBit, 1);

			for (int region = 0; region < 2; region++) {
				result.tableSelect[ch][region] = nextBits(buffer, cBit, 5);
			}

			for (int window = 0; window < 3; window++) {
				result.subblockGain[ch][window] = nextBits(buffer, cBit, 3);
			}

			if(result.blockType[ch] == 2 && result.mixedBlockflag[ch] == 0){
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
	}

	return result;
}

FrameHeader parseHeader(FILE *fp){
    FrameHeader fr;
    unsigned char buffer[16];
	size_t cBit = 0;

    // Read the frame header
    fread(buffer, 1, 4, fp);

	// Check frame sync
	// TODO: support MPEG-2.5
	if((nextBits(buffer, &cBit, 12) ^ 0xFFF) > 0){
		int pos = (int) ftell(fp);
		rewind(fp);
		pos = pos - ftell(fp);
		fprintf(stderr, "Unknown format: %2x %2x at pos %x\n", buffer[0], buffer[1], pos);
		exit(2);
	}

	fr.version = (MPEGVersion) nextBits(buffer, &cBit, 1);
	fr.layer = (Layer) nextBits(buffer, &cBit, 2);
	fr.protection = (bool) nextBits(buffer, &cBit, 1);

	fr.bitrate = BitrateTable[fr.version][fr.layer][nextBits(buffer, &cBit, 4)];
	fr.frequeny = FrequencyTable[fr.version][nextBits(buffer, &cBit, 2)];
	fr.padding = (bool) nextBits(buffer, &cBit, 1);
	fr.private = (bool) nextBits(buffer, &cBit, 1);

	fr.mode = (ChannelMode) nextBits(buffer, &cBit, 2);
	fr.MSStereo = (bool) nextBits(buffer, &cBit, 1);
	fr.intensityStereo = (bool) nextBits(buffer, &cBit, 1);
	fr.copyright = (bool) nextBits(buffer, &cBit, 1);
	fr.home = (bool) nextBits(buffer, &cBit, 1);
	fr.emphasis = nextBits(buffer, &cBit, 2);

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
