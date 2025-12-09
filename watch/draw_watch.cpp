#include"draw_watch.h"
#include"draw_util.h"
#include "output.h"

static  void drawPointedHand(float angle, int length, int width, int tail, uint16_t color) ;
static void drawHourNumber(); 
void watchInit() {
  ScreenInit();
  drawClockFace();
  DisplayBuffers();
}

void drawClockFace() {
  // Vẽ nền đồng hồ
  Draw565ImageProgmem(0, 0, 240, 240, FACE);

  // Vẽ các số và vạch giờ
  for (int i = 0; i < 12; i++) {
    float angle = i * 30 * DEG_TO_RAD; // 30 độ mỗi giờ (360/12)

    // Vẽ vạch giờ to hơn
    int markOuterX = CLOCK_CENTER_X + sin(angle) * CLOCK_RADIUS;
    int markOuterY = CLOCK_CENTER_Y - cos(angle) * CLOCK_RADIUS;
    int markInnerX = CLOCK_CENTER_X + sin(angle) * (CLOCK_RADIUS - 8); // Vạch giờ dài hơn
    int markInnerY = CLOCK_CENTER_Y - cos(angle) * (CLOCK_RADIUS - 8);

    drawLine(markOuterX, markOuterY, markInnerX, markInnerY, HOUR_MARK_COLOR);

  }

  // Vẽ các vạch phút
  for (int i = 0; i < 60; i++) {
    if (i % 5 != 0) { // Bỏ qua các vạch giờ vì đã vẽ ở trên
      float angle = i * 6 * DEG_TO_RAD; // 6 độ mỗi phút (360/60)
      int x1 = CLOCK_CENTER_X + sin(angle) * CLOCK_RADIUS;
      int y1 = CLOCK_CENTER_Y - cos(angle) * CLOCK_RADIUS;
      int x2 = CLOCK_CENTER_X + sin(angle) * (CLOCK_RADIUS - 3); // Vạch phút ngắn hơn
      int y2 = CLOCK_CENTER_Y - cos(angle) * (CLOCK_RADIUS - 3);
      drawLine(x1, y1, x2, y2, CLOCK_FACE_COLOR);
    }
  }


}

void drawClockHands(int hour, int minute, int second) {

  // Lưu lại trạng thái màn hình xung quanh khu vực đồng hồ
  // (Thay vì xóa riêng từng kim, chúng ta vẽ lại toàn bộ)
  fillCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS - 1, BACKGROUND_COLOR);

  // Vẽ lại mặt đồng hồ
  drawClockFace();
  drawHourNumber();
  // Tính góc cho các kim
  float hour_angle = ((hour * 30) + (minute * 0.5)) * DEG_TO_RAD; // 30 độ mỗi giờ + điều chỉnh theo phút
  float minute_angle = minute * 6 * DEG_TO_RAD; // 6 độ mỗi phút
  float second_angle = second * 6 * DEG_TO_RAD; // 6 độ mỗi giây

  // Vẽ kim giờ (hình tam giác nhọn) - tất cả màu bạc
  drawPointedHand(hour_angle, HOUR_HAND_LENGTH, HOUR_HAND_WIDTH, HOUR_HAND_TAIL, HOUR_COLOR);

  // Vẽ kim phút (hình tam giác nhọn) - màu bạc
  drawPointedHand(minute_angle, MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, MINUTE_HAND_TAIL, MIN_COLOR);

  // Vẽ kim giây (hình tam giác nhọn mảnh hơn) - màu bạc
  drawPointedHand(second_angle, SECOND_HAND_LENGTH, SECOND_HAND_WIDTH, SECOND_HAND_TAIL, SEC_COLOR);

  // Vẽ lại điểm trung tâm
  fillCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, 3, CENTER_COLOR); // Tăng kích thước điểm trung tâm
  DisplayBuffers();
}

void drawPointedHand(float angle, int length, int width, int tail, uint16_t color) {
  // Điểm mút của kim
  int tipX = CLOCK_CENTER_X + sin(angle) * length;
  int tipY = CLOCK_CENTER_Y - cos(angle) * length;

  // Điểm đuôi (phía sau trung tâm)
  int tailX = CLOCK_CENTER_X - sin(angle) * tail;
  int tailY = CLOCK_CENTER_Y + cos(angle) * tail;

  // Góc vuông góc với góc của kim (để tính điểm hai bên)
  float perpAngle = angle + PI / 2;

  // Điểm bên trái chân kim (gần trung tâm đồng hồ)
  int leftBaseX = CLOCK_CENTER_X + sin(perpAngle) * (width / 2);
  int leftBaseY = CLOCK_CENTER_Y - cos(perpAngle) * (width / 2);

  // Điểm bên phải chân kim (gần trung tâm đồng hồ)
  int rightBaseX = CLOCK_CENTER_X - sin(perpAngle) * (width / 2);
  int rightBaseY = CLOCK_CENTER_Y + cos(perpAngle) * (width / 2);

  // Vẽ tam giác cho phần chính của kim (từ chân đến mút)
  fillTriangle(
    leftBaseX, leftBaseY,
    rightBaseX, rightBaseY,
    tipX, tipY,
    color
  );

  // Vẽ phần đuôi kim (tam giác nhỏ hơn ở phía sau)
  fillTriangle(
    leftBaseX, leftBaseY,
    rightBaseX, rightBaseY,
    tailX, tailY,
    color
  );
}

void drawHourNumber(){
  for (int i = 0; i < 12; i++) {
    float angle = i * 30 * DEG_TO_RAD; // 30 độ mỗi giờ (360/12)
    int hourNum = i == 0 ? 12 : i; // Đổi 0 thành 12
    char hourStr[3];
    sprintf(hourStr, "%d", hourNum);

    int numX = CLOCK_CENTER_X + sin(angle) * (CLOCK_RADIUS - 15); // Vị trí số
    int numY = CLOCK_CENTER_Y - cos(angle) * (CLOCK_RADIUS - 15);

    // Canh giữa số
    int textWidth = hourNum < 10 ? 8 : 16;
    int textHeight = 32;

    DrawString( numX - textWidth / 2, numY - textHeight / 2,  hourStr, 0xFFFF);
    }
   
}

/*
  
  }
*/
