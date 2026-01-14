/*****************************************************************************
* | File      	:   GUI_BMP.c
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

#ifndef __GUI_BMP_H
#define __GUI_BMP_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "gui_paint.h"

#define RGB(r,g,b) (((r>>3)<<11)|((g>>2)<<5)|(b>>3))  // Macro for converting RGB to 16-bit color format (RGB565)

/****************************** Bitmap standard information *************************************/
/* Bitmap file header (14 bytes) */
typedef struct BMP_FILE_HEADER {
    UWORD bType;                 // File identifier ('BM' for BMP files)
    UDOUBLE bSize;               // Size of the entire BMP file
    UWORD bReserved1;            // Reserved value, should be 0
    UWORD bReserved2;            // Reserved value, should be 0
    UDOUBLE bOffset;             // Offset from the beginning of the file to the image data
} __attribute__ ((packed)) BMPFILEHEADER;    // 14-byte header

/* Bitmap information header (40 bytes) */
typedef struct BMP_INFO {
    UDOUBLE bInfoSize;           // Size of the header (usually 40 bytes)
    UDOUBLE bWidth;              // Width of the image
    UDOUBLE bHeight;             // Height of the image
    UWORD bPlanes;               // Number of color planes (must be 1)
    UWORD bBitCount;             // Bits per pixel (e.g., 24 for RGB)
    UDOUBLE bCompression;        // Compression type (0 for no compression)
    UDOUBLE bmpImageSize;        // Size of the image data (excluding headers)
    UDOUBLE bXPelsPerMeter;      // Horizontal resolution (pixels per meter)
    UDOUBLE bYPelsPerMeter;      // Vertical resolution (pixels per meter)
    UDOUBLE bClrUsed;            // Number of colors used in the image
    UDOUBLE bClrImportant;       // Number of important colors (typically 0)
} __attribute__ ((packed)) BMPINF;

/* Color table entry: a single color in the palette */
typedef struct RGB_QUAD {
    UBYTE rgbBlue;               // Blue intensity (0-255)
    UBYTE rgbGreen;              // Green intensity (0-255)
    UBYTE rgbRed;                // Red intensity (0-255)
    // UBYTE rgbReversed;        // Reserved value (unused in standard BMP)
} __attribute__ ((packed)) RGBQUAD;

/* ARGB format: includes Alpha channel for transparency */
typedef struct ARGB_QUAD {
    UBYTE rgbBlue;               // Blue intensity (0-255)
    UBYTE rgbGreen;              // Green intensity (0-255)
    UBYTE rgbRed;                // Red intensity (0-255)
    UBYTE a;                     // Alpha channel (transparency, 0-255)
} __attribute__ ((packed)) ARGBQUAD;

/**************************************** End of Structures ***********************************************/

/**
 * @brief  Reads and displays a BMP image file on the display starting at the given coordinates.
 *
 * @param Xstart  The X coordinate where the image will be displayed.
 * @param Ystart  The Y coordinate where the image will be displayed.
 * @param path    The file path to the BMP image.
 * @return UBYTE  Returns 1 if successful, 0 if there's an error.
 */
UBYTE GUI_ReadBmp(UWORD Xstart, UWORD Ystart, const char *path);

#endif  // __GUI_BMP_H
