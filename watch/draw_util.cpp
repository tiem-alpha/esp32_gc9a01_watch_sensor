#include "draw_util.h"
#include "output.h"
#include <TFT_eSPI.h>
#include "FontMaker.h"

// Khởi tạo đối tượng TFT
TFT_eSPI tft = TFT_eSPI();

void setpx(int16_t x, int16_t y, uint16_t color)
{
  drawPixel(x, y, color);
}

MakeFont myfont(&setpx);
// Con trỏ cho hai buffer được cấp phát động
uint16_t *buffer1 = NULL; // Buffer cho nửa trên màn hình
uint16_t *buffer2 = NULL; // Buffer cho nửa dưới màn hình

// Hàm cấp phát động cho các buffer
static bool InitBuffers()
{
  // Giải phóng buffer cũ nếu đã tồn tại
  if (buffer1 != NULL)
  {
    free(buffer1);
    buffer1 = NULL;
  }

  if (buffer2 != NULL)
  {
    free(buffer2);
    buffer2 = NULL;
  }

  // Cấp phát buffer mới
  buffer1 = (uint16_t *)malloc(SCREEN_WIDTH * HALF_HEIGHT * sizeof(uint16_t));
  buffer2 = (uint16_t *)malloc(SCREEN_WIDTH * HALF_HEIGHT * sizeof(uint16_t));

  // Kiểm tra xem cấp phát có thành công không
  if (buffer1 == NULL || buffer2 == NULL)
  {
    // Xử lý lỗi - giải phóng bất kỳ bộ nhớ nào đã được cấp phát
    if (buffer1 != NULL)
    {
      free(buffer1);
      buffer1 = NULL;
    }

    if (buffer2 != NULL)
    {
      free(buffer2);
      buffer2 = NULL;
    }

    Serial.println("Failed to allocate memory for buffers");
    return false;
  }

  // Khởi tạo buffer với giá trị 0
  memset(buffer1, 0, SCREEN_WIDTH * HALF_HEIGHT * sizeof(uint16_t));
  memset(buffer2, 0, SCREEN_WIDTH * HALF_HEIGHT * sizeof(uint16_t));

  return true;
}

// Hàm giải phóng bộ nhớ buffer
static void FreeBuffers()
{
  if (buffer1 != NULL)
  {
    free(buffer1);
    buffer1 = NULL;
  }

  if (buffer2 != NULL)
  {
    free(buffer2);
    buffer2 = NULL;
  }
}

void ScreenInit()
{
  // Khởi tạo màn hình
  tft.init();
  tft.setRotation(0);
  myfont.set_font(VN);
  // Cấp phát bộ nhớ cho các buffer
  if (!InitBuffers())
  {
    // Serial.println("Buffer initialization failed!");
    return;
  }
}

// Hàm xóa toàn bộ buffer (màu đen)
void ClearBuffers()
{
  // Kiểm tra xem các buffer đã được cấp phát chưa
  if (buffer1 == NULL || buffer2 == NULL)
    return;

  memset(buffer1, 0, SCREEN_WIDTH * HALF_HEIGHT * sizeof(uint16_t));
  memset(buffer2, 0, SCREEN_WIDTH * HALF_HEIGHT * sizeof(uint16_t));
}

// Hàm hiển thị hai buffer lên màn hình sử dụng TFT_eSPI
void DisplayBuffers()
{
  // Kiểm tra xem các buffer đã được cấp phát chưa
  if (buffer1 == NULL || buffer2 == NULL)
    return;

  // Hiển thị nửa trên của màn hình
  tft.setAddrWindow(0, 0, SCREEN_WIDTH, HALF_HEIGHT);
  tft.pushColors(buffer1, SCREEN_WIDTH * HALF_HEIGHT);

  // Hiển thị nửa dưới của màn hình
  tft.setAddrWindow(0, HALF_HEIGHT, SCREEN_WIDTH, HALF_HEIGHT);
  tft.pushColors(buffer2, SCREEN_WIDTH * HALF_HEIGHT);
}

void Draw565ImageProgmem(int x, int y, int width, int height, const uint16_t *pBmp)
{
  // Kiểm tra xem các buffer đã được cấp phát chưa
  if (buffer1 == NULL || buffer2 == NULL)
    return;
  const int sizePixels = width * height;
  for (int i = 0; i < sizePixels; i++)
  {

    if (i < HALF_HEIGHT * SCREEN_WIDTH)
    {
      buffer1[i] = pgm_read_word(&(pBmp[i]));
    }
    else
    {
      buffer2[i - HALF_HEIGHT * SCREEN_WIDTH] = pgm_read_word(&(pBmp[i]));
    }
  }
}

