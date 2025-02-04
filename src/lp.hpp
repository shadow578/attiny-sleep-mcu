/**
 * Low-Power module, provides functions for drastically reducing power consuption
 */
#pragma once

namespace lp
{
    /**
     * disable all extra peripherals, keeps GPIO untouched
     */
    void power_down_all();

    /**
     * set GPIO pins to input, no pull-up, to reduce power consuption
     */
    void reset_gpio();
} // namespace lp
