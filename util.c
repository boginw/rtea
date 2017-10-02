//
// Created by hamburger on 9/30/17.
//

#include "util.h"
#include <stdio.h>
#include <unistd.h>

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
        fr.frequeny,
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