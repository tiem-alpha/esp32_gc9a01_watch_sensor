#include "filesystem.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define BACK_GROUND_SIZE (SCREEN_WIDTH * SCREEN_WIDTH * 2)
static bool foundFace = false;
static bool isbusy = false;
bool readImageFile(const char *path, uint8_t *buffer, size_t bufferSize)
{
    if (isbusy)
        return false;
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

bool readImageFile(const char *path, uint8_t *buffer, size_t bufferSize, uint8_t *buffer2, size_t bufferSize2)
{
    if (isbusy)
        return false;
    File file = SPIFFS.open(path, "r");
    if (!file)
    {
        Serial.println("Không tìm thấy file ảnh!");
        return false;
    }

    size_t bytesRead = file.read(buffer, bufferSize);
    if (bytesRead != bufferSize)
    {
        Serial.println("error read first block");
    }
    bytesRead = file.read(buffer2, bufferSize2);
    if (bytesRead != bufferSize2)
    {
        Serial.println("error read first block");
    }
    file.close();

    return bytesRead > 0;
}

void listFiles()
{
    Serial.println("Listing files in SPIFFS:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while (file)
    {
        String name = file.name();
        size_t size = file.size();
        Serial.printf("File: %s, Size: %u bytes\n", name.c_str(), (unsigned int)size);

        if (memcmp(name.c_str(),BACK_GROUND_FILE, strlen(BACK_GROUND_FILE)) ==0  && size >= BACK_GROUND_SIZE)
        {
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

uint32_t img_offset = 0;
File imgFile;
uint8_t status = 0;

bool openFile(const char *filePath)
{
    isbusy = true;
    img_offset = 0;
    status = 0;
    imgFile = SPIFFS.open(filePath, "w");
    if (imgFile)
    {
        //  status = 1;
        Serial.println("mở file thành công");
        return true;
    }
    Serial.println("không thể mở file");
    status = 1;
    foundFace = false;
    return false;
}

size_t writeFile(uint8_t *data, size_t len, size_t index, size_t total)
{
    size_t writeBytes = 0;

    if (imgFile)
    {
        writeBytes = imgFile.write(data, len);
        img_offset += len;
        if (writeBytes != len)
        {
            foundFace = false;
            status = 1;
            Serial.printf("Write error! written=%u len=%u\n", writeBytes, len);
        }
        else
        {
            Serial.printf("write thành công %d bytes\n", writeBytes);
        }
    }
    else
    {
        status = 1;
    }
    return writeBytes;
}

void closeFile()
{
    if (imgFile)
    {

        if (status == 0)
        {
            foundFace = true;
        }
        img_offset = 0;
        status = 0;
        imgFile.flush();
        imgFile.close();
        Serial.println("Image upload done");
    }
    isbusy = false;
}