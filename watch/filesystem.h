#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include"data_type.h"
#include <SPIFFS.h>

#define BACK_GROUND_FILE "/face.bin"
void initFileSystem();
bool readImageFile(const char *path, uint8_t *buffer, size_t bufferSize);
bool getBackgroundFile();
void closeFile();
size_t writeFile(uint8_t *data, size_t len, size_t index, size_t total);
bool openFile(const char *filePath);
#endif // FILE_SYSTEM_H