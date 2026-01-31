/**
 * ALIN 流处理节点: filter_level (日志级别过滤器)
 * 
 * 功能: 按日志级别过滤事件，只保留指定级别及以上的日志
 * 输入: 标准化 ALIN 事件 {"_type":"log","level":"ERROR",...}
 * 输出: 通过过滤的事件 (原样输出) 或空 (被过滤)
 * 
 * 配置: 通过环境变量 ALIN_FILTER_LEVEL 设置 (默认: ERROR)
 *       级别优先级: DEBUG < INFO < WARN < ERROR < FATAL
 * 
 * 热切换: 创建不同配置的版本 (filter_level_error, filter_level_warn 等)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 65536

// 日志级别优先级
int get_level_priority(const char* level) {
    if (strcasecmp(level, "DEBUG") == 0 || strcasecmp(level, "TRACE") == 0) return 0;
    if (strcasecmp(level, "INFO") == 0) return 1;
    if (strcasecmp(level, "WARN") == 0 || strcasecmp(level, "WARNING") == 0) return 2;
    if (strcasecmp(level, "ERROR") == 0 || strcasecmp(level, "ERR") == 0) return 3;
    if (strcasecmp(level, "FATAL") == 0 || strcasecmp(level, "CRITICAL") == 0) return 4;
    if (strcasecmp(level, "RAW") == 0) return 1; // RAW 当作 INFO 处理
    return 1; // 默认 INFO 级别
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
            if (*pos == '\\' && *(pos + 1)) {
                pos++;
            }
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
    char* end;
    while (*start == ' ' || *start == '\n' || *start == '\r' || *start == '\t') start++;
    if (*start == '\0') { str[0] = '\0'; return; }
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t')) end--;
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

int main(int argc, char* argv[]) {
    char input[MAX_INPUT_SIZE];
    char level[256] = "";
    
    // 获取过滤级别配置 (默认 ERROR)
    const char* filter_level_str = getenv("ALIN_FILTER_LEVEL");
    if (!filter_level_str || filter_level_str[0] == '\0') {
        filter_level_str = "ERROR";
    }
    int min_priority = get_level_priority(filter_level_str);
    
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        // 无输入，静默退出
        return 0;
    }
    
    trim(input);
    
    // 空输入
    if (input[0] == '\0') {
        return 0;
    }
    
    // 提取日志级别
    if (!extract_string_field(input, "level", level, sizeof(level))) {
        // 无 level 字段，透传
        printf("%s\n", input);
        return 0;
    }
    
    // 比较级别
    int event_priority = get_level_priority(level);
    
    if (event_priority >= min_priority) {
        // 通过过滤，原样输出
        printf("%s\n", input);
    }
    // 否则静默丢弃
    
    return 0;
}
