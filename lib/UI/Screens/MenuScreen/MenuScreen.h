#pragma once

#include <cstdint>

#include <lvgl.h>

#include "InputManager.h"
#include "../../../Core/Menu/MenuItem.h"

class MenuScreen final
{
public:
    using SelectionCallback = void (*)(
        void *context,
        std::uint8_t index,
        const MenuItem &item);

    MenuScreen() = default;

    /*
     * Tạo giao diện menu.
     *
     * items:
     *   Danh sách mục menu bên ngoài truyền vào.
     *
     * itemCount:
     *   Số lượng mục menu.
     *
     * selectionCallback:
     *   Callback được gọi khi người dùng chọn một mục.
     *
     * callbackContext:
     *   Con trỏ ngữ cảnh trả lại cho callback.
     */
    bool create(
        lv_obj_t *parent,
        const MenuItem *items,
        std::uint8_t itemCount,
        SelectionCallback selectionCallback,
        void *callbackContext);

    /*
     * Mở menu.
     */
    void open();

    /*
     * Đóng menu và ẩn status.
     */
    void close();

    /*
     * Kiểm tra menu có đang hiển thị không.
     */
    bool isVisible() const;

    /*
     * Nhận input khi menu đang mở.
     */
    void handleInput(
        InputManager::Action action);

    /*
     * Hiển thị thông báo trạng thái.
     *
     * Nội dung và ý nghĩa thông báo được quyết định
     * bởi HomeScreen hoặc lớp gọi bên ngoài.
     */
    void showStatus(
        const char *text,
        std::uint32_t color);

    /*
     * Ẩn thông báo trạng thái.
     */
    void hideStatus();

    /*
     * Trả về index đang được chọn.
     */
    std::uint8_t selectedIndex() const;

private:
    void createStatusLabel(
        lv_obj_t *parent);

    void createMenuCard(
        lv_obj_t *parent);

    void updateMenuDisplay();
    void selectCurrentMenuItem();

    const MenuItem *items_ = nullptr;
    std::uint8_t itemCount_ = 0;

    SelectionCallback selectionCallback_ = nullptr;
    void *callbackContext_ = nullptr;

    lv_obj_t *menuCard_ = nullptr;
    lv_obj_t *menuItemLabel_ = nullptr;
    lv_obj_t *menuPositionLabel_ = nullptr;
    lv_obj_t *menuControlLabel_ = nullptr;

    lv_obj_t *statusLabel_ = nullptr;

    bool visible_ = false;
    std::uint8_t selectedMenuIndex_ = 0;
};