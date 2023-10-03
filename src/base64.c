#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "base64.h"

static char symbols[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t encode_base64(const char *const filename, char *output, const size_t len)
{
    unsigned char *input = calloc(len, sizeof(unsigned char));

    FILE *f = fopen(filename, "rb");
    size_t input_len = fread(input, sizeof(char), len - 1, f);
    input[input_len] = '\0';
    fclose(f);

    uint32_t triple;
    uint32_t mask_1 = 0b000000111111000000000000;
    uint32_t mask_2 = 0b000000000000111111000000;
    uint32_t mask_3 = 0b000000000000000000111111;
    
    size_t i = 0, j = 0;
    for (i = 0; i < input_len - 2; i += 3)
    {
        triple = (input[i] << 16) + (input[i + 1] << 8) + (input[i + 2]);
    
        output[j++] = symbols[triple >> 18];
        output[j++] = symbols[(triple & mask_1) >> 12];
        output[j++] = symbols[(triple & mask_2) >> 6];
        output[j++] = symbols[triple & mask_3];
    }

    if (input_len % 3 == 1)
    {
        triple = (input[i] << 16);
    
        output[j++] = symbols[triple >> 18];
        output[j++] = symbols[(triple & mask_1) >> 12];
        output[j++] = '=';
        output[j++] = '=';
    }
    else if (input_len % 3 == 2)
    {
        triple = (input[i] << 16) + (input[i + 1] << 8);
    
        output[j++] = symbols[triple >> 18];
        output[j++] = symbols[(triple & mask_1) >> 12];
        output[j++] = symbols[(triple & mask_2) >> 6];
        output[j++] = '=';
    }
    
    output[j] = '\0';

    free(input);

    return j;
}
