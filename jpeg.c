#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <jerror.h>

// sudo apt install libjpeg-dev
// https://github.com/LuaDist/libjpeg/blob/master/example.c
// https://stackoverflow.com/questions/5616216/need-help-in-reading-jpeg-file-using-libjpeg

typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
} Image;

/* Function to read a JPEG file and return an Image struct */
Image* read_jpeg(const char *filename) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    JSAMPROW row_pointer[1];
    Image *img = NULL;

    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", filename);
        return NULL;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);

    /* Set parameters for decompression */
    cinfo.out_color_space = JCS_RGB; /* Request RGB output color space */
    // cinfo.dct_method = JDCT_ISLOW; /* Use the slow but accurate method */

    jpeg_start_decompress(&cinfo);

    /* Allocate memory for the image data structure and pixel buffer */
    img = (Image*)malloc(sizeof(Image));
    if (img == NULL) {
        fprintf(stderr, "Memory error for Image struct!\n");
        jpeg_finish_decompress(&cinfo);
        fclose(infile);
        return NULL;
    }

    img->width = cinfo.output_width;
    img->height = cinfo.output_height;
    img->channels = cinfo.output_components;
    long raw_image_size = img->width * img->height * img->channels;
    img->data = (unsigned char*)malloc(raw_image_size);

    if (img->data == NULL) {
        fprintf(stderr, "Memory error for image data!\n");
        free(img);
        jpeg_finish_decompress(&cinfo);
        fclose(infile);
        return NULL;
    }

    /* Read scanlines */
    // Explicação 1 
    while (cinfo.output_scanline < cinfo.output_height) {
        // Aloca o ponteiro do buffer para a localização da memória do struct
        // Dai ele vai ler a próxima linha da imagem e já vai direto para a struct
        row_pointer[0] = &(img->data[cinfo.output_scanline * img->width * img->channels]); // &(struct->data) == mudando o ponteiro para a struct
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    fclose(infile);
    jpeg_destroy_decompress(&cinfo);

    return img;
}

/* Main function to demonstrate usage */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <jpeg_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Image *image = read_jpeg(argv[1]);

    // gcc jpeg.c -ljpeg -o outer && ./outer land.jpg

    if (image != NULL) {
        printf("Successfully read image: %s\n", argv[1]);
        printf("Dimensions: %d x %d pixels\n", image->width, image->height);
        printf("Channels: %d\n", image->channels);

        /* Example of accessing a pixel at (x, y) */
        // Explicação 2
        int x = 1080, y = 1080;

        // Faz um slice do array, começando pelo pixel que se quer ler 
        // eg: data[pixel_loc:] em python (que desgraçado)
        unsigned char *pixel = &(image->data[(y * image->width + x) * image->channels]);
        printf("Pixel at (%d, %d): R=%u, G=%u, B=%u\n", x, y, pixel[0], pixel[1], pixel[2]);

        /* Free the allocated memory */
        free(image->data);
        free(image);
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
