# -*- coding: utf-8 -*-

import re
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import math
# Kích thước màn hình (bạn cần gán giá trị thật)
SCREEN_WIDTH = 240
SCREEN_HEIGHT = 240

# Tọa độ và bán kính đồng hồ
CLOCK_CENTER_X = SCREEN_WIDTH / 2
CLOCK_CENTER_Y = SCREEN_HEIGHT / 2
CLOCK_RADIUS = SCREEN_WIDTH / 2 - 1  # Giảm CLOCK_RADIUS một chút để có chỗ cho số

# Màu sắc (RGB565)
BACKGROUND_COLOR = 0x0000
SILVER_COLOR = 0xC618  # Màu bạc (xám sáng)
CLOCK_FACE_COLOR = 0xFFFF
HOUR_MARK_COLOR = SILVER_COLOR
NUMBER_COLOR = SILVER_COLOR
CENTER_COLOR = 0x0000

KIM_COLOR = 0x355a
HOUR_COLOR = KIM_COLOR
MIN_COLOR = KIM_COLOR
KIM_COLOR_EDGE = 0x50aa
SEC_COLOR = 0x3d02

# Độ dài kim
HOUR_HAND_LENGTH = CLOCK_RADIUS * 0.55
MINUTE_HAND_LENGTH = CLOCK_RADIUS * 0.75
SECOND_HAND_LENGTH = CLOCK_RADIUS * 0.85

# Độ rộng kim
HOUR_HAND_WIDTH = 10
MINUTE_HAND_WIDTH = 7
SECOND_HAND_WIDTH = 3

# Độ dài đuôi kim
HOUR_HAND_TAIL = 12
MINUTE_HAND_TAIL = 15
SECOND_HAND_TAIL = 18

gradient_speed = 1.5

width, height = SCREEN_WIDTH, SCREEN_HEIGHT

def draw_pixel(buff, x, y, color):
    height = len(buff)
    width = len(buff[0]) if height > 0 else 0

    # Kiểm tra giới hạn
    if x < 0 or x >= width or y < 0 or y >= height:
        return

    # Gán màu cho pixel
    buff[y][x] = color
    return

def draw_line(buff, x0, y0, x1, y1, color):
    steep = abs(y1 - y0) > abs(x1 - x0)

    # Nếu đường dốc, hoán đổi x và y
    if steep:
        x0, y0 = y0, x0
        x1, y1 = y1, x1

    # Đảm bảo vẽ từ trái sang phải
    if x0 > x1:
        x0, x1 = x1, x0
        y0, y1 = y1, y0

    dx = x1 - x0
    dy = abs(y1 - y0)
    err = dx / 2
    ystep = 1 if y0 < y1 else -1

    y = y0
    for x in range(x0, x1 + 1):
        if steep:
            draw_pixel(buff, y, x, color)
        else:
            draw_pixel(buff, x, y, color)

        err -= dy
        if err < 0:
            y += ystep
            err += dx
    return

def draw_horizontal_line(buff, x, y, w, color):
    w = int(w)
    x = int(x)
    y = int(y)
    color =  np.uint16(color)
    height = len(buff)
    width = len(buff[0]) if height > 0 else 0

    # Kiểm tra nếu dòng nằm ngoài màn hình
    if y < 0 or y >= height or w <= 0:
        return

    # Cắt phần nằm ngoài bên trái
    if x < 0:
        w += x
        x = 0

    # Cắt phần nằm ngoài bên phải
    if x >= width:
        return
    if x + w > width:
        w = width - x

    # Vẽ đường ngang
    for i in range(w):
        buff[y][x + i] = color
    return

