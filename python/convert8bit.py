from PIL import Image
import os
import matplotlib.pyplot as plt
import cv2
import cv2
import numpy as np

def convert_image_to_8bit_array(image_path):

    image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    height, width = image.shape
    print(f'{height} {width}')
    width = width // 3
    height = height // 3
    image = cv2.resize(image, (width, height), interpolation=cv2.INTER_AREA)
    print(f'{height} {width}')
    # image = (image).astype(np.uint8)
    image = image.flatten()
    return image, width, height


def save_to_header_file(images_data, header_file_path):
    with open(header_file_path, 'w') as header_file:
        header_file.write('#ifndef IMAGE_EIGTH_DATA_H\n')
        header_file.write('#define IMAGE_EIGTH_DATA_H\n\n')
        header_file.write('#include "data_type.h"\n\n')
        struct =""
        
        struct += f'const Image8Bit images8[] = {{'
        for idx, (name, pixel_array, width, height) in enumerate(images_data):
            image  = np.zeros((height, width), dtype=np.uint8)
            lenh = ((width*height))
            list = np.zeros(lenh)
            # print(len(pixel_array))
            # print(lenh)
            header_file.write(f'const uint8_t {name}_8_data[] PROGMEM = {{\n')
            for i in range(0, len(pixel_array)):
                list[i] = pixel_array[i]
                if( i %32 ==0):
                    header_file.write(f'\n')
                header_file.write(f'0x{pixel_array[i]:02X}, ')
            header_file.write('};\n\n')
            struct+= f'\n{{ {name}_8_data, {width}, {height} }},'
            count =0
            for y in range(height):
                for x in range(width):
                    image[y, x] = list[count]
                    count += 1
            # cv2.imshow('Gray Image (All 0)', image)
            # cv2.waitKey(0)
            # cv2.destroyAllWindows()
        struct+= f'\n}};\n\n'
        header_file.write(struct)
        header_file.write('#endif // IMAGE_EIGTH_DATA_H\n')



def main():
    project_folder = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    image_folder = os.path.join(project_folder, 'image')
    source_path = os.path.join(project_folder, 'watch')
    header_file_path = os.path.join(source_path, 'image_eight_data.h')
    images_data = []

    for i in range(10):
        image_path = os.path.join(image_folder, f'{i}.png')
        print(f'anh {i}')
        pixel_array, width, height = convert_image_to_8bit_array(image_path)
        name = f'image_{i}'
        images_data.append((name, pixel_array, width, height))
        # convert8bittorgb888(pixel_array,width,height)

    save_to_header_file(images_data, header_file_path)

if __name__ == '__main__':
    main()