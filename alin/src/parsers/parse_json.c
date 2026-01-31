/**
 * ALIN 流处理节点: parse_json (JSON 日志解析器)
 * 
 * 功能: 解析 JSON 格式的日志行，提取关键字段
 * 输入: 原始 JSON 日志行 {"level":"ERROR","msg":"something failed","ts":1234567890}
 * 输出: 标准化的 ALIN 事件格式
 * 
 * 标准化输出格式:
 * {"_type":"log","level":"ERROR","message":"...", "timestamp":..., "_raw":{原始数据}}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_INPUT_SIZE 65536
#define MAX_FIELD_SIZE 4096

// 简单的 JSON 字段提取 (不依赖外部库)
int extract_string_field(const char* json, const char* field, char* value, size_t max_size) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    
    const char* pos = strstr(json, pattern);
    if (!pos) return 0;
    
    // 跳过字段名和冒号
    pos += strlen(pattern);
    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t')) pos++;
    
    if (*pos == '"') {
        pos++;
        size_t i = 0;
        while (*pos && *pos != '"' && i < max_size - 1) {
            if (*pos == '\\' && *(pos + 1)) {
                pos++;
                switch (*pos) {
                    case 'n': value[i++] = '\n'; break;
                    case 't': value[i++] = '\t'; break;
                    case 'r': value[i++] = '\r'; break;
                    case '"': value[i++] = '"'; break;
                    case '\\': value[i++] = '\\'; break;
                    default: value[i++] = *pos;
                }
            } else {
                value[i++] = *pos;
            }
            pos++;
        }
        value[i] = '\0';
        return 1;
    }
    return 0;
}

long extract_number_field(const char* json, const char* field) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    
    const char* pos = strstr(json, pattern);
    if (!pos) return 0;
    
    pos += strlen(pattern);
    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t')) pos++;
    
    if (*pos == '-' || isdigit(*pos)) {
        return strtol(pos, NULL, 10);
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
    char* end;
    while (*start == ' ' || *start == '\n' || *start == '\r' || *start == '\t') start++;
    if (*start == '\0') { str[0] = '\0'; return; }
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t')) end--;
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

// 转义 JSON 字符串
void json_escape(const char* src, char* dst, size_t max_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j < max_size - 2; i++) {
        switch (src[i]) {
            case '"':  dst[j++] = '\\'; dst[j++] = '"'; break;
            case '\\': dst[j++] = '\\'; dst[j++] = '\\'; break;
            case '\n': dst[j++] = '\\'; dst[j++] = 'n'; break;
            case '\r': dst[j++] = '\\'; dst[j++] = 'r'; break;
            case '\t': dst[j++] = '\\'; dst[j++] = 't'; break;
            default:   dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

int main(int argc, char* argv[]) {
    char input[MAX_INPUT_SIZE];
    char output[MAX_INPUT_SIZE];
    char level[MAX_FIELD_SIZE] = "INFO";
    char message[MAX_FIELD_SIZE] = "";
    char msg[MAX_FIELD_SIZE] = "";
    char escaped_msg[MAX_FIELD_SIZE * 2] = "";
    char escaped_raw[MAX_INPUT_SIZE * 2] = "";
    long timestamp = 0;
    
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        fprintf(stderr, "Error: No input received\n");
        return 1;
    }
    
    trim(input);
    
    // 检查是否为 JSON (以 { 开头)
    if (input[0] != '{') {
        // 非 JSON，包装为原始消息
        json_escape(input, escaped_msg, sizeof(escaped_msg));
        snprintf(output, MAX_INPUT_SIZE,
            "{\"_type\":\"log\",\"level\":\"RAW\",\"message\":\"%s\",\"timestamp\":%ld}",
            escaped_msg, (long)time(NULL));
        printf("%s\n", output);
        return 0;
    }
    
    // 提取字段
    extract_string_field(input, "level", level, MAX_FIELD_SIZE);
    
    // 尝试多种消息字段名
    if (!extract_string_field(input, "message", message, MAX_FIELD_SIZE)) {
        if (!extract_string_field(input, "msg", message, MAX_FIELD_SIZE)) {
            extract_string_field(input, "error", message, MAX_FIELD_SIZE);
        }
    }
    
    // 尝试多种时间戳字段名
    timestamp = extract_number_field(input, "timestamp");
    if (timestamp == 0) {
        timestamp = extract_number_field(input, "ts");
    }
    if (timestamp == 0) {
        timestamp = extract_number_field(input, "time");
    }
    if (timestamp == 0) {
        timestamp = (long)time(NULL);
    }
    
    // 转义消息和原始输入
    json_escape(message, escaped_msg, sizeof(escaped_msg));
    json_escape(input, escaped_raw, sizeof(escaped_raw));
    
    // 转换 level 为大写
    for (int i = 0; level[i]; i++) {
        level[i] = toupper(level[i]);
    }
    
    // 生成标准化输出
    snprintf(output, MAX_INPUT_SIZE,
        "{\"_type\":\"log\",\"level\":\"%s\",\"message\":\"%s\",\"timestamp\":%ld,\"_raw\":\"%s\"}",
        level, escaped_msg, timestamp, escaped_raw);
    
    printf("%s\n", output);
    
    return 0;
}
