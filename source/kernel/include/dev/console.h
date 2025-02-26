#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"
#include "dev/tty.h"

#define CONSOLE_DISP_ADDR 0Xb8000
#define CONSOLE_DISP_END  (0Xb8000 + 32 * 1024)
#define CONSOLE_ROW_MAX 25
#define CONSOLE_COL_MAX 80

#define ASCII_ESC 0X1b
#define ESC_PARAM_MAX 10

typedef enum {
    COLOR_Black = 0,
    COLOR_Blue,
    COLOR_Green,
    COLOR_Cyan,
    COLOR_Red,
    COLOR_Magenta,
    COLOR_Brown,        // 基础色 (6)
    COLOR_Gray,         // 基础色 (7)
    COLOR_DarkGray,     // 亮色 (8, 原名为Bright Black)
    COLOR_Light_Blue,    // 亮色 (9)
    COLOR_Light_Green,   // 亮色 (10)
    COLOR_Light_Cyan,    // 亮色 (11)
    COLOR_Light_Red,     // 亮色 (12)
    COLOR_Light_Magenta, // 亮色 (13)
    COLOR_Yellow,       // 亮色 (14, 对应基础色的Brown)
    COLOR_White         // 亮色 (15)
} color_t;

typedef union _disp_char_t {
    struct {
        char c;
        char foreground : 4;
        char background : 3;
    };
    uint16_t v;
}disp_char_t;

typedef struct _console_t {
    enum {
        CONSOLE_WRITE_NORMAL,
        CONSOLE_WRITE_ESC,
        CONSOLE_WRITE_SQUARE,
    }write_state;

    disp_char_t * disp_base;
    int cursor_row, cursor_col;
    int display_rows, display_cols;
    color_t foreground, background;

    int old_cursor_col, old_cursor_row;
    int esc_param[ESC_PARAM_MAX];
    int curr_param_index;
}console_t;

int console_init (int index);
int console_write (tty_t * tty);
void console_close (int console);
void console_select (int index);

#endif