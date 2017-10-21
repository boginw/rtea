//
// Created by hamburger on 9/30/17.
//

#include <stdbool.h>
#include <limits.h>

#ifndef RTEA_DEFINITIONS_H
#include "definitions.h"
#endif

#ifndef RTEA_UTIL_H
#define RTEA_UTIL_H

#endif //RTEA_UTIL_H

bool fileExists(char* filename);
bool getBit(char byte, int position);
char setBit(char byte, int pos, bool set);
void printHeader(FrameHeader fr);
char *getArgs(int argc, char *argv[]);
unsigned int nextBits(const unsigned char *buffer, size_t *start, int amount);