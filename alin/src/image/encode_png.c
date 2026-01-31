/**
 * ALIN 图像处理节点: encode_png (PNG 编码器)
 * 
 * 功能: 将 PPM 格式图像编码为 PNG 并保存
 * 输入: JSON {"_type":"image", "ppm":"<base64>", "output":"/path/to/output.png"}
 * 输出: JSON {"_type":"result", "success":true, "path":"/path/to/output.png"}
 * 
 * 使用系统工具进行转换 (sips/convert)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 10485760
#define MAX_PATH 4096

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

int extract_string_field(const char* json, const char* field, char* value, size_t max_size) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    const char* pos = strstr(json, pattern);
    if (!pos) return 0;
    pos += strlen(pattern);
    while (*pos && (*pos == ':' || *pos == ' ')) pos++;
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

int convert_ppm_to_png(const char* ppm_path, const char* png_path) {
    char cmd[MAX_PATH * 3];
    
    // macOS sips
    snprintf(cmd, sizeof(cmd), "sips -s format png \"%s\" --out \"%s\" 2>/dev/null", ppm_path, png_path);
    if (system(cmd) == 0) return 0;
    
    // ImageMagick
    snprintf(cmd, sizeof(cmd), "convert \"%s\" \"%s\" 2>/dev/null", ppm_path, png_path);
    return system(cmd);
}

int main(int argc, char* argv[]) {
    char* input = malloc(MAX_INPUT_SIZE);
    if (!input) return 1;
    
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        free(input);
        fprintf(stderr, "Error: No input\n");
        return 1;
    }
    
    // 提取输出路径
    char output_path[MAX_PATH] = "";
    if (!extract_string_field(input, "output", output_path, MAX_PATH)) {
        // 默认输出路径
        snprintf(output_path, MAX_PATH, "/tmp/alin_output_%d.png", getpid());
    }
    
    // 提取 PPM 数据
    char* ppm_b64 = extract_ppm_field(input);
    if (!ppm_b64) {
        free(input);
        fprintf(stderr, "Error: No ppm field\n");
        return 1;
    }
    
    // 解码 PPM
    size_t ppm_max = strlen(ppm_b64);
    unsigned char* ppm_data = malloc(ppm_max);
    size_t ppm_len = base64_decode(ppm_b64, ppm_data, ppm_max);
    free(ppm_b64);
    
    // 写入临时 PPM 文件
    char tmp_ppm[MAX_PATH];
    snprintf(tmp_ppm, sizeof(tmp_ppm), "/tmp/alin_encode_%d.ppm", getpid());
    
    FILE* f = fopen(tmp_ppm, "wb");
    if (!f) {
        free(input);
        free(ppm_data);
        return 1;
    }
    fwrite(ppm_data, 1, ppm_len, f);
    fclose(f);
    free(ppm_data);
    
    // 转换为 PNG
    int success = (convert_ppm_to_png(tmp_ppm, output_path) == 0);
    
    // 清理
    unlink(tmp_ppm);
    free(input);
    
    // 输出结果
    printf("{\"_type\":\"result\",\"success\":%s,\"path\":\"%s\"}\n",
        success ? "true" : "false", output_path);
    
    return success ? 0 : 1;
}
