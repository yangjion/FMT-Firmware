/******************************************************************************
 * Copyright 2020-2021 The Firmament Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include "driver/ncp5623c.h"
#include "hal/i2c.h"

#define DRV_DBG(...) console_printf(__VA_ARGS__)
// #define DRV_DBG(...)

#define LED_BRIGHT 0x1f /**< full brightness */

static struct rt_i2c_device* i2c_device;
static struct rt_device ncp5623c_dev;
static uint8_t _r = 0;
static uint8_t _g = 0;
static uint8_t _b = 0;
static uint8_t _brightness = 0;

static void send_led_rgb(void)
{
    struct rt_i2c_msg msgs[4];
    uint8_t data[4] = { NCP5623_LED_CURRENT | _brightness,
        NCP5623_LED_PWM0 | _r, NCP5623_LED_PWM1 | _g, NCP5623_LED_PWM2 | _b };

    msgs[0].flags = RT_I2C_WR | i2c_device->flags;
    msgs[0].buf = &data[0];
    msgs[0].len = 1;

    msgs[1].flags = RT_I2C_WR | i2c_device->flags;
    msgs[1].buf = &data[1];
    msgs[1].len = 1;

    msgs[2].flags = RT_I2C_WR | i2c_device->flags;
    msgs[2].buf = &data[2];
    msgs[2].len = 1;

    msgs[3].flags = RT_I2C_WR | i2c_device->flags;
    msgs[3].buf = &data[3];
    msgs[3].len = 1;

    if (rt_i2c_transfer(i2c_device->bus, i2c_device->slave_addr, msgs, 4) != 4) {
        DRV_DBG("set led rgb fail!\n");
    }
}

static rt_err_t ncp5623c_control(rt_device_t dev, int cmd, void* args)
{
    uint8_t color;

    switch (cmd) {
    case NCP5623_CMD_SET_COLOR:
        color = (uint32_t)args;
        if (color == NCP5623_LED_RED) {
            _r = LED_BRIGHT;
            _g = 0;
            _b = 0;
        } else if (color == NCP5623_LED_GREEN) {
            _r = 0;
            _g = LED_BRIGHT;
            _b = 0;
        } else if (color == NCP5623_LED_BLUE) {
            _r = 0;
            _g = 0;
            _b = LED_BRIGHT;
        } else if (color == NCP5623_LED_YELLOW) {
            _r = LED_BRIGHT;
            _g = LED_BRIGHT;
            _b = 0;
        } else if (color == NCP5623_LED_PURPLE) {
            _r = LED_BRIGHT;
            _g = 0;
            _b = LED_BRIGHT;
        } else if (color == NCP5623_LED_CYAN) {
            _r = 0;
            _g = LED_BRIGHT;
            _b = LED_BRIGHT;
        } else if (color == NCP5623_LED_BLUE) {
            _r = LED_BRIGHT;
            _g = LED_BRIGHT;
            _b = LED_BRIGHT;
        } else {
            _r = 0;
            _g = 0;
            _b = 0;
        }
        break;

    case NCP5623_CMD_SET_BRIGHT:
        _brightness = (uint32_t)args & 0x1F;
        // NOTE: NCP5623C speed is low, so don't write i2c command too fast
        send_led_rgb();
        break;

    default:
        break;
    }

    return RT_EOK;
}

rt_err_t drv_ncp5623c_init(const char* dev_name)
{
    i2c_device = (struct rt_i2c_device*)rt_device_find(dev_name);
    RT_ASSERT(i2c_device != NULL);

    RT_CHECK_RETURN(rt_device_open(&i2c_device->parent, RT_DEVICE_OFLAG_RDWR));

    rt_device_t device = &ncp5623c_dev;

    device->type = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    device->user_data = RT_NULL;

    device->init = RT_NULL;
    device->open = RT_NULL;
    device->close = RT_NULL;
    device->read = RT_NULL;
    device->write = RT_NULL;
    device->control = ncp5623c_control;

    return rt_device_register(device, "ncp5623c", RT_DEVICE_OFLAG_RDWR);
}