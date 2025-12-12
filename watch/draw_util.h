#pragma once 
#include"data_type.h"

#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define BLACK  0x0000
#define WHITE 0xFFFF


// Định nghĩa kích thước màn hình
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define HALF_HEIGHT 120

#define GRADIENT_SPEED (1.5)

// bool InitBuffers(); 
// void FreeBuffers() ; 
void ScreenInit();
void ClearBuffers(); 
void DisplayBuffers(); 
void Draw565ImageProgmem(int x, int y, int width, int height, const uint16_t* pBmp); 
void getBufferAndOffset(int16_t y, uint16_t** buffer, int16_t* offset); 
void drawPixel(int16_t x, int16_t y, uint16_t color); 
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color); 
void drawHorizontalLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) ; 
void swap(int16_t* a, int16_t* b); 
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color); 
void DrawString(int x , int y, const char* str, uint16_t color);
void fillTriangleGradient(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                            uint16_t x2, uint16_t y2, uint16_t colorCenter);
