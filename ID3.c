//
// Created by hamburger on 10/1/17.
//

#include "ID3.h"
#include <string.h>

/**
 * Parse ID3 header and skip its body
 * @param fp
 * @return ID3 header
 */
ID3 parseID3Header(FILE *fp) {
  ID3 ret;
  unsigned char buffer[10];

  // Read the ID3 header
  fread(buffer, 1, 10, fp);

  // Set the ID3 header ID3Version
  ret.version = (ID3Version) {
      .major = (int) buffer[3],
      .minor = (int) buffer[4]
  };

  // Fetch flags
  ret.flags = (ID3_Flags) buffer[5];

  // Size of the ID3 frame
  ret.size = (buffer[6] << 24) | (buffer[7] << 16) | (buffer[8] << 8) | buffer[9];

  fseek(fp, (long) (ftell(fp) + ret.size), SEEK_SET);
  return ret;
}

/**
 * Determines if a file pointer is pointing on an ID3 block
 * @param fp File pointer
 * @return Whether or not file pointer is pointing on an ID3 block
 */
bool isID3(FILE *fp) {
  int before = (int) ftell(fp);
  unsigned char buffer[3];
  fread(buffer, 1, 3, fp);
  fseek(fp, before, SEEK_SET);
  return strcmp((const char *) buffer, "ID3") == 0;
}