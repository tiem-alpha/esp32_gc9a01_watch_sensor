#pragma once

#define CLOCK_CENTER_X (SCREEN_WIDTH / 2)
#define CLOCK_CENTER_Y (SCREEN_HEIGHT / 2)
#define CLOCK_RADIUS (SCREEN_WIDTH / 2 - 1)  // Giảm radius một chút để có chỗ cho số

// Màu sắc
#define BACKGROUND_COLOR 0x0000
#define SILVER_COLOR 0xC618  // Màu bạc (xám sáng)
#define CLOCK_FACE_COLOR 0xFFFF
#define HOUR_MARK_COLOR SILVER_COLOR
#define NUMBER_COLOR SILVER_COLOR
#define CENTER_COLOR 0x0000

#define HOUR_COLOR 0x2eac
#define MIN_COLOR 0x2eac
#define SEC_COLOR 0xbfa6

// Độ dài kim
#define HOUR_HAND_LENGTH (CLOCK_RADIUS * 0.55)  // Tăng kích thước kim
#define MINUTE_HAND_LENGTH (CLOCK_RADIUS * 0.75)
#define SECOND_HAND_LENGTH (CLOCK_RADIUS * 0.85)

// Độ rộng kim (ở phần rộng nhất) - tăng kích thước
#define HOUR_HAND_WIDTH 10    // Tăng từ 7 lên 10
#define MINUTE_HAND_WIDTH 7   // Tăng từ 5 lên 7  
#define SECOND_HAND_WIDTH 3   // Tăng từ 2 lên 3

// Độ dài đuôi kim (phần đuôi phía sau trung tâm)
#define HOUR_HAND_TAIL 12
#define MINUTE_HAND_TAIL 15
#define SECOND_HAND_TAIL 18

void watchInit(); 
void drawClockFace(); 
void drawClockHands(int hour, int minute, int second) ; 


// int hour, minute, second;
