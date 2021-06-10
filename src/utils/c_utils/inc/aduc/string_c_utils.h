/**
 * @file string_c_utils.h
 * @brief String utilities for C code.
 *
 * @copyright Copyright (c) 2019, Microsoft Corp.
 */
#ifndef ADUC_STRING_C_UTILS_H
#define ADUC_STRING_C_UTILS_H

#include <aduc/c_utils.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

EXTERN_C_BEGIN

char* ADUC_StringUtils_Trim(char* str);

bool ADUC_ParseUpdateType(const char* updateType, char** updateTypeName, unsigned int* updateTypeVersion);

bool ReadDelimitedValueFromFile(const char* fileName, const char* key, char* value, unsigned int valueLen);

bool LoadBufferWithFileContents(const char* filePath, char* strBuffer, const size_t strBuffSize);

bool atoul(const char* str, unsigned long* converted);

bool atoui(const char* str, unsigned int* ui);

char* ADUC_StringFormat(const char* fmt, ...);

EXTERN_C_END

#endif // ADUC_STRING_C_UTILS_H
