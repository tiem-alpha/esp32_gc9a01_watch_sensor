#include "filesystem.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define BACK_GROUND_SIZE (SCREEN_WIDTH*SCREEN_WIDTH*2)
static bool foundFace = false;
bool readImageFile(const char *path, uint8_t *buffer, size_t bufferSize)
{
    File file = SPIFFS.open(path, "r");
    if (!file)
    {
        Serial.println("Không tìm thấy file ảnh!");
        return false;
    }

    size_t bytesRead = file.read(buffer, bufferSize);
    file.close();

    Serial.printf("Đã đọc %d byte từ %s\n", bytesRead, path);
    return bytesRead > 0;
}

void listFiles() {
  Serial.println("Listing files in SPIFFS:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while (file) {
    String name = file.name();
    size_t size = file.size();
    Serial.printf("File: %s, Size: %u bytes\n", name.c_str(), (unsigned int)size);

    if (name == BACK_GROUND_FILE && size>= BACK_GROUND_SIZE ) {
      foundFace = true;
      Serial.printf(" Found /face.bin, size = %u bytes\n", (unsigned int)size);
    }
    file = root.openNextFile();
  }
}

void initFileSystem()
{
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS mount failed!");
        return;
    }
    Serial.println("mount file thành công");
    listFiles();
}

bool getBackgroundFile()
{
  return foundFace; 
}