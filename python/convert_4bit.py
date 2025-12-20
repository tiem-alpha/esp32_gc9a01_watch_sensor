from PIL import Image
import os
import matplotlib.pyplot as plt
import cv2
import cv2
import numpy as np

def convert_image_to_4bit_array(image_path):

    image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    height, width = image.shape
    thead =10
    print(f'{height} {width}')
    image[image >= thead] = 255
    image[image < thead] = 0
    width = width // 2
    height = height // 2
    image = cv2.resize(image, (width, height), interpolation=cv2.INTER_AREA)
    # height, width = image.shape
    # image = 255 - image 
    image = (image // 16).astype(np.uint8)
    image = image.flatten()
    # Nếu số pixel lẻ, thêm 1 pixel 0 để đủ cặp
    if len(image) % 2 != 0:
        image = np.append(image, 0)
    high = image[0::2] << 4
    low = image[1::2] & 0x0F
    packed = (high | low).astype(np.uint8)
    return packed, width, height

def convert4bittorgb888(pixel_array, width, height):
    image  = np.zeros((height, width), dtype=np.uint8)
    lenh = ((width*height))
    list = np.zeros(lenh)
    for i in range(0, len(pixel_array)):
        low = ((pixel_array[i]&0x0F) * 17).astype(np.uint8)  # 17 = 255/15
        high = ((pixel_array[i]&0xF0)).astype(np.uint8)
        if (i*2 < lenh):
            list[i*2] = high
        if (i*2+1 < lenh):
            list[i*2+1] =low
        # list.append(low)
    # img_array = np.array(list, dtype=np.uint8)
    print(f'{height}-{width}')
    print(len(list))
    count =0
    for y in range(height):
        for x in range(width):
            image[y, x] = list[count]
            count += 1
    cv2.imshow('Gray Image (All 0)', image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def save_to_header_file(images_data, header_file_path):
    with open(header_file_path, 'w') as header_file:
        header_file.write('#ifndef IMAGE_DATA_H\n')
        header_file.write('#define IMAGE_DATA_H\n\n')
        header_file.write('#include "data_type.h"\n\n')
        struct =""
        struct += f'const Image4Bit images[] = {{'
        for idx, (name, pixel_array, width, height) in enumerate(images_data):
            header_file.write(f'const uint8_t {name}_data[] PROGMEM = {{\n')
            for i in range(0, len(pixel_array)):
                # line = ', '.join(f'0x{pixel:02X}' for pixel in pixel_array[i:i+12])
                if( i %16 ==0):
                    header_file.write(f'\n')
                header_file.write(f'0x{pixel_array[i]:02X}, ')
            header_file.write('};\n\n')
            struct+= f'\n{{ {name}_data, {width}, {height} }},'
        struct+= f'\n}};\n\n'
        header_file.write(struct)
        header_file.write('#endif // IMAGE_DATA_H\n')



def main():
    project_folder = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    image_folder = os.path.join(project_folder, 'image')
    source_path = os.path.join(project_folder, 'watch')
    header_file_path = os.path.join(source_path, 'image_data.h')
    images_data = []

    for i in range(10):
        image_path = os.path.join(image_folder, f'{i}.png')
        pixel_array, width, height = convert_image_to_4bit_array(image_path)
        name = f'image_{i}'
        images_data.append((name, pixel_array, width, height))
        # convert4bittorgb888(pixel_array,width,height)

    save_to_header_file(images_data, header_file_path)

if __name__ == '__main__':
    main()