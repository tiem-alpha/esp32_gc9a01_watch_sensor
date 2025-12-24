#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include"data_type.h"
#include <SPIFFS.h>

#define BACK_GROUND_FILE "/face.bin"
void initFileSystem();
bool readImageFile(const char *path, uint8_t *buffer, size_t bufferSize);
bool getBackgroundFile();
#endif // FILE_SYSTEM_H