/*
  FontMaker.cpp - Thư viện Font tương thích utf-8
  Created by Dao Nguyen - IOT47.com
  Liên hệ: daonguyen20798@gmail.com
  Người dùng toàn quyền sử dụng cho mục đích cá nhân
*/
#include "FontMaker.h"

MakeFont::MakeFont(set_px_typedef set_px)
{
   FontMaker_setpx = set_px;
}

void MakeFont::set_font(const MyFont_typedef myfont)
{
   _f_name = myfont.f_name;
   _f_map = myfont.f_map;
}

void MakeFont::setSize(uint8_t size)
{
   this->size = size;
}

unsigned char MakeFont::read(int16_t x, int16_t y, uint16_t txt)
{
   unsigned char temp = x % 8;
   unsigned char x1 = 0x80 >> temp;
   unsigned int str = pgm_read_word(_f_map + txt);
   unsigned char line = pgm_read_byte(str + _f_name - 2);
   return pgm_read_byte(str + _f_name + y * line + x / 8) & x1;
}

unsigned char MakeFont::putChar(int16_t x, int16_t y, uint16_t txt, uint16_t color)
{
   unsigned int str = pgm_read_word(_f_map + txt);
   int size_w = pgm_read_byte(str + _f_name - 4); // lấy chiều rộng Font
   int size_h = pgm_read_byte(str + _f_name - 3); // lấy chiều cao Font

   int start_y = pgm_read_byte(str + _f_name - 1);
   for (int i = 0; i < size_w; i++)
   {
      for (int h = 0; h < size_h; h++)
      {
         if (read(i, h, txt) != 0)
         {
            for (int sch = 0; sch < size; sch++)
            {
               for (int scw = 0; scw < size; scw++)
               {
                  FontMaker_setpx(i * size + x + scw, h * size + y + start_y + sch, color);
               }
            }
         }
      }
   }
return size_w*size;
}

unsigned char MakeFont::getCharLength(uint16_t txt)
{
   unsigned int str = pgm_read_word(_f_map + txt);
   int size_w = pgm_read_byte(str + _f_name - 4)*size; // lấy chiều rộng Font
   return size_w;
}
void MakeFont::print(int16_t x, int16_t y, unsigned char *s, uint16_t color)
{
   unsigned char offset = 0;
   uint16_t utf8_addr;
   while (*s)
   {
      utf8_addr = UTF8_GetAddr(s, &offset);
      x += putChar(x, y, utf8_addr, color) + 1*size;
      s += offset;
   }
}

void MakeFont::print(int16_t x, int16_t y, char *s, uint16_t color)
{
   unsigned char offset = 0;
   uint16_t utf8_addr;
   while (*s)
   {
      utf8_addr = UTF8_GetAddr((unsigned char *)s, &offset);
      x += putChar(x, y, utf8_addr, color) + 1*size;
      s += offset;
   }
}

void MakeFont::print(int16_t x, int16_t y, String str, uint16_t color)
{
   unsigned char offset = 0;
   uint16_t utf8_addr;
   unsigned char *s = (unsigned char *)&str[0];
   while (*s)
   {
      utf8_addr = UTF8_GetAddr((unsigned char *)s, &offset);
      x += putChar(x, y, utf8_addr, color) + 1*size;
      s += offset;
   }
}

uint16_t MakeFont::getLength(unsigned char *s)
{
   unsigned char offset = 0;
   uint16_t utf8_addr;
   uint16_t x = 0;
   while (*s)
   {
      utf8_addr = UTF8_GetAddr(s, &offset);
      x += getCharLength(utf8_addr) + 1*size;
      s += offset;
   }
   return x;
}
uint16_t MakeFont::getLength(char *s)
{
   unsigned char offset = 0;
   uint16_t utf8_addr;
   uint16_t x = 0;
   while (*s)
   {
      utf8_addr = UTF8_GetAddr((unsigned char *)s, &offset);
      x += getCharLength(utf8_addr) + 1*size;
      s += offset;
   }
   return x;
}
uint16_t MakeFont::getLength(String str)
{
   unsigned char offset = 0;
   uint16_t utf8_addr;
   uint16_t x = 0;
   unsigned char *s = (unsigned char *)&str[0];
   while (*s)
   {
      utf8_addr = UTF8_GetAddr(s, &offset);
      x += getCharLength(utf8_addr) + 1*size;
      s += offset;
   }
   return x;
}
