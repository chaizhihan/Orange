/**
 * ALIN 图像处理节点: decode_image (图像解码器)
 * 
 * 功能: 解码 JPEG/PNG 图像为 PPM 格式
 * 输入: JSON {"path": "/path/to/image"} 或 {"data": "<base64>"}
 * 输出: JSON {"width": N, "height": M, "ppm": "<base64 PPM data>"}
 * 
 * 由于避免外部依赖，使用系统工具进行转换：
 * - macOS: sips 命令
 * - Linux: ImageMagick convert
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1048576  // 1MB
#define MAX_PATH 4096

// Base64 编码表
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64 编码
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

// Base64 解码
size_t base64_decode(const char* input, unsigned char* output, size_t output_max) {
    static const unsigned char decode_table[256] = {
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
        52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
        64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
        64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64};
    
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

int extract_string_field(const char* json, const char* field, char* value, size_t max_size) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    const char* pos = strstr(json, pattern);
    if (!pos) return 0;
    pos += strlen(pattern);
    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t')) pos++;
    if (*pos == '"') {
        pos++;
        size_t i = 0;
        while (*pos && *pos != '"' && i < max_size - 1) {
            if (*pos == '\\' && *(pos + 1)) pos++;
            value[i++] = *pos++;
        }
        value[i] = '\0';
        return 1;
    }
    return 0;
}

int read_stdin(char* buffer, size_t max_size) {
    size_t total = 0;
    int c;
    while ((c = getchar()) != EOF && total < max_size - 1) {
        buffer[total++] = (char)c;
    }
    buffer[total] = '\0';
    return (int)total;
}

void trim(char* str) {
    char* start = str;
    while (*start == ' ' || *start == '\n' || *start == '\r' || *start == '\t') start++;
    if (*start == '\0') { str[0] = '\0'; return; }
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t')) end--;
    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

// 使用 sips (macOS) 或 convert (Linux) 转换图像为 PPM
int convert_to_ppm(const char* input_path, const char* output_path) {
    char cmd[MAX_PATH * 3];
    
    // 尝试 sips (macOS)
    snprintf(cmd, sizeof(cmd), 
        "sips -s format ppm \"%s\" --out \"%s\" 2>/dev/null", 
        input_path, output_path);
    
    if (system(cmd) == 0) {
        return 0;
    }
    
    // 尝试 ImageMagick convert
    snprintf(cmd, sizeof(cmd),
        "convert \"%s\" \"%s\" 2>/dev/null",
        input_path, output_path);
    
    return system(cmd);
}

// 读取 PPM 文件的尺寸
int get_ppm_dimensions(const char* ppm_path, int* width, int* height) {
    FILE* f = fopen(ppm_path, "rb");
    if (!f) return -1;
    
    char magic[3];
    if (fscanf(f, "%2s", magic) != 1) { fclose(f); return -1; }
    
    // 跳过注释
    int c;
    while ((c = fgetc(f)) == '#') {
        while ((c = fgetc(f)) != '\n' && c != EOF);
    }
    ungetc(c, f);
    
    if (fscanf(f, "%d %d", width, height) != 2) { fclose(f); return -1; }
    
    fclose(f);
    return 0;
}

int main(int argc, char* argv[]) {
    char input[MAX_INPUT_SIZE];
    char path[MAX_PATH] = "";
    char tmp_ppm[MAX_PATH];
    
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        fprintf(stderr, "Error: No input received\n");
        return 1;
    }
    
    trim(input);
    
    // 提取路径
    if (!extract_string_field(input, "path", path, MAX_PATH)) {
        fprintf(stderr, "Error: No 'path' field in input\n");
        return 1;
    }
    
    // 检查文件是否存在
    if (access(path, F_OK) != 0) {
        fprintf(stderr, "Error: File not found: %s\n", path);
        return 1;
    }
    
    // 创建临时 PPM 文件
    snprintf(tmp_ppm, sizeof(tmp_ppm), "/tmp/alin_decode_%d.ppm", getpid());
    
    // 转换为 PPM
    if (convert_to_ppm(path, tmp_ppm) != 0) {
        fprintf(stderr, "Error: Failed to convert image\n");
        return 1;
    }
    
    // 获取尺寸
    int width = 0, height = 0;
    get_ppm_dimensions(tmp_ppm, &width, &height);
    
    // 读取 PPM 文件
    FILE* f = fopen(tmp_ppm, "rb");
    if (!f) {
        fprintf(stderr, "Error: Failed to read PPM\n");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long ppm_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned char* ppm_data = malloc(ppm_size);
    if (!ppm_data) {
        fclose(f);
        return 1;
    }
    
    fread(ppm_data, 1, ppm_size, f);
    fclose(f);
    
    // Base64 编码
    size_t b64_size = (ppm_size + 2) / 3 * 4 + 1;
    char* b64_data = malloc(b64_size);
    if (!b64_data) {
        free(ppm_data);
        return 1;
    }
    
    base64_encode(ppm_data, ppm_size, b64_data, b64_size);
    
    // 输出 JSON
    printf("{\"_type\":\"image\",\"width\":%d,\"height\":%d,\"format\":\"ppm\",\"ppm\":\"%s\"}\n",
        width, height, b64_data);
    
    // 清理
    free(ppm_data);
    free(b64_data);
    unlink(tmp_ppm);
    
    return 0;
}
