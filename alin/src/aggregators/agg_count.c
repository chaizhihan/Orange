/**
 * ALIN 流处理节点: agg_count (事件计数聚合器)
 * 
 * 功能: 累积计数通过的事件，维护状态
 * 输入: 标准化 ALIN 事件
 * 输出: 事件 + 累积计数信息 {"...原事件...", "_count": N, "_count_by_level": {...}}
 * 
 * 状态: 使用文件持久化计数 (ALIN_STATE_FILE 环境变量)
 * 
 * 统计维度:
 * - 总事件数
 * - 按 level 分组计数
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_INPUT_SIZE 65536
#define MAX_LEVELS 16
#define MAX_PATH 1024

typedef struct {
    char level[64];
    long count;
} LevelCount;

typedef struct {
    long total_count;
    long session_start;
    int level_count;
    LevelCount levels[MAX_LEVELS];
} AggState;

// 全局状态
AggState state = {0};
char state_file[MAX_PATH] = "";

void load_state() {
    if (state_file[0] == '\0') return;
    
    FILE* f = fopen(state_file, "r");
    if (!f) {
        state.session_start = (long)time(NULL);
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64], value[64];
        if (sscanf(line, "%63[^=]=%63s", key, value) == 2) {
            if (strcmp(key, "total") == 0) {
                state.total_count = atol(value);
            } else if (strcmp(key, "session_start") == 0) {
                state.session_start = atol(value);
            } else if (strncmp(key, "level_", 6) == 0 && state.level_count < MAX_LEVELS) {
                strncpy(state.levels[state.level_count].level, key + 6, 63);
                state.levels[state.level_count].count = atol(value);
                state.level_count++;
            }
        }
    }
    fclose(f);
    
    if (state.session_start == 0) {
        state.session_start = (long)time(NULL);
    }
}

void save_state() {
    if (state_file[0] == '\0') return;
    
    FILE* f = fopen(state_file, "w");
    if (!f) return;
    
    fprintf(f, "total=%ld\n", state.total_count);
    fprintf(f, "session_start=%ld\n", state.session_start);
    for (int i = 0; i < state.level_count; i++) {
        fprintf(f, "level_%s=%ld\n", state.levels[i].level, state.levels[i].count);
    }
    fclose(f);
}

int find_or_create_level(const char* level) {
    for (int i = 0; i < state.level_count; i++) {
        if (strcasecmp(state.levels[i].level, level) == 0) {
            return i;
        }
    }
    if (state.level_count < MAX_LEVELS) {
        strncpy(state.levels[state.level_count].level, level, 63);
        state.levels[state.level_count].count = 0;
        return state.level_count++;
    }
    return -1;
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
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

int main(int argc, char* argv[]) {
    char input[MAX_INPUT_SIZE];
    char output[MAX_INPUT_SIZE * 2];
    char level[64] = "UNKNOWN";
    
    // 获取状态文件路径
    const char* state_path = getenv("ALIN_STATE_FILE");
    if (state_path && state_path[0] != '\0') {
        strncpy(state_file, state_path, MAX_PATH - 1);
    }
    
    // 加载现有状态
    load_state();
    
    if (read_stdin(input, MAX_INPUT_SIZE) <= 0) {
        return 0;
    }
    
    trim(input);
    
    if (input[0] == '\0') {
        return 0;
    }
    
    // 提取 level
    extract_string_field(input, "level", level, sizeof(level));
    
    // 更新计数
    state.total_count++;
    int level_idx = find_or_create_level(level);
    if (level_idx >= 0) {
        state.levels[level_idx].count++;
    }
    
    // 保存状态
    save_state();
    
    // 构建输出: 在原 JSON 基础上添加统计信息
    // 找到最后一个 }
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '}') {
        input[len - 1] = '\0';
        
        // 构建 level 统计 JSON
        char level_stats[4096] = "{";
        for (int i = 0; i < state.level_count; i++) {
            char entry[128];
            snprintf(entry, sizeof(entry), "%s\"%s\":%ld",
                i > 0 ? "," : "",
                state.levels[i].level,
                state.levels[i].count);
            strcat(level_stats, entry);
        }
        strcat(level_stats, "}");
        
        long duration = (long)time(NULL) - state.session_start;
        double rate = duration > 0 ? (double)state.total_count / duration : 0;
        
        snprintf(output, sizeof(output),
            "%s,\"_agg\":{\"total\":%ld,\"rate\":%.2f,\"by_level\":%s}}",
            input, state.total_count, rate, level_stats);
    } else {
        strcpy(output, input);
    }
    
    printf("%s\n", output);
    
    return 0;
}
