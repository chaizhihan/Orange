/**
 * ALIN 流处理节点: alert_console (控制台告警输出)
 * 
 * 功能: 将事件格式化为人类可读的告警信息输出到控制台
 * 输入: 带聚合信息的 ALIN 事件
 * 输出: 格式化的告警文本
 * 
 * 配置: 
 * - ALIN_ALERT_THRESHOLD: 触发告警的阈值 (默认: 0 = 每条都告警)
 * - ALIN_ALERT_FORMAT: 输出格式 (text/json, 默认: text)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT_SIZE 65536

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

long extract_number_field(const char* json, const char* field) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    
    const char* pos = strstr(json, pattern);
    if (!pos) return 0;
    
    pos += strlen(pattern);
    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t')) pos++;
    
    if (*pos == '-' || (*pos >= '0' && *pos <= '9')) {
        return strtol(pos, NULL, 10);
    }
    return 0;
}

double extract_double_field(const char* json, const char* field) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    
    const char* pos = strstr(json, pattern);
    if (!pos) return 0;
    
    pos += strlen(pattern);
    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t')) pos++;
    
    if (*pos == '-' || *pos == '.' || (*pos >= '0' && *pos <= '9')) {
        return strtod(pos, NULL);
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
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

const char* get_level_color(const char* level) {
    if (strcasecmp(level, "ERROR") == 0 || strcasecmp(level, "FATAL") == 0) return "\033[0;31m";  // Red
    if (strcasecmp(level, "WARN") == 0 || strcasecmp(level, "WARNING") == 0) return "\033[0;33m";  // Yellow
    if (strcasecmp(level, "INFO") == 0) return "\033[0;32m";  // Green
    if (strcasecmp(level, "DEBUG") == 0) return "\033[0;36m";  // Cyan
    return "\033[0m";  // Default
}

const char* get_level_icon(const char* level) {
    if (strcasecmp(level, "ERROR") == 0) return "🔴";
    if (strcasecmp(level, "FATAL") == 0 || strcasecmp(level, "CRITICAL") == 0) return "💀";
    if (strcasecmp(level, "WARN") == 0 || strcasecmp(level, "WARNING") == 0) return "🟡";
    if (strcasecmp(level, "INFO") == 0) return "🟢";
    if (strcasecmp(level, "DEBUG") == 0) return "🔵";
    return "⚪";
}

int main(int argc, char* argv[]) {
    char input[MAX_INPUT_SIZE];
    char level[64] = "INFO";
    char message[4096] = "";
    
    // 获取配置
    const char* threshold_str = getenv("ALIN_ALERT_THRESHOLD");
    long threshold = threshold_str ? atol(threshold_str) : 0;
    
    const char* format = getenv("ALIN_ALERT_FORMAT");
    int json_format = (format && strcasecmp(format, "json") == 0);
    
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        return 0;
    }
    
    trim(input);
    
    if (input[0] == '\0') {
        return 0;
    }
    
    // 提取字段
    extract_string_field(input, "level", level, sizeof(level));
    extract_string_field(input, "message", message, sizeof(message));
    
    long total = extract_number_field(input, "total");
    double rate = extract_double_field(input, "rate");
    long timestamp = extract_number_field(input, "timestamp");
    
    // 检查阈值
    if (threshold > 0 && total < threshold) {
        // 未达阈值，静默
        printf("%s\n", input);
        return 0;
    }
    
    // 格式化时间
    time_t ts = timestamp > 0 ? (time_t)timestamp : time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&ts));
    
    if (json_format) {
        // JSON 格式输出
        printf("{\"alert\":true,\"time\":\"%s\",\"level\":\"%s\",\"message\":\"%s\",\"total\":%ld,\"rate\":%.2f}\n",
            time_str, level, message, total, rate);
    } else {
        // 人类可读格式
        const char* color = get_level_color(level);
        const char* icon = get_level_icon(level);
        const char* reset = "\033[0m";
        
        fprintf(stderr, "\n");
        fprintf(stderr, "╔══════════════════════════════════════════════════════════╗\n");
        fprintf(stderr, "║ %s ALIN ALERT %s%-44s ║\n", icon, color, level);
        fprintf(stderr, "╠══════════════════════════════════════════════════════════╣\n");
        fprintf(stderr, "║ 🕐 Time:    %-46s ║\n", time_str);
        fprintf(stderr, "║ 📝 Message: %-46.46s ║\n", message[0] ? message : "(no message)");
        fprintf(stderr, "║ 📊 Count:   %-6ld  Rate: %-6.2f events/sec            ║\n", total, rate);
        fprintf(stderr, "╚══════════════════════════════════════════════════════════╝%s\n", reset);
        fprintf(stderr, "\n");
        
        // 同时输出原始 JSON 到 stdout (保持管道链)
        printf("%s\n", input);
    }
    
    return 0;
}
