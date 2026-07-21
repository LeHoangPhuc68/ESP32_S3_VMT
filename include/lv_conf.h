#pragma once

#ifndef LV_CONF_H
#define LV_CONF_H

/* Màu màn hình RGB565 */
#define LV_COLOR_DEPTH 16

/* Dùng malloc/free mặc định của LVGL */
#define LV_USE_STDLIB_MALLOC LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_BUILTIN

/* Tick được cấp bằng lv_tick_set_cb() trong UIManager.cpp */
#define LV_TICK_CUSTOM 0

/* Tần suất refresh */
#define LV_DEF_REFR_PERIOD 16
#define LV_DPI_DEF 130

/* Logging */
#define LV_USE_LOG 0

/* Bật các font đang dùng trong UIManager.cpp */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 0

/* Font mặc định */
#define LV_FONT_DEFAULT &lv_font_montserrat_12

/* Không cần các font này ở thời điểm hiện tại */
#define LV_FONT_MONTSERRAT_8 0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Widget cần dùng */
#define LV_USE_LABEL 1

#endif
