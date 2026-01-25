#include <stdio.h>
#include <stdlib.h>
#include <png.h>

// http://www.libpng.org/pub/png/book/chapter13.html
// https://gist.github.com/abforce/2a4dbdeb47d4e6bcaf79de38380a13b9
// https://stackoverflow.com/questions/65589714/read-and-write-a-png-file-using-libpng-in-c

void read_png_file(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    // 1. Create and initialize the png_structs
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    // 2. Set up error handling
    if (setjmp(png_jmpbuf(png))) abort(); // Required for libpng error handling

    // 3. Initialize IO
    png_init_io(png, fp);

    // 4. Read file information
    png_read_info(png, info);

    png_uint_32 width      = png_get_image_width(png, info);
    png_uint_32 height     = png_get_image_height(png, info);
    png_byte color_type   = png_get_color_type(png, info);
    png_byte bit_depth    = png_get_bit_depth(png, info);

    // 5. Set up transformations for a consistent format (e.g., 8-bit RGBA)
    // Read any color_type into 8bit depth, RGBA format.
    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info); // Update info based on transforms

    // 6. Allocate memory for image data (row pointers)
    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }

    // 7. Read the image data
    png_read_image(png, row_pointers);

    // y (height)
    png_byte* row = row_pointers[300]; 
    // x (width)
    png_byte* ptr = &(row[400 * 4]); // Assuming 4 channels (RGBA) after transforms
    printf("Pixel at [0, 0] has RGBA values: %d - %d - %d - %d\n", ptr[0], ptr[1], ptr[2], ptr[3]);

    // 9. Clean up
    fclose(fp);
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    png_destroy_read_struct(&png, &info, NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <jpeg_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Compile with: gcc -std=c99 main.c -lpng
    // gcc png.c -lpng -o outer && ./outer dado.png 
    read_png_file(argv[1]);
    return 0;
}
