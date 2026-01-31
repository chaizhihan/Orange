/**
 * ALIN 图像处理节点: filter_sepia (复古滤镜)
 * 
 * 功能: 将图像转换为复古棕褐色调
 * 输入: JSON {"_type":"image", "width":N, "height":M, "ppm":"<base64>"}
 * 输出: 相同格式，但应用了复古色调
 * 
 * 算法:
 *   newR = 0.393*R + 0.769*G + 0.189*B
 *   newG = 0.349*R + 0.686*G + 0.168*B
 *   newB = 0.272*R + 0.534*G + 0.131*B
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 10485760

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t base64_encode(const unsigned char* input, size_t input_len, char* output, size_t output_max) {
    size_t i, j;
    for (i = 0, j = 0; i < input_len && j < output_max - 4;) {
        uint32_t octet_a = i < input_len ? input[i++] : 0;
        uint32_t octet_b = i < input_len ? input[i++] : 0;
        uint32_t octet_c = i < input_len ? input[i++] : 0;
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        output[j++] = base64_table[(triple >> 18) & 0x3F];
        output[j++] = base64_table[(triple >> 12) & 0x3F];
        output[j++] = (i > input_len + 1) ? '=' : base64_table[(triple >> 6) & 0x3F];
        output[j++] = (i > input_len) ? '=' : base64_table[triple & 0x3F];
    }
    output[j] = '\0';
    return j;
}

size_t base64_decode(const char* input, unsigned char* output, size_t output_max) {
    static const unsigned char decode_table[256] = {
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
        64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
        64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64};
    size_t input_len = strlen(input);
    size_t out_len = 0;
    for (size_t i = 0; i < input_len && out_len < output_max;) {
        uint32_t sextet_a = input[i] == '=' ? 0 : decode_table[(unsigned char)input[i]]; i++;
        uint32_t sextet_b = input[i] == '=' ? 0 : decode_table[(unsigned char)input[i]]; i++;
        uint32_t sextet_c = input[i] == '=' ? 0 : decode_table[(unsigned char)input[i]]; i++;
        uint32_t sextet_d = input[i] == '=' ? 0 : decode_table[(unsigned char)input[i]]; i++;
        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        if (out_len < output_max) output[out_len++] = (triple >> 16) & 0xFF;
        if (out_len < output_max && input[i-2] != '=') output[out_len++] = (triple >> 8) & 0xFF;
        if (out_len < output_max && input[i-1] != '=') output[out_len++] = triple & 0xFF;
    }
    return out_len;
}

long extract_number_field(const char* json, const char* field) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    const char* pos = strstr(json, pattern);
    if (!pos) return 0;
    pos += strlen(pattern);
    while (*pos && (*pos == ':' || *pos == ' ')) pos++;
    return strtol(pos, NULL, 10);
}

char* extract_ppm_field(const char* json) {
    const char* start = strstr(json, "\"ppm\":\"");
    if (!start) return NULL;
    start += 7;
    const char* end = start;
    while (*end && *end != '"') end++;
    size_t len = end - start;
    char* result = malloc(len + 1);
    if (result) { memcpy(result, start, len); result[len] = '\0'; }
    return result;
}

int read_stdin(char* buffer, size_t max_size) {
    size_t total = 0; int c;
    while ((c = getchar()) != EOF && total < max_size - 1) buffer[total++] = (char)c;
    buffer[total] = '\0';
    return (int)total;
}

static inline unsigned char clamp(int v) {
    return v < 0 ? 0 : (v > 255 ? 255 : v);
}

void apply_sepia(unsigned char* ppm_data, size_t data_len) {
    size_t pos = 0;
    int newlines = 0;
    while (pos < data_len && newlines < 3) {
        if (ppm_data[pos] == '\n') newlines++;
        if (ppm_data[pos] == '#') while (pos < data_len && ppm_data[pos] != '\n') pos++;
        pos++;
    }
    
    for (size_t i = pos; i + 2 < data_len; i += 3) {
        unsigned char r = ppm_data[i];
        unsigned char g = ppm_data[i + 1];
        unsigned char b = ppm_data[i + 2];
        
        int newR = (int)(0.393 * r + 0.769 * g + 0.189 * b);
        int newG = (int)(0.349 * r + 0.686 * g + 0.168 * b);
        int newB = (int)(0.272 * r + 0.534 * g + 0.131 * b);
        
        ppm_data[i] = clamp(newR);
        ppm_data[i + 1] = clamp(newG);
        ppm_data[i + 2] = clamp(newB);
    }
}

int main(int argc, char* argv[]) {
    char* input = malloc(MAX_INPUT_SIZE);
    if (!input) return 1;
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) { free(input); return 1; }
    
    int width = (int)extract_number_field(input, "width");
    int height = (int)extract_number_field(input, "height");
    char* ppm_b64 = extract_ppm_field(input);
    if (!ppm_b64) { free(input); return 1; }
    
    size_t ppm_max = strlen(ppm_b64);
    unsigned char* ppm_data = malloc(ppm_max);
    size_t ppm_len = base64_decode(ppm_b64, ppm_data, ppm_max);
    free(ppm_b64);
    
    apply_sepia(ppm_data, ppm_len);
    
    size_t b64_size = (ppm_len + 2) / 3 * 4 + 1;
    char* b64_out = malloc(b64_size);
    base64_encode(ppm_data, ppm_len, b64_out, b64_size);
    
    printf("{\"_type\":\"image\",\"width\":%d,\"height\":%d,\"format\":\"ppm\",\"filter\":\"sepia\",\"ppm\":\"%s\"}\n",
        width, height, b64_out);
    
    free(input); free(ppm_data); free(b64_out);
    return 0;
}
