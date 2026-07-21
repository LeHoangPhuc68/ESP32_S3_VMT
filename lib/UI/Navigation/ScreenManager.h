#pragma once

#include <cstdint>

#include "../Screens/BaseScreen.h"
#include "ScreenId.h"

class ScreenManager final
{
public:
    /*
     * Khởi tạo bộ quản lý màn hình.
     */
    static bool begin();

    /*
     * Đăng ký một screen với ScreenId.
     *
     * Screen phải được create() trước khi hiển thị.
     */
    static bool registerScreen(
        ScreenId screenId,
        BaseScreen &screen);

    /*
     * Chuyển sang màn hình theo ScreenId.
     */
    static bool show(
        ScreenId screenId);

    /*
     * Chuyển trực tiếp sang một BaseScreen.
     *
     * Hàm này chủ yếu phục vụ nội bộ.
     */
    static bool show(
        BaseScreen &screen);

    /*
     * Trả về screen đang hiển thị.
     */
    static BaseScreen *currentScreen();

    /*
     * Trả về ScreenId đang hiển thị.
     */
    static ScreenId currentScreenId();

    /*
     * Tìm screen theo ScreenId.
     */
    static BaseScreen *getScreen(
        ScreenId screenId);

    /*
     * Kiểm tra ScreenManager đã được khởi tạo chưa.
     */
    static bool isInitialized();

private:
    static constexpr std::uint8_t ScreenCount =
        static_cast<std::uint8_t>(
            ScreenId::Count);

    static bool isValidScreenId(
        ScreenId screenId);

    static bool initialized_;

    static BaseScreen *screens_[ScreenCount];

    static BaseScreen *currentScreen_;

    static ScreenId currentScreenId_;
};