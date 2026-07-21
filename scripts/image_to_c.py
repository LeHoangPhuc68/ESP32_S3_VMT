Import("env")

from pathlib import Path
import subprocess
import sys


# ============================================================
# CẤU HÌNH
# ============================================================

PROJECT_DIR = Path(env.subst("$PROJECT_DIR"))

INPUT_IMAGE = (
    PROJECT_DIR
    / "assets_src"
    / "wallpaper.png"
)

OUTPUT_DIRECTORY = (
    PROJECT_DIR
    / "lib"
    / "Assets"
)

OUTPUT_HEADER = OUTPUT_DIRECTORY / "wallpaper.h"
OUTPUT_SOURCE = OUTPUT_DIRECTORY / "wallpaper.c"

VARIABLE_NAME = "wallpaper"

TARGET_WIDTH = 320
TARGET_HEIGHT = 170

BYTES_PER_LINE = 16


# ============================================================
# PILLOW
# ============================================================

def load_pillow():
    try:
        from PIL import Image
        return Image

    except ImportError:
        print("[Assets] Pillow chưa được cài.")
        print("[Assets] Đang cài Pillow...")

        subprocess.check_call(
            [
                sys.executable,
                "-m",
                "pip",
                "install",
                "Pillow",
            ]
        )

        from PIL import Image
        return Image


Image = load_pillow()


# ============================================================
# KIỂM TRA FILE CÓ CẦN CONVERT LẠI KHÔNG
# ============================================================

def outputs_are_current() -> bool:
    if not INPUT_IMAGE.exists():
        return False

    if not OUTPUT_HEADER.exists():
        return False

    if not OUTPUT_SOURCE.exists():
        return False

    input_modified_time = INPUT_IMAGE.stat().st_mtime
    header_modified_time = OUTPUT_HEADER.stat().st_mtime
    source_modified_time = OUTPUT_SOURCE.stat().st_mtime

    return (
        header_modified_time >= input_modified_time
        and source_modified_time >= input_modified_time
    )


# ============================================================
# RESIZE VÀ CROP ẢNH
# ============================================================

def resize_and_crop(image):
    source_width, source_height = image.size

    if source_width <= 0 or source_height <= 0:
        raise ValueError("Kích thước ảnh đầu vào không hợp lệ.")

    source_ratio = source_width / source_height
    target_ratio = TARGET_WIDTH / TARGET_HEIGHT

    # Ảnh đầu vào rộng hơn màn hình:
    # resize theo chiều cao, sau đó cắt hai bên.
    if source_ratio > target_ratio:
        resized_height = TARGET_HEIGHT

        resized_width = round(
            source_width
            * TARGET_HEIGHT
            / source_height
        )

    # Ảnh đầu vào cao hơn màn hình:
    # resize theo chiều rộng, sau đó cắt trên/dưới.
    else:
        resized_width = TARGET_WIDTH

        resized_height = round(
            source_height
            * TARGET_WIDTH
            / source_width
        )

    resized_image = image.resize(
        (resized_width, resized_height),
        Image.Resampling.LANCZOS,
    )

    crop_left = (resized_width - TARGET_WIDTH) // 2
    crop_top = (resized_height - TARGET_HEIGHT) // 2

    crop_right = crop_left + TARGET_WIDTH
    crop_bottom = crop_top + TARGET_HEIGHT

    return resized_image.crop(
        (
            crop_left,
            crop_top,
            crop_right,
            crop_bottom,
        )
    )


# ============================================================
# CHUYỂN RGB888 SANG RGB565
# ============================================================

def convert_to_rgb565(image) -> bytearray:
    rgb_image = image.convert("RGB")

    output = bytearray()

    pixels = rgb_image.getdata()

    for red, green, blue in pixels:
        rgb565 = (
            ((red & 0xF8) << 8)
            | ((green & 0xFC) << 3)
            | (blue >> 3)
        )

        # ESP32 là little-endian:
        # byte thấp trước, byte cao sau.
        output.append(rgb565 & 0xFF)
        output.append((rgb565 >> 8) & 0xFF)

    return output


# ============================================================
# FORMAT MẢNG BYTE CHO FILE C
# ============================================================

def format_byte_array(data: bytearray) -> str:
    output_lines = []

    for index in range(0, len(data), BYTES_PER_LINE):
        chunk = data[index:index + BYTES_PER_LINE]

        formatted_chunk = ", ".join(
            f"0x{value:02X}"
            for value in chunk
        )

        output_lines.append(
            f"    {formatted_chunk},"
        )

    return "\n".join(output_lines)


# ============================================================
# SINH FILE HEADER
# ============================================================

def create_header() -> str:
    return f"""#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {{
#endif

extern const lv_image_dsc_t {VARIABLE_NAME};

#ifdef __cplusplus
}}
#endif
"""


# ============================================================
# SINH FILE SOURCE
# ============================================================

def create_source(pixel_data: bytearray) -> str:
    formatted_data = format_byte_array(pixel_data)

    stride = TARGET_WIDTH * 2

    return f"""/*
 * File này được sinh tự động bởi scripts/image_to_c.py.
 *
 * KHÔNG sửa file này bằng tay.
 * Mọi thay đổi sẽ bị ghi đè trong lần build tiếp theo.
 */

#include "wallpaper.h"

#include <stddef.h>
#include <stdint.h>

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_LARGE_CONST
#define LV_ATTRIBUTE_LARGE_CONST
#endif

static const LV_ATTRIBUTE_MEM_ALIGN
LV_ATTRIBUTE_LARGE_CONST
uint8_t {VARIABLE_NAME}_map[] = {{
{formatted_data}
}};

const lv_image_dsc_t {VARIABLE_NAME} = {{
    .header = {{
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_RGB565,
        .flags = 0,
        .w = {TARGET_WIDTH},
        .h = {TARGET_HEIGHT},
        .stride = {stride},
        .reserved_2 = 0,
    }},

    .data_size = sizeof({VARIABLE_NAME}_map),
    .data = {VARIABLE_NAME}_map,
    .reserved = NULL,
}};
"""


# ============================================================
# CONVERT
# ============================================================

def convert_image():
    print()
    print("[Assets] Kiểm tra wallpaper...")

    if not INPUT_IMAGE.exists():
        print(
            "[Assets] Không tìm thấy file:"
        )

        print(
            f"[Assets] {INPUT_IMAGE}"
        )

        print(
            "[Assets] Hãy tạo assets_src/wallpaper.png"
        )

        return

    if outputs_are_current():
        print(
            "[Assets] Wallpaper không thay đổi."
        )

        print(
            "[Assets] Không cần convert lại."
        )

        return

    OUTPUT_DIRECTORY.mkdir(
        parents=True,
        exist_ok=True,
    )

    print(
        f"[Assets] Đang đọc: {INPUT_IMAGE.name}"
    )

    with Image.open(INPUT_IMAGE) as source_image:
        processed_image = resize_and_crop(
            source_image
        )

        pixel_data = convert_to_rgb565(
            processed_image
        )

    OUTPUT_HEADER.write_text(
        create_header(),
        encoding="utf-8",
    )

    OUTPUT_SOURCE.write_text(
        create_source(pixel_data),
        encoding="utf-8",
    )

    print(
        "[Assets] Convert thành công."
    )

    print(
        f"[Assets] Kích thước: "
        f"{TARGET_WIDTH}x{TARGET_HEIGHT}"
    )

    print(
        f"[Assets] Dữ liệu RGB565: "
        f"{len(pixel_data)} bytes"
    )

    print(
        "[Assets] Đã tạo:"
    )

    print(
        f"[Assets] "
        f"{OUTPUT_HEADER.relative_to(PROJECT_DIR)}"
    )

    print(
        f"[Assets] "
        f"{OUTPUT_SOURCE.relative_to(PROJECT_DIR)}"
    )


try:
    convert_image()

except Exception as error:
    print()
    print("[Assets] CONVERT THẤT BẠI:")
    print(f"[Assets] {error}")
    print()

    env.Exit(1)