def fill_circle(buff, x0, y0, r, color):
    if r <= 0:
        return

    height = len(buff)
    width = len(buff[0]) if height > 0 else 0

    # Vẽ đường ngang giữa tâm
    draw_horizontal_line(buff, x0 - r, y0, int(2 * r + 1), color)

    f = 1 - r
    ddF_x = 1
    ddF_y = -2 * r
    x = 0
    y = r

    while x < y:
        if f >= 0:
            y -= 1
            ddF_y += 2
            f += ddF_y

        x += 1
        ddF_x += 2
        f += ddF_x

        # Vẽ các dải ngang đối xứng
        draw_horizontal_line(buff, x0 - x, y0 + y, 2 * x + 1, color)
        draw_horizontal_line(buff, x0 - x, y0 - y, 2 * x + 1, color)
        draw_horizontal_line(buff, x0 - y, y0 + x, 2 * y + 1, color)
        draw_horizontal_line(buff, x0 - y, y0 - x, 2 * y + 1, color)
    return


def fill_triangle_gradient(buff, x0, y0, x1, y1, x2, y2, color_center):
    # --- Làm sáng màu RGB565 ---
    def lighten_rgb565(color, k=0.4):
        r = ((color >> 11) & 0x1F) << 3
        g = ((color >> 5) & 0x3F) << 2
        b = (color & 0x1F) << 3
        r = int(r + (255 - r) * k)
        g = int(g + (255 - g) * k)
        b = int(b + (255 - b) * k)
        return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

    color_edge = lighten_rgb565(color_center, 0.4)

    # --- Tính trung điểm đáy và vector trung trực ---
    mx, my = (x0 + x1) / 2, (y0 + y1) / 2
    vx, vy = mx - x2, my - y2  # vector trung trực (từ đỉnh xuống đáy)
    v_len = math.hypot(vx, vy)
    if v_len == 0:
        return
    nx, ny = vx / v_len, vy / v_len  # vector đơn vị trung trực

    # --- Giới hạn vùng vẽ ---
    min_x, max_x = int(min(x0, x1, x2)), int(max(x0, x1, x2))
    min_y, max_y = int(min(y0, y1, y2)), int(max(y0, y1, y2))

    # --- Kiểm tra điểm trong tam giác ---
    def point_in_triangle(px, py):
        d1 = (px - x1)*(y0 - y1) - (x0 - x1)*(py - y1)
        d2 = (px - x2)*(y1 - y2) - (x1 - x2)*(py - y2)
        d3 = (px - x0)*(y2 - y0) - (x2 - x0)*(py - y0)
        has_neg = (d1 < 0) or (d2 < 0) or (d3 < 0)
        has_pos = (d1 > 0) or (d2 > 0) or (d3 > 0)
        return not (has_neg and has_pos)

    # --- Trộn màu RGB565 ---
    def blend_rgb565(c1, c2, t):
        r1 = (c1 >> 11) & 0x1F
        g1 = (c1 >> 5) & 0x3F
        b1 = c1 & 0x1F
        r2 = (c2 >> 11) & 0x1F
        g2 = (c2 >> 5) & 0x3F
        b2 = c2 & 0x1F
        r = int(r1 + (r2 - r1) * t)
        g = int(g1 + (g2 - g1) * t)
        b = int(b1 + (b2 - b1) * t)
        return (r << 11) | (g << 5) | b

    # --- Tô màu ---
    for y in range(min_y, max_y + 1):
        for x in range(min_x, max_x + 1):
            if not point_in_triangle(x, y):
                continue

            # Vector từ đỉnh đến điểm hiện tại
            dx, dy = x - x2, y - y2

            # Khoảng cách vuông góc đến trung trực
            # (dùng tích có hướng để tính độ lệch ngang)
            perp_dist = abs(dx * ny - dy * nx)

            # Chuẩn hóa theo chiều rộng tam giác
            base_half = math.hypot((x1 - x0) / 2, (y1 - y0) / 2)
            t = min((perp_dist / base_half) * gradient_speed, 1.0)

            color = blend_rgb565(color_center, color_edge, t)
            if 0 <= y < buff.shape[0] and 0 <= x < buff.shape[1]:
                buff[y, x] = np.uint16(color)

    return

