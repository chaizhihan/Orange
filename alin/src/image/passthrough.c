/**
 * ALIN 图像处理节点: passthrough (直通节点)
 * 
 * 功能: 不做任何处理，直接传递数据
 * 用途: 作为管道中的占位符，便于热切换
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 10485760

int main(int argc, char* argv[]) {
    char* buffer = malloc(MAX_INPUT_SIZE);
    if (!buffer) return 1;
    
    size_t total = 0;
    int c;
    while ((c = getchar()) != EOF && total < MAX_INPUT_SIZE - 1) {
        buffer[total++] = (char)c;
    }
    buffer[total] = '\0';
    
    printf("%s", buffer);
    
    free(buffer);
    return 0;
}
