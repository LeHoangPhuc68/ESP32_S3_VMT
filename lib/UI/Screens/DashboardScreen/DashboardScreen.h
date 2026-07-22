#pragma once

#include <lvgl.h>

#include "../BaseScreen.h"

class DashboardScreen final : public BaseScreen
{
public:
    using NavigationCallback =
        void (*)(void *context);

    DashboardScreen() = default;

    /*
     * Tạo toàn bộ object LVGL của Dashboard.
     *
     * navigationCallback được gọi khi người dùng
     * yêu cầu quay lại menu cha.
     */
    bool create(
        lv_obj_t *parent,
        NavigationCallback navigationCallback,
        void *navigationContext);

    /*
     * BaseScreen interface.
     */
    bool create(lv_obj_t *parent) override;

    void show() override;
    void hide() override;
    void update() override;

    void handleInput(
        InputManager::Action action) override;

    const char *name() const override;

private:
    void createHeader();
    void createSystemCard();
    void createConnectivityCard();
    void createFooter();

    void requestParent();

    lv_obj_t *root_ = nullptr;

    lv_obj_t *titleLabel_ = nullptr;
    lv_obj_t *subtitleLabel_ = nullptr;

    lv_obj_t *systemCard_ = nullptr;
    lv_obj_t *systemTitleLabel_ = nullptr;
    lv_obj_t *systemValueLabel_ = nullptr;

    lv_obj_t *connectivityCard_ = nullptr;
    lv_obj_t *connectivityTitleLabel_ = nullptr;
    lv_obj_t *connectivityValueLabel_ = nullptr;

    lv_obj_t *footerLabel_ = nullptr;

    NavigationCallback navigationCallback_ = nullptr;
    void *navigationContext_ = nullptr;

    unsigned long lastUpdateTime_ = 0;
};