def swap(a, b):
    return b, a

def fill_triangle(buff, x0, y0, x1, y1, x2, y2, color):
    # Sắp xếp các đỉnh theo thứ tự y tăng dần
    if y0 > y1:
        y0, y1 = swap(y0, y1)
        x0, x1 = swap(x0, x1)
    if y1 > y2:
        y1, y2 = swap(y1, y2)
        x1, x2 = swap(x1, x2)
    if y0 > y1:
        y0, y1 = swap(y0, y1)
        x0, x1 = swap(x0, x1)

    # Nếu tam giác phẳng (cao = 0) thì bỏ qua
    if y0 == y2:
        return

    dx02 = x2 - x0
    dy02 = y2 - y0
    dx01 = x1 - x0
    dy01 = y1 - y0
    dx12 = x2 - x1
    dy12 = y2 - y1

    sa = sb = 0
    last = y1

    # Vẽ phần trên của tam giác
    if y0 != y1:
        for y in range(y0, last + 1):
            a = x0 + dx01 * (y - y0) / dy01 if dy01 != 0 else x0
            b = x0 + dx02 * (y - y0) / dy02 if dy02 != 0 else x0
            if a > b:
                a, b = swap(a, b)
            draw_horizontal_line(buff, int(a), y, int(b - a + 1), color)

    # Vẽ phần dưới của tam giác
    if y1 != y2:
        for y in range(y1, y2 + 1):
            a = x1 + dx12 * (y - y1) / dy12 if dy12 != 0 else x1
            b = x0 + dx02 * (y - y0) / dy02 if dy02 != 0 else x0
            if a > b:
                a, b = swap(a, b)
            draw_horizontal_line(buff, int(a), y, int(b - a + 1), color)
    return 

def draw_clock_face(buff):
    # --- Vẽ nền ---
    h, w = buff.shape

    # --- Vẽ 12 vạch giờ ---
    for i in range(12):
        angle = math.radians(i * 30)  # 30° mỗi giờ
        mark_outer_x = int(CLOCK_CENTER_X + math.sin(angle) * CLOCK_RADIUS)
        mark_outer_y = int(CLOCK_CENTER_Y - math.cos(angle) * CLOCK_RADIUS)
        mark_inner_x = int(CLOCK_CENTER_X + math.sin(angle) * (CLOCK_RADIUS - 8))
        mark_inner_y = int(CLOCK_CENTER_Y - math.cos(angle) * (CLOCK_RADIUS - 8))
        draw_line(buff, mark_outer_x, mark_outer_y, mark_inner_x, mark_inner_y, HOUR_MARK_COLOR)

    # --- Vẽ 60 vạch phút ---
    for i in range(60):
        if i % 5 != 0:  # bỏ qua vạch giờ
            angle = math.radians(i * 6)  # 6° mỗi phút
            x1 = int(CLOCK_CENTER_X + math.sin(angle) * CLOCK_RADIUS)
            y1 = int(CLOCK_CENTER_Y - math.cos(angle) * CLOCK_RADIUS)
            x2 = int(CLOCK_CENTER_X + math.sin(angle) * (CLOCK_RADIUS - 3))
            y2 = int(CLOCK_CENTER_Y - math.cos(angle) * (CLOCK_RADIUS - 3))
            draw_line(buff, x1, y1, x2, y2, CLOCK_FACE_COLOR)
    return 


