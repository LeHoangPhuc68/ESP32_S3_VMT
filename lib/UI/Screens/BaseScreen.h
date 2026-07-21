#pragma once

#include <lvgl.h>

#include "InputManager.h"

class BaseScreen
{
public:
    virtual ~BaseScreen() = default;

    /*
     * Tạo toàn bộ object LVGL của screen.
     *
     * Hàm chỉ được gọi một lần trong quá trình khởi tạo.
     */
    virtual bool create(lv_obj_t *parent) = 0;

    /*
     * Hiển thị screen.
     */
    virtual void show() = 0;

    /*
     * Ẩn screen.
     */
    virtual void hide() = 0;

    /*
     * Cập nhật logic riêng của screen.
     *
     * Mặc định không làm gì.
     */
    virtual void update()
    {
    }

    /*
     * Nhận input đã được InputManager chuyển đổi
     * thành action cấp cao.
     */
    virtual void handleInput(
        InputManager::Action action) = 0;

    /*
     * Tên screen phục vụ debug và điều hướng.
     */
    virtual const char *name() const = 0;
};