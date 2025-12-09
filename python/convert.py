# -*- coding: utf-8 -*-

from PIL import Image
import sys
import os

def image_to_rgb565_header(input_path, name="IMAGE", output_path="outputT.h"):
    # Mở ảnh
    img = Image.open(input_path).convert("RGB")
    w, h = img.size

    # Cắt ảnh thành hình vuông theo min(width, height)
    min_side = min(w, h)
    left = (w - min_side) // 2
    top = (h - min_side) // 2
    right = left + min_side
    bottom = top + min_side
    img = img.crop((left, top, right, bottom))

    # Resize về 240x240
    img = img.resize((240, 240), Image.LANCZOS)

    # Lấy dữ liệu pixel
    pixels = list(img.getdata())

    # Chuyển sang RGB565
    rgb565_data = []
    for r, g, b in pixels:
        rgb565 = ((r & 0xF8) << 😎 | ((g & 0xFC) << 3) | (b >> 3)
        rgb565_data.append(rgb565)

    # Ghi ra file header
    with open(output_path, "w") as f:
        f.write(f"#define {name}_W 240\n")
        f.write(f"#define {name}_H 240\n\n")
        f.write(f"const uint16_t {name}[] PROGMEM = {{\n")

        for i, val in enumerate(rgb565_data):
            f.write(f"0x{val:04X}, ")
            if (i + 1) % 12 == 0:
                f.write("\n")

        f.write("\n};\n")

    print(f"Đã tạo file header: {output_path}")

# Ví dụ sử dụng
if _name_ == "_main_":
    input_path = "image.jpg"
    name =  "IMAGE"
    output_path = sys.argv[3] if len(sys.argv) > 3 else f"{os.path.splitext(name)[0]}.h"
    image_to_rgb565_header(input_path, name, output_path)