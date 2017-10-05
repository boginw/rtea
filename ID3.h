//
// Created by hamburger on 10/1/17.
//

#include <stdbool.h>
#include <stdio.h>

#ifndef RTEA_ID3_H
#define RTEA_ID3_H

#endif //RTEA_ID3_H

typedef struct ID3Version{
    int major;
    int minor;
} ID3Version;

typedef enum ID3_Flags{
    UNSYNC = 0x80,
    EXTHEA = 0x40,
    EXPERH = 0x20,
    FOOTPR = 0x10
} ID3_Flags;

typedef struct ID3{
    ID3Version version;
    ID3_Flags flags;
    unsigned int size;
} ID3;

bool isID3(FILE *fp);
ID3 parseID3Header(FILE *fp);