// Hàm để lấy buffer và offset đúng dựa trên tọa độ y
void getBufferAndOffset(int16_t y, uint16_t **buffer, int16_t *offset)
{
  if (y < HALF_HEIGHT)
  {
    *buffer = buffer1;
    *offset = y * SCREEN_WIDTH;
  }
  else
  {
    *buffer = buffer2;
    *offset = (y - HALF_HEIGHT) * SCREEN_WIDTH;
  }
}

// Hàm vẽ pixel
void drawPixel(int16_t x, int16_t y, uint16_t color)
{
  // Kiểm tra giới hạn
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
  {
    return;
  }

  uint16_t *buffer;
  int16_t offset;
  getBufferAndOffset(y, &buffer, &offset);

  buffer[offset + x] = color;
}

// Thuật toán Bresenham vẽ đường thẳng
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (steep)
  {
    // Đổi x và y
    int16_t temp = x0;
    x0 = y0;
    y0 = temp;

    temp = x1;
    x1 = y1;
    y1 = temp;
  }

  if (x0 > x1)
  {
    // Đổi điểm bắt đầu và kết thúc
    int16_t temp = x0;
    x0 = x1;
    x1 = temp;

    temp = y0;
    y0 = y1;
    y1 = temp;
  }

  int16_t dx = x1 - x0;
  int16_t dy = abs(y1 - y0);
  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  }
  else
  {
    ystep = -1;
  }

  for (; x0 <= x1; x0++)
  {
    if (steep)
    {
      drawPixel(y0, x0, color);
    }
    else
    {
      drawPixel(x0, y0, color);
    }

    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

// Vẽ đường ngang tối ưu
void drawHorizontalLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  // Kiểm tra giới hạn
  if (y < 0 || y >= SCREEN_HEIGHT || x >= SCREEN_WIDTH)
  {
    return;
  }

  if (x < 0)
  {
    w += x;
    x = 0;
  }

  if ((x + w) > SCREEN_WIDTH)
  {
    w = SCREEN_WIDTH - x;
  }

  if (w <= 0)
  {
    return;
  }

  uint16_t *buffer;
  int16_t offset;
  getBufferAndOffset(y, &buffer, &offset);

  for (int16_t i = 0; i < w; i++)
  {
    buffer[offset + x + i] = color;
  }
}

// Tô hình tròn sử dụng thuật toán midpoint circle
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  // Kiểm tra bán kính
  if (r <= 0)
  {
    return;
  }

  // Tối ưu vẽ đường ngang
  drawHorizontalLine(x0 - r, y0, 2 * r + 1, color);

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    // Vẽ các dòng ngang giữa các điểm đối xứng
    drawHorizontalLine(x0 - x, y0 + y, 2 * x + 1, color);
    drawHorizontalLine(x0 - x, y0 - y, 2 * x + 1, color);
    drawHorizontalLine(x0 - y, y0 + x, 2 * y + 1, color);
    drawHorizontalLine(x0 - y, y0 - x, 2 * y + 1, color);
  }
}

// Hàm sort có sử dụng để vẽ tam giác
void swap(int16_t *a, int16_t *b)
{
  int16_t t = *a;
  *a = *b;
  *b = t;
}

