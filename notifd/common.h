#ifndef NOTIF_COMMON_H
#define NOTIF_COMMON_H

#include <stdint.h>
#include <sys/types.h>
#include <cairo/cairo.h>
#include <X11/Xft/Xft.h>

#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

#define MAX(a, b)       ((a > b) ? a : b)
#define MIN(a, b)       ((a < b) ? a : b)
#define MAXSTR(a, b)    ((strlen(a) > strlen(b)) ? a : b)
#define MINSTR(a, b)    ((strlen(a) < strlen(b)) ? a : b)
#define LEN(x)          (sizeof(x)/sizeof(*x))

typedef uint64_t        u64;
typedef  int64_t        i64;

typedef uint32_t        u32;
typedef  int32_t        i32;

typedef uint16_t        u16;
typedef  int16_t        i16;

typedef uint8_t         u8;
typedef  int8_t         i8;

typedef __u_char        uchar;

#ifndef __cplusplus
typedef uchar           bool;
#define false           0
#define true            1
#endif

typedef enum {
    COLOR_BG        = (u8) 0,
    COLOR_SUMMARY   = (u8) 1,
    COLOR_BODY      = (u8) 2,
    
    COLOR_MAX,
} notif_color_enum_e;

typedef enum {
    NOTIF_FONT_WEIGHT_NORMAL    = CAIRO_FONT_WEIGHT_NORMAL,
    NOTIF_FONT_WEIGHT_BOLD      = CAIRO_FONT_WEIGHT_BOLD,
} notif_font_weight_e;

typedef enum {
    NOTIF_FONT_SLANT_NORMAL     = CAIRO_FONT_SLANT_NORMAL,
    NOTIF_FONT_SLANT_ITALIC     = CAIRO_FONT_SLANT_ITALIC,
    NOTIF_FONT_SLANT_OBLIQUE    = CAIRO_FONT_SLANT_OBLIQUE,
} notif_font_slant_e;

typedef struct {
    const char * const      color;
    notif_font_weight_e     weight;
    notif_font_slant_e      slant;
    XftColor                rawColor;
} notif_font_setting_t;

typedef struct {
    u32 width, height;
} notif_rect_t;

#endif /* NOTIF_COMMON_H */