def draw_pointed_hand(buff, angle, length, width, tail, color):
    # Điểm mút của kim
    tip_x = int(CLOCK_CENTER_X + math.sin(angle) * length)
    tip_y = int(CLOCK_CENTER_Y - math.cos(angle) * length)

    # Điểm đuôi (phía sau trung tâm)
    tail_x = int(CLOCK_CENTER_X - math.sin(angle) * tail)
    tail_y = int(CLOCK_CENTER_Y + math.cos(angle) * tail)

    # Góc vuông góc với kim
    perp_angle = angle + math.pi / 2

    # Hai điểm gốc của kim (gần tâm)
    left_base_x = int(CLOCK_CENTER_X + math.sin(perp_angle) * (width / 2))
    left_base_y = int(CLOCK_CENTER_Y - math.cos(perp_angle) * (width / 2))
    right_base_x = int(CLOCK_CENTER_X - math.sin(perp_angle) * (width / 2))
    right_base_y = int(CLOCK_CENTER_Y + math.cos(perp_angle) * (width / 2))

    # # Vẽ phần chính của kim (tam giác từ gốc đến mũi)
    # fill_triangle(buff,
    #               left_base_x, left_base_y,
    #               right_base_x, right_base_y,
    #               tip_x, tip_y,
    #               color)

    # # Vẽ phần đuôi kim (tam giác nhỏ phía sau)
    # fill_triangle(buff,
    #               left_base_x, left_base_y,
    #               right_base_x, right_base_y,
    #               tail_x, tail_y,
    #               color)
    fill_triangle_gradient(buff,
                  left_base_x, left_base_y,
                  right_base_x, right_base_y,
                  tip_x, tip_y,
                  color)
    
    fill_triangle_gradient(buff,
                  left_base_x, left_base_y,
                  right_base_x, right_base_y,
                  tail_x, tail_y,
                  color)
    return


def draw_clock_hands(buff, hour, minute, second):

    # --- Xóa vùng cũ (vẽ lại nền tròn) ---
    # fill_circle(buff, CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS - 1, BACKGROUND_COLOR)
    draw_clock_face(buff)
    # --- Tính góc cho từng kim ---
    hour_angle = math.radians((hour * 30) + (minute * 0.5))   # 30° mỗi giờ + điều chỉnh theo phút
    minute_angle = math.radians(minute * 6)                   # 6° mỗi phút
    second_angle = math.radians(second * 6)                   # 6° mỗi giây

    # --- Vẽ kim giờ ---
    draw_pointed_hand(buff, hour_angle,
                      HOUR_HAND_LENGTH,  HOUR_HAND_WIDTH, HOUR_HAND_TAIL, HOUR_COLOR)

    # --- Vẽ kim phút ---
    draw_pointed_hand(buff, minute_angle,
                      MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, MINUTE_HAND_TAIL, MIN_COLOR)

    # --- Vẽ kim giây ---
    draw_pointed_hand(buff, second_angle,
                     SECOND_HAND_LENGTH, SECOND_HAND_WIDTH, SECOND_HAND_TAIL, SEC_COLOR)

    # --- Vẽ lại điểm trung tâm ---
    fill_circle(buff, CLOCK_CENTER_X, CLOCK_CENTER_Y, 3, CENTER_COLOR)
    return 


# Đọc file header chứa mảng RGB565
with open("IMAGE.h", "r", encoding="utf-8") as f:
    text = f.read()

# Lấy tất cả giá trị hex trong mảng
hex_values = re.findall(r'0x[0-9A-Fa-f]+', text)

# Chuyển sang mảng numpy kiểu uint16
arr = np.array([int(x, 16) for x in hex_values], dtype=np.uint16)

# Giả sử ảnh có kích thước 240x320 (thay đổi theo ảnh thật)
arr = arr.reshape((height, width))
buffer = arr
# draw pixel trên buffer 
draw_clock_hands(buffer, 6 , 15, 22)


# Giải mã RGB565
r = ((buffer >> 11) & 0x1F) << 3
g = ((buffer >> 5) & 0x3F) << 2
b = (buffer & 0x1F) << 3

# Gộp lại thành ảnh RGB
rgb = np.dstack((r, g, b)).astype(np.uint8)

# Hiển thị bằng matplotlib
plt.imshow(rgb)
plt.axis('off')
plt.show()