#ifndef NOTIF_CONFIG_H
#define NOTIF_CONFIG_H

#include <notifd/common.h>

static const char * const font = "monospace";
static const bool fontAntialias = true;
static const u32 fontsize = 14;
static const u32 textSpacing = 10;

static const u32 xPadding = 24;
static const u32 yPadding = 24;

static const u32 xMargin = 24;
static const u32 yMargin = 24;

static const u32 activeScreen = 1;

static const notif_font_setting_t fontSettings[] = {
    // Background
    { "#333333", NOTIF_FONT_WEIGHT_NORMAL, NOTIF_FONT_SLANT_NORMAL },

    // Summary
    { "#ffffff", NOTIF_FONT_WEIGHT_NORMAL, NOTIF_FONT_SLANT_NORMAL },

    // Body
    { "#ffffff", NOTIF_FONT_WEIGHT_NORMAL, NOTIF_FONT_SLANT_NORMAL },
};

#endif /* NOTIF_CONFIG_H */
