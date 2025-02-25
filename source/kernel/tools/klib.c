#include "tools/klib.h"

char * get_file_name (const char * name){
    char * s = (char *)name;
    while (*s != '\0') {
        s++;
    }

    while ((*s != '/') && (*s != '\\') && (s >= name)) {
        s--;
    }

    return s + 1;
}

int strings_count (char ** start){
    int count = 0;

    if (start) {
        while (*start++) {
            count ++;
        }
    }

    return count;
}

void kernel_strcpy(char * dest, const char * src) {
    if (!dest || !src) {
        return;
    }

    while (*dest && *src) {
        *dest++ = *src++;
    }

    *dest = '\0';
}

void kernel_strncpy(char * dest, const char * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    char * d = dest;
    const char * s = src;

    while ((size-- > 0) && (*s)) {
        *d++ = *s++;
    }

    if (size == 0) {
        *(d - 1) = '\0';
    } else {
        *d = '\0';
    }

}

int kernel_strncmp(const char * s1, const char * s2, int size){
    if (!s1 || !s2) {
        return -1;
    }

    while (*s1 && *s2 && (*s1 == *s2) && size--) {
        s1 ++;
        s2 ++;
    }

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}

int kernel_strlen(const char * str) {
    if (!str) {
        return 0;
    }

    const char * c = str;
    int len = 0;
    while (*c++){
        len ++;
    }

    return len;
}

void kernel_memcpy(void * dest, void * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    uint8_t * s = (uint8_t *)src;
    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void * dest, uint8_t v, int size) {
    if (!dest || !size) {
        return;
    }

    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = v;
    }
}

int kernel_memcmp(void * d1, void * d2, int size) {
    if (!d1 || !d2 || !size) {
        return 1;
    }

    uint8_t * p_d1 = (uint8_t *)d1;
    uint8_t * p_d2 = (uint8_t *)d2;
    while (size--) {
        if (*p_d1++ != *p_d2++) {
            return 1;
        }
    }

    return 0;
}

void kernel_itoa(char *buf, int num, int base) {
    static const char *num2ch = "0123456789ABCDEF";  // 十六进制字符映射
    char *p = buf;
    
    // 仅支持部分进制
    if ((base != 2) && (base != 8) && (base != 10) && (base != 16)) {
        *p = '\0';
        return;
    }

    // 处理十进制负数
    int signed_num = 0;
    if ((num < 0) && (base == 10)) {
        *p++ = '-';
        signed_num = 1;
        num = -num;  // 负数转为正数
    }

    // 处理数字转换
    if (base == 10 || base == 16) {
        uint32_t u_num = (uint32_t)num;  // 转为无符号数处理
        do {
            *p++ = num2ch[u_num % base];  // 获取当前位字符
            u_num /= base;
        } while (u_num);  // 直到除尽
    }

    *p-- = '\0';  // 结束字符串

    // 将转换结果逆序，生成最终的结果
    char *start = buf + (signed_num ? 1 : 0);  // 如果有符号，从第1位开始
    while (start < p) {
        char ch = *start;
        *start = *p;
        *p-- = ch;
        start++;
    }
}



void kernel_vsprintf(char * buf, const char * fmt, va_list args) {
    enum {NORMAL, READ_FMT} state = NORMAL;
    char * curr = buf;
    char ch;
    while ((ch = *fmt++)) {
        switch (state) {
            case NORMAL:
                if (ch == '%') {
                    state = READ_FMT;
                } else {
                    *curr++ = ch;
                }
                break;
            case READ_FMT:
                if (ch == 'd') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 10);
                    curr += kernel_strlen(curr);
                } else if (ch == 'x') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 16);
                    curr += kernel_strlen(curr);
                } else if (ch == 'c') {
                    char c = va_arg(args, int);
                    *curr += c;
                } else if (ch == 's') {
                    const char * str = va_arg(args, char *);
                    int len = kernel_strlen(str);
                    while (len--) {
                        *curr++ = *str++;
                    }
                }
                state = NORMAL;
                break;
        }
    }
}

void kernel_sprintf(char * buf, const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kernel_vsprintf(buf, fmt, args);
    va_end(args);
}

void pannic(const char * file, int line, const char * func, const char * cond) {
    log_printf("assert failed! %s", cond);
    log_printf("file: %s\nfunc: %s\nline: %d\n", file, func, line);
    for (;;) {
        hlt();
    }
}