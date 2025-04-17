#include "detectors.h"
#include "../utils/property/custom_system_properties.h"
#include "../utils//logging.h"

char *property_get_by_popen(const char *prop_name) {
    FILE *pipe;
    char command[256];
    static char output[128];
    snprintf(command, sizeof(command), "/system/bin/getprop %s", prop_name);
    pipe = popen(command, "r");
    if (!pipe) {
        return strdup("Error: Failed to execute getprop");
    }
    if (fgets(output, sizeof(output), pipe) == NULL) {
        pclose(pipe);
        return strdup("Error: No output");
    }
    output[strcspn(output, "\n\r")] = '\0';
    pclose(pipe);
    return output;
}

char *property_get_by_system_get(const char *key) {
    char buffer[PROP_VALUE_MAX] = {0};
    int flag = __system_property_get(key, buffer);
    if (flag < 0) {
        return strdup("Error: __system_property_get error");
    }
    return strdup(buffer);
}

void (*property_read_callback)(
        const prop_info *pi,
        void (*callback)(void *cookie, const char *name, const char *value, uint32_t serial),
        void *cookie);

void *libc;

void get_from_libc(const char *fn_name, void *fn_ptr) {
    void **fn_ptr_ptr = static_cast<void **>(fn_ptr);
    if (*fn_ptr_ptr == nullptr) {
        if (!libc) {
            libc = dlopen("libc.so", RTLD_LAZY);
            if (libc == nullptr) return;
        }
        *fn_ptr_ptr = dlsym(libc, fn_name);
    }
}

void handle_property(void *cookie, const char *name, const char *value, uint32_t  __unused serial) {
    auto *context = static_cast<ReadCallbackContext *>(cookie);
    strncpy(context->value, value, PROP_VALUE_MAX - 1);
    context->value[PROP_VALUE_MAX - 1] = '\0'; // 确保终止
    context->found = true;
}

char *property_get_by_system_readcallback(const char *key) {
    const prop_info *pi = __system_property_find(key);
    if (!pi) {
        return strdup("Error: __system_property_find error");
    }
    ReadCallbackContext context = {.found = false};
    get_from_libc("__system_property_read_callback", &property_read_callback);
    if (property_read_callback == nullptr) return nullptr;
    property_read_callback(pi, handle_property, &context);
    if (context.found) {
        return strdup(context.value);
    }
    return nullptr;
}

char *property_get_by_fork_execve(const char *prop_name) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return strdup("pipe error");
    }
    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char *argv[] = {
                (char *) "/system/bin/getprop",
                (char *) prop_name,
                nullptr
        };
        execve(argv[0], argv, nullptr);
        _exit(EXIT_FAILURE);
    } else if (pid > 0) {
        close(pipefd[1]);
        char buffer[256];
//        TODO
        ssize_t count = read(pipefd[0], buffer, sizeof(buffer) - 1) - 1;
        close(pipefd[0]);
        if (count > 0) {
            buffer[count] = '\0';
            return strdup(buffer);
        }
        return strdup("read error");
    }
    return strdup("fork error");
}

Property *get_all_system_properties(int *count) {
    LOGI("init");
    FILE *pipe;
    char buffer[256];
    Property *properties = NULL;
    int capacity = 0;
    *count = 0;

    // 执行 getprop 命令
    pipe = popen("/system/bin/getprop", "r");
    if (!pipe) {
        perror("popen failed");
        return NULL;
    }

    // 逐行读取输出
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // 解析键值对（格式示例：[key]: [value]）
        char *key_start = strchr(buffer, '[');
        if (!key_start) continue;
        key_start++; // 跳过 '['

        char *key_end = strstr(key_start, "]: [");
        if (!key_end) continue;
        *key_end = '\0'; // 终止键名

        char *value_start = key_end + 4; // 值起始位置
        char *value_end = strchr(value_start, ']');
        if (!value_end) continue;
        *value_end = '\0'; // 终止值字符串

        // 动态扩展数组
        if (*count >= capacity) {
            capacity = (capacity == 0) ? 16 : capacity * 2;
            properties = static_cast<Property *>(realloc(properties, capacity * sizeof(Property)));
            if (!properties) {
                pclose(pipe);
                return NULL;
            }
        }

        // 存储键值对
        properties[*count].key = strdup(key_start);
        properties[*count].value = strdup(value_start);
        (*count)++;
    }

    pclose(pipe);
    return properties; // 调用者需释放内存
}

void free_properties(Property *properties, int count) {
    LOGI("free");
    for (int i = 0; i < count; i++) {
        free(properties[i].key);
        free(properties[i].value);
    }
    free(properties);
}

const char *property_get_by_search_from_all(Property *props, int count, const char *key) {
    for (int i = 0; i < count; i++) {
        if (strcmp(props[i].key, key) == 0) {
            return props[i].value;
        }
    }
    return nullptr;
}

static std::vector<Property> g_properties;

void property_foreach_callback(const prop_info *pi, void *cookie) {
    char name[PROP_NAME_MAX] = {0};
    char value[PROP_VALUE_MAX] = {0};
    __system_property_read(pi, name, value);
    g_properties.push_back({name, value});
}

Property *props;
int count;

void init_properties() {
    count = 0;
    props = get_all_system_properties(&count);
    LOGI("number of property: %d", count);
//    耗时太长： 平均3s
//    g_properties.clear();
//    __system_property_foreach(property_foreach_callback, nullptr);
}

void free_properties() {
    free_properties(props, count);
}

char *get_security_property(const char *key) {
    char *source1 = property_get_by_file_parse(key);
    char *source2 = property_get_by_popen(key);
    char *source3 = property_get_by_fork_execve(key);
    char *source4 = property_get_by_system_get(key);
    const char *source5 = property_get_by_search_from_all(props, count, key);
    char *source6 = property_get_by_system_readcallback(key);
    if (strcmp(source1, source2) == 0) {
        if (strcmp(source2, source3) == 0) {
            if (strcmp(source3, source4) == 0) {
                if (strcmp(source4, source5) == 0) {
                    if (strcmp(source5, source6) == 0) {
                        return source1;
                    }
                }
            }
        }
    }
    return strdup("error");
}