// Tô tam giác
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  // Sắp xếp các điểm theo y (y0 <= y1 <= y2)
  if (y0 > y1)
  {
    swap(&y0, &y1);
    swap(&x0, &x1);
  }
  if (y1 > y2)
  {
    swap(&y1, &y2);
    swap(&x1, &x2);
  }
  if (y0 > y1)
  {
    swap(&y0, &y1);
    swap(&x0, &x1);
  }

  // Trường hợp tam giác có chiều cao = 0
  if (y0 == y2)
  {
    // Tam giác có chiều cao = 0, không vẽ
    return;
  }

  int16_t a, b, y, last;

  // Tính hệ số góc các cạnh
  // Cạnh từ y0 đến y2
  int16_t dx02 = x2 - x0,
          dy02 = y2 - y0,
          dx01 = x1 - x0,
          dy01 = y1 - y0,
          dx12 = x2 - x1,
          dy12 = y2 - y1;

  int32_t sa = 0,
          sb = 0;

  // Đối với cạnh dưới, luôn có y0 đến y1
  last = y1;

  // Nếu y0 = y1 (cạnh dưới nằm ngang), xử lý phần còn lại
  if (y0 == y1)
  {
    // Sắp xếp x0 và x1
    if (x0 > x1)
    {
      swap(&x0, &x1);
    }

    // Vẽ đường ngang
    for (a = x0; a <= x1; a++)
    {
      drawPixel(a, y0, color);
    }

    // Không có cạnh dưới để vẽ
    sa = dx12 * 256 / dy12;
    sb = dx02 * 256 / dy02;
  }
  else
  {
    // Cạnh dưới không ngang
    sa = dx01 * 256 / dy01;
    sb = dx02 * 256 / dy02;
  }

  // Vẽ từng dòng quét từ dưới lên trên
  for (y = y0; y <= last; y++)
  {
    a = x0 + (sa * (y - y0)) / 256;
    b = x0 + (sb * (y - y0)) / 256;

    if (a > b)
    {
      swap(&a, &b);
    }

    // Vẽ đường ngang
    drawHorizontalLine(a, y, b - a + 1, color);
  }

  // Vẽ phần cạnh trên nếu y1 != y2
  if (y1 != y2)
  {
    sa = dx12 * 256 / dy12;

    for (y = y1 + 1; y <= y2; y++)
    {
      a = x1 + (sa * (y - y1)) / 256;
      b = x0 + (sb * (y - y0)) / 256;

      if (a > b)
      {
        swap(&a, &b);
      }

      drawHorizontalLine(a, y, b - a + 1, color);
    }
  }
}

void DrawString(int x, int y, const char *str, uint16_t color)
{
  myfont.print(x, y, str, color, BLACK);
}

static uint16_t getRGB565(uint8_t r, uint8_t g, uint8_t b)
{
   return (uint16_t)((((r >> 3) << 11)&0xF800) | (((g >> 2) << 5)&0xFFE0) | (uint16_t)(((b >> 3)&0x001F) & 0xFFFF));
}

// Khoảng cách từ điểm (px,py) đến đường thẳng ax + by + c = 0
float lineDistance(float px, float py, float A, float B, float C)
{
    return fabsf(A * px + B * py + C) / sqrtf(A*A + B*B);
}

// Giảm sáng RGB565
uint16_t darkenRGB565(uint16_t color, float factor)
{
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5)  & 0x3F;
    uint8_t b =  color        & 0x1F;

    r = (uint8_t)(r * factor);
    g = (uint8_t)(g * factor);
    b = (uint8_t)(b * factor);

    return (r << 11) | (g << 5) | b;
}

void fillTriangleGradient(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                            uint16_t x2, uint16_t y2, uint16_t colorCenter)
{
    // Trung điểm cạnh AB
    float xm = (x0 + x1) * 0.5f;
    float ym = (y0 + y1) * 0.5f;

    // Phương trình đường CM
    // (y - y2) = k (x - x2)
    // Đưa về dạng Ax + By + C = 0
    float A = ym - y2;
    float B = x2 - xm;
    float C = xm*y2 - x2*ym;

    // Khoảng cách xa nhất từ A hoặc B đến đường CM
    float dA = lineDistance(x0, y0, A, B, C);
    float dB = lineDistance(x1, y1, A, B, C);
    float maxDist = fmaxf(dA, dB);
    if (maxDist < 1.0f) maxDist = 1.0f;

    // Sắp xếp theo y để scanline
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
    if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

    auto interp = [&](float y, float xA, float yA, float xB, float yB){
        if (fabs(yB - yA) < 0.001f) return xA;
        return xA + (xB - xA) * ((y - yA) / (float)(yB - yA));
    };

    // Scan tam giác (2 phần)
    for (int phase = 0; phase < 2; phase++)
    {
        int sy = (phase == 0 ? y0 : y1);
        int ey = (phase == 0 ? y1 : y2);

        for (int y = sy; y <= ey; y++)
        {
            float xa = (phase == 0) ?
                interp(y, x0, y0, x1, y1) :
                interp(y, x1, y1, x2, y2);

            float xb = interp(y, x0, y0, x2, y2);

            if (xa > xb) std::swap(xa, xb);

            for (int x = (int)xa; x <= (int)xb; x++)
            {
                float d = lineDistance(x, y, A, B, C);
                float t = d / maxDist;
                if (t > 1.0f) t = 1.0f;

                float factor = 1.0f - t;

                uint16_t color = darkenRGB565(colorCenter, factor);

              drawPixel(x, y, color);  
            }
        }
    }
}
