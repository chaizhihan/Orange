/**
 * ALIN 原子节点: Double (数值翻倍)
 * 
 * 功能: 将输入 JSON 数组中的每个数值翻倍
 * 输入: JSON 数组, 例如 [1, 2, 3]
 * 输出: JSON 数组, 例如 [2, 4, 6]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 65536
#define MAX_NUMBERS 1024

/**
 * 从 stdin 读取全部输入
 */
int read_stdin(char* buffer, size_t max_size) {
    size_t total = 0;
    int c;
    while ((c = getchar()) != EOF && total < max_size - 1) {
        buffer[total++] = (char)c;
    }
    buffer[total] = '\0';
    return (int)total;
}

/**
 * 去除字符串首尾空白
 */
void trim(char* str) {
    char* start = str;
    char* end;
    
    while (*start == ' ' || *start == '\n' || *start == '\r' || *start == '\t') {
        start++;
    }
    
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }
    
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t')) {
        end--;
    }
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

/**
 * 简单解析 JSON 数组中的数字
 */
int parse_json_array(const char* json, double* numbers, int max_count) {
    int count = 0;
    const char* p = json;
    
    // 跳过开头的 [
    while (*p && *p != '[') p++;
    if (*p == '[') p++;
    
    while (*p && *p != ']' && count < max_count) {
        // 跳过空白和逗号
        while (*p && (*p == ' ' || *p == ',' || *p == '\n' || *p == '\r' || *p == '\t')) {
            p++;
        }
        
        if (*p == ']' || *p == '\0') break;
        
        // 解析数字
        char* end;
        double num = strtod(p, &end);
        if (end != p) {
            numbers[count++] = num;
            p = end;
        } else {
            p++;
        }
    }
    
    return count;
}

/**
 * 主处理函数: 数值翻倍
 * 支持数组输入: [1, 2, 3] -> [2, 4, 6]
 * 支持单值输入: 6 -> 12
 */
int process(const char* input, char* output, size_t output_size) {
    double numbers[MAX_NUMBERS];
    int is_array = 0;
    
    // 检测是否为数组格式
    const char* p = input;
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    
    if (*p == '[') {
        is_array = 1;
        int count = parse_json_array(input, numbers, MAX_NUMBERS);
        
        if (count == 0) {
            strcpy(output, "[]");
            return 0;
        }
        
        // 翻倍处理
        for (int i = 0; i < count; i++) {
            numbers[i] *= 2;
        }
        
        // 生成输出 JSON 数组
        char* out = output;
        out += sprintf(out, "[");
        for (int i = 0; i < count; i++) {
            if (i > 0) out += sprintf(out, ", ");
            if (numbers[i] == (int)numbers[i]) {
                out += sprintf(out, "%d", (int)numbers[i]);
            } else {
                out += sprintf(out, "%.6g", numbers[i]);
            }
        }
        sprintf(out, "]");
    } else {
        // 单个数值
        char* end;
        double num = strtod(input, &end);
        num *= 2;
        
        if (num == (int)num) {
            sprintf(output, "%d", (int)num);
        } else {
            sprintf(output, "%.6g", num);
        }
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    char input[MAX_INPUT_SIZE];
    char output[MAX_INPUT_SIZE];
    
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        fprintf(stderr, "Error: No input received\n");
        return 1;
    }
    
    trim(input);
    
    if (process(input, output, MAX_INPUT_SIZE) != 0) {
        fprintf(stderr, "Error: Processing failed\n");
        return 1;
    }
    
    printf("%s\n", output);
    
    return 0;
}
