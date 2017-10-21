//
// Created by hamburger on 9/30/17.
//

#include "util.h"
#ifndef RTEA_TABLES_H
#include "tables.h"
#endif //RTEA_TABLES_H
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

int debug;


char *getArgs(int argc, char *argv[]){
    int c, fflag = 0;
    char *filename;

    static char usage[] = "usage: %s [-d] [-f filename] | filename\n";

    while ((c = getopt(argc, argv, "df::")) != -1){
        switch (c) {
            case 'd':
                debug = 1;
                break;
            case 'f':
                fflag = 1;
                filename = optarg;
                break;
            default:break;
        }
    }

    if (!fflag && (optind + 1) > argc) {
        fprintf(stderr, "%s: missing name\n", argv[0]);
        fprintf(stderr, usage, argv[0]);
        exit(1);
    } else if((optind + 1 == argc)){
        filename = argv[optind];
    } else{
        fprintf(stderr, "%s: Too many arguments\n", argv[0]);
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }

    return filename;
}

/**
 * Checks if a given file exists
 * @param filename File path
 * @return Whether or not the file exists
 */
bool fileExists(char* filename){
    return access(filename, F_OK) != -1;
}


bool getBit(char byte, int position){
    return (bool) ((byte >> position) & 1);
}

char setBit(char byte, int pos, bool set){
	return byte ^ (set << (int)pow(7 - pos, 2));
}

/**
 * Receives a list of bytes and returns a select few
 * @param buffer Char array
 * @param start Which bit to start with
 * @param end Which bit to end with
 * @return An integer with selected bytes
 */
unsigned int nextBits(const unsigned char *buffer, size_t* start, int amount){
	int cByte = (int) (*start / 8),
			cBit = (int) (*start % 8);
	size_t totalBits = (size_t) amount;
	unsigned int result = 0;

	for (int i = 0; i < totalBits; i++, cBit++) {
		result |= getBit(buffer[cByte], 7 - cBit);
		result <<= 1;

		if(cBit > 7){
			cBit = 0;
			cByte++;
		}
	}
	*start += amount;

	return result >> 1;
}

size_t nextBytes(const unsigned char *buffer, size_t* start, int amount, char* out){
	int cByte = (int) (*start / 8),
			cBit = (int) (*start % 8),
			tByte = 0,
			tBit = 0;
	size_t totalBits = (size_t) amount;
	unsigned int result = 0;
	size_t size = (size_t) ceilf((float)totalBits / 8);
	out = calloc(size, sizeof(char));

	for (int i = 0; i < totalBits; i++, cBit++, tBit++) {
		result |= getBit(buffer[cByte], 7 - cBit);

		if(result){
			out[tByte] = setBit(out[tByte], tBit, (bool) result);
		}

		if(tBit > 7){
			tBit = 0;
			tByte++;
		}

		if(cBit > 7){
			cBit = 0;
			cByte++;
		}
	}

	*start += amount;
	return size;
}



// TODO: move this
void printHeader(FrameHeader fr){
    printf(
        "\nFrame:\nMPEG Version: MPEG-%d\n"
        "CRC Protection: %s\n"
        "Copyright: %s\n"
        "Bitrate: %d kbits/s\n"
        "Frequeny: %d Hz\n"
        "Private Bit: %d\n"
        "Padding: %d\n"
        "Channel Mode: %d\n",
        !fr.version + 1,
        fr.protection ? "Yes" : "No",
        fr.copyright ? "Yes" : "No",
        fr.bitrate,
		FrequencyTable[fr.version][fr.frequency],
        fr.private,
        fr.padding,
        fr.mode
    );
    if(fr.mode == 1){
        printf(
            "Intensity stereo: %d\n"
            "MS Stereo: %d\n",
            fr.intensityStereo,
            fr.MSStereo
        );
    }
    printf(
        "Home bit: %s\n"
                "Emphasis: %d\n",
        fr.home ? "Yes" : "No",
        fr.emphasis
    );
}