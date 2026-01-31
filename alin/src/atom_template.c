/**
 * ALIN 原子节点模板 (Atomic Node Template)
 * 
 * 所有 ALIN 兼容节点必须遵循此模板规范：
 * 1. 输入/输出通过 stdin/stdout 流式传输
 * 2. 使用 JSON 格式进行数据交换
 * 3. 每个节点是无状态的纯函数
 * 
 * 编译: make <node_name>
 * 运行: echo '[1,2,3]' | ./alin/nodes/<node_name>_<hash>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 65536

/**
 * 主处理函数 - 子类需要实现
 * @param input  输入的 JSON 字符串
 * @param output 输出缓冲区
 * @param output_size 输出缓冲区大小
 * @return 0 成功, -1 失败
 */
int process(const char* input, char* output, size_t output_size);

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
    
    // 去除开头空白
    while (*start == ' ' || *start == '\n' || *start == '\r' || *start == '\t') {
        start++;
    }
    
    // 如果全是空白
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }
    
    // 找到结尾
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\n' || *end == '\r' || *end == '\t')) {
        end--;
    }
    
    // 移动内容
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

/**
 * 主入口函数
 */
int main(int argc, char* argv[]) {
    char input[MAX_INPUT_SIZE];
    char output[MAX_INPUT_SIZE];
    
    // 读取输入
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        fprintf(stderr, "Error: No input received\n");
        return 1;
    }
    
    // 去除空白
    trim(input);
    
    // 处理数据
    if (process(input, output, MAX_INPUT_SIZE) != 0) {
        fprintf(stderr, "Error: Processing failed\n");
        return 1;
    }
    
    // 输出结果
    printf("%s\n", output);
    
    return 0;
}
