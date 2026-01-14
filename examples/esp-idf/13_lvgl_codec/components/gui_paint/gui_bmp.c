/*****************************************************************************
* | File      	:   BMP_APP.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                   The bmp picture is read from the SD card and drawn into the buffer
*                
*----------------
* |	This version:   V1.0
* | Date        :   2024-12-06
* | Info        :   Basic version
*
******************************************************************************/ 
#include "gui_bmp.h"

// Function to extract pixel color based on the bit depth of the BMP image
UWORD ExtractPixelColor(UBYTE *row_data, int col, int bBitCount, BMPINF *bmpInfoHeader) {
    UWORD color = 0;
    static RGBQUAD RGBPAD[256];  // Palette for 256-color images
    
    switch (bBitCount) {
        case 1: {  // 1 bit per pixel (black and white)
            int byte_offset = col / 8;   // 1 byte for every 8 pixels
            int bit_offset = 7 - (col % 8); // High bit first
            UBYTE bit = (row_data[byte_offset] >> bit_offset) & 0x01;
            color = bit ? 0xFFFF : 0x0000;  // White or Black
            break;
        }
        case 4: {  // 4 bits per pixel (16 colors)
            int byte_offset = col / 2;   // 1 byte for every 2 pixels
            int nibble_offset = (col % 2 == 0) ? 4 : 0; // High nibble or low nibble
            UBYTE index = (row_data[byte_offset] >> nibble_offset) & 0x0F;
            color = RGB(RGBPAD[index].rgbRed, RGBPAD[index].rgbGreen, RGBPAD[index].rgbBlue);
            break;
        }
        case 8: {  // 8 bits per pixel (256 colors)
            UBYTE index = row_data[col];
            color = RGB(RGBPAD[index].rgbRed, RGBPAD[index].rgbGreen, RGBPAD[index].rgbBlue);
            break;
        }
        case 16: { // 16 bits per pixel (RGB565 or XRGB1555)
            UWORD pixel = ((UWORD *)row_data)[col];
            if (bmpInfoHeader->bInfoSize == 0x38) { // RGB565 format
                color = pixel;
            } else if ((bmpInfoHeader->bInfoSize == 0x28) && (bmpInfoHeader->bCompression == 0x00)) { // XRGB1555 format
                color = ((((pixel >> 10) & 0x1F) * 0x1F / 0x1F) << 11) |
                        ((((pixel >> 5) & 0x1F) * 0x3F / 0x1F) << 5) |
                        ((pixel & 0x1F) * 0x1F / 0x1F);
            }
            break;
        }
        case 24: { // 24 bits per pixel (RGB888)
            int byte_offset = col * 3;
            UBYTE blue = row_data[byte_offset];
            UBYTE green = row_data[byte_offset + 1];
            UBYTE red = row_data[byte_offset + 2];
            color = RGB(red, green, blue);
            break;
        }
        case 32: { // 32 bits per pixel (ARGB8888 or XRGB8888)
            int byte_offset = col * 4;
            UBYTE blue = row_data[byte_offset];
            UBYTE green = row_data[byte_offset + 1];
            UBYTE red = row_data[byte_offset + 2];
            // Ignore the Alpha channel, or process it if necessary
            color = RGB(red, green, blue);
            break;
        }
        default:
            printf("Unsupported bBitCount: %d\n", bBitCount);  // Print an error message for unsupported bit depths
            break;
    }
    
    return color;
}

// Function to read and display BMP image from file
UBYTE GUI_ReadBmp(UWORD Xstart, UWORD Ystart, const char *path) {
    FILE *fp;
    
    // Open the BMP file for reading
    if ((fp = fopen(path, "rb")) == NULL) {
        Debug("Cannot open the file: %s\n", path);  // Print error if file can't be opened
        return 0;
    }
    printf("open: %s\n", path);  // Print the file path
    
    // Load the entire BMP file into memory
    fseek(fp, 0, SEEK_END);  // Seek to the end of the file to get the size
    size_t file_size = ftell(fp);  // Get the file size
    fseek(fp, 0, SEEK_SET);  // Seek back to the beginning of the file

    // Allocate memory to store the file content
    UBYTE *file_buffer = malloc(file_size);
    if (!file_buffer) {
        Debug("Memory allocation failed\n");  // Print error if memory allocation fails
        fclose(fp);
        return 0;
    }
    
    // Read the file content into memory
    fread(file_buffer, file_size, 1, fp);
    fclose(fp);  // Close the file after reading

    // Parse BMP headers
    BMPFILEHEADER *bmpFileHeader = (BMPFILEHEADER *)file_buffer;
    BMPINF *bmpInfoHeader = (BMPINF *)(file_buffer + sizeof(BMPFILEHEADER));

    // Compute the starting address of pixel data and the row size
    UBYTE *pixel_data = file_buffer + bmpFileHeader->bOffset;
    int row_bytes = ((bmpInfoHeader->bWidth * bmpInfoHeader->bBitCount + 31) / 32) * 4;

    printf("bBitCount = %d\n", bmpInfoHeader->bBitCount);  // Print the number of bits per pixel

    // Loop through the rows and columns of the image and extract pixel colors
    for (int row = 0; row < bmpInfoHeader->bHeight; row++) {
        UBYTE *row_data = pixel_data + row * row_bytes;
        
        // Loop through each column (pixel) in the row
        for (int col = 0; col < bmpInfoHeader->bWidth; col++) {
            // Extract the pixel color and display it on the screen
            UWORD color = ExtractPixelColor(row_data, col, bmpInfoHeader->bBitCount, bmpInfoHeader);
            Paint_SetPixel(col + Xstart, Ystart + bmpInfoHeader->bHeight - row - 1, color);
        }
    }

    free(file_buffer);  // Free the memory used for the file buffer
    return 1;  // Return success
}
