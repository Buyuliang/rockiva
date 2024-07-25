#ifndef _COMMON_UTILS_H
#define _COMMON_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned long GetTimeStampMS();

void StrUInt16Array(char* str, int size, const uint16_t* array, int num);

void StrIntArray(char* str, int size, const int* array, int num);

int ReadDataFile(const char *path, char **out_data);

int WriteDataToFile(const char *path, const char *data, unsigned int size);

#ifdef __cplusplus
} //extern "C"
#endif

#endif