// touchscreen.c
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include "ft5x06_touchscreen.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "string.h"
#include "display.h"
#include "soc/io_mux_reg.h"
#include "ui.h"
#include "pins.h"

#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_FREQ_HZ                 (100000)

int16_t touch_x_delta = 0;
int16_t touch_y_delta = 0;
float touch_y_scale = 1.0f;

typedef struct {
    i2c_config_t i2c_conf;   /*!<I2C bus parameters*/
    i2c_port_t i2c_port;     /*!<I2C port number */
} i2c_bus_t;

static const char* I2C_BUS_TAG = "i2c_bus";
#define I2C_BUS_CHECK(a, str, ret)  if(!(a)) {                                             \
    ESP_LOGE(I2C_BUS_TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);      \
    return (ret);                                                                   \
    }
#define ESP_INTR_FLG_DEFAULT  (0)
#define ESP_I2C_MASTER_BUF_LEN  (0)

i2c_bus_handle_t iot_i2c_bus_create(i2c_port_t port, i2c_config_t* conf)
{
    I2C_BUS_CHECK(port < I2C_NUM_MAX, "I2C port error", NULL);
    I2C_BUS_CHECK(conf != NULL, "Pointer error", NULL);
    i2c_bus_t* bus = (i2c_bus_t*) calloc(1, sizeof(i2c_bus_t));
    bus->i2c_conf = *conf;
    bus->i2c_port = port;
    esp_err_t ret = i2c_param_config(bus->i2c_port, &bus->i2c_conf);
    if(ret != ESP_OK) {
        goto error;
    }
    ret = i2c_driver_install(bus->i2c_port, bus->i2c_conf.mode, ESP_I2C_MASTER_BUF_LEN, ESP_I2C_MASTER_BUF_LEN, ESP_INTR_FLG_DEFAULT);
    if(ret != ESP_OK) {
        goto error;
    }
    return (i2c_bus_handle_t) bus;
    error:
    if(bus) {
        free(bus);
    }

    return NULL;
}

esp_err_t iot_i2c_bus_delete(i2c_bus_handle_t bus)
{
    I2C_BUS_CHECK(bus != NULL, "Handle error", ESP_FAIL);
    i2c_bus_t* i2c_bus = (i2c_bus_t*) bus;
    i2c_driver_delete(i2c_bus->i2c_port);
    free(bus);
    return ESP_OK;
}

esp_err_t iot_i2c_bus_cmd_begin(i2c_bus_handle_t bus, i2c_cmd_handle_t cmd, portBASE_TYPE ticks_to_wait)
{
    I2C_BUS_CHECK(bus != NULL, "Handle error", ESP_FAIL);
    I2C_BUS_CHECK(cmd != NULL, "I2C cmd error", ESP_FAIL);
    i2c_bus_t* i2c_bus = (i2c_bus_t*) bus;
    return i2c_master_cmd_begin(i2c_bus->i2c_port, cmd, ticks_to_wait);
}

esp_err_t iot_ft5x06_read(ft5x06_handle_t dev, uint8_t start_addr,
        uint8_t read_num, uint8_t *data_buf)
{
    esp_err_t ret = ESP_FAIL;

    if (data_buf != NULL) {
        i2c_cmd_handle_t cmd = NULL;
        ft5x06_dev_t* device = (ft5x06_dev_t*) dev;
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (device->dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, start_addr, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 50 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        if(ret != ESP_OK) {
            return ESP_FAIL;
        }
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (device->dev_addr << 1) | READ_BIT, ACK_CHECK_EN);
        if(read_num > 1) {
            i2c_master_read(cmd, data_buf, read_num-1, ACK_VAL);
        }
        i2c_master_read_byte(cmd, &data_buf[read_num-1], NACK_VAL);
        i2c_master_stop(cmd);
        ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 50 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }
    return ret;
}

esp_err_t iot_ft5x06_write(ft5x06_handle_t dev, uint8_t start_addr,
        uint8_t write_num, uint8_t *data_buf)
{
    esp_err_t ret = ESP_FAIL;
    if (data_buf != NULL) {
        ft5x06_dev_t* device = (ft5x06_dev_t*) dev;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (device->dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, start_addr, ACK_CHECK_EN);
        i2c_master_write(cmd, data_buf, write_num, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }
    return ret;
}

esp_err_t iot_ft5x06_touch_report(ft5x06_handle_t device, touch_info_t* ifo)
{
    if(device == NULL || ifo == NULL) {
        return ESP_FAIL;
    }
    ft5x06_dev_t* dev = (ft5x06_dev_t*) device;
    uint8_t data[32];
    memset(data, 0, sizeof(data));
    if(iot_ft5x06_read(dev, 0x00, 32, data) == ESP_OK) {
        ifo->touch_point = data[2] & 0x7;
        if(ifo->touch_point > 0 && ifo->touch_point <=5) {
            if(!(dev->xy_swap)) {
                ifo->curx[0] = H_RES -(((uint16_t)(data[0x03] & 0x0f) << 8) | data[0x04]);
                ifo->cury[0] = ((uint16_t)(data[0x05] & 0x0f) << 8) | data[0x06];
                ifo->curx[1] = H_RES -(((uint16_t)(data[0x09] & 0x0f) << 8) | data[0x0a]);
                ifo->cury[1] = ((uint16_t)(data[0x0b] & 0x0f) << 8) | data[0x0c];
                ifo->curx[2] = H_RES -(((uint16_t)(data[0x0f] & 0x0f) << 8) | data[0x10]);
                ifo->cury[2] = ((uint16_t)(data[0x11] & 0x0f) << 8) | data[0x12];
                ifo->curx[3] = H_RES -(((uint16_t)(data[0x15] & 0x0f) << 8) | data[0x16]);
                ifo->cury[3] = ((uint16_t)(data[0x17] & 0x0f) << 8) | data[0x18];
                ifo->curx[4] = H_RES -(((uint16_t)(data[0x1b] & 0x0f) << 8) | data[0x1c]);
                ifo->cury[4] = ((uint16_t)(data[0x1d] & 0x0f) << 8) | data[0x1e];
            } else {
                ifo->cury[0] = H_RES - (((uint16_t)(data[0x03] & 0x0f) << 8) | data[0x04]);
                ifo->curx[0] = ((uint16_t)(data[0x05] & 0x0f) << 8) | data[0x06];
                ifo->cury[1] = H_RES - (((uint16_t)(data[0x09] & 0x0f) << 8) | data[0x0a]);
                ifo->curx[1] = ((uint16_t)(data[0x0b] & 0x0f) << 8) | data[0x0c];
                ifo->cury[2] = H_RES - (((uint16_t)(data[0x0f] & 0x0f) << 8) | data[0x10]);
                ifo->curx[2] = ((uint16_t)(data[0x11] & 0x0f) << 8) | data[0x12];
                ifo->cury[3] = H_RES - (((uint16_t)(data[0x15] & 0x0f) << 8) | data[0x16]);
                ifo->curx[3] = ((uint16_t)(data[0x17] & 0x0f) << 8) | data[0x18];
                ifo->cury[4] = H_RES - (((uint16_t)(data[0x1b] & 0x0f) << 8) | data[0x1c]);
                ifo->curx[4] = ((uint16_t)(data[0x1d] & 0x0f) << 8) | data[0x1e];
            }
            ifo->touch_event = TOUCH_EVT_PRESS;
        } else {
            ifo->touch_event = TOUCH_EVT_RELEASE;
            ifo->touch_point = 0;
        }
    } else {
        ifo->touch_event = TOUCH_EVT_RELEASE;
        ifo->touch_point = 0;
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t iot_ft5x06_init(ft5x06_handle_t dev, ft5x06_cfg_t * cfg)
{
    return ESP_OK;
}

ft5x06_handle_t iot_ft5x06_create(i2c_bus_handle_t bus, uint16_t dev_addr)
{
    ft5x06_dev_t* dev = (ft5x06_dev_t*) calloc(1, sizeof(ft5x06_dev_t));
    if(dev == NULL) {
        fprintf(stderr,"dev handle calloc fail\n");
        return NULL;
    }
    dev->bus = bus;
    dev->dev_addr = dev_addr;
    dev->xy_swap = false;
    return (ft5x06_handle_t) dev;
}


static i2c_bus_handle_t i2c_bus = NULL;
static ft5x06_handle_t dev = NULL;

/**
 * @brief i2c master initialization
 */
static void i2c_bus_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_CLK_FREQ,
    };

    i2c_bus = iot_i2c_bus_create(I2C_NUM, &conf);
}

void ft5x06_init()
{
    i2c_bus_init(); 
    gpio_set_direction(TOUCH_INT, GPIO_MODE_INPUT);
    dev = iot_ft5x06_create(i2c_bus, FT5X06_ADDR_DEF);
    if(dev == NULL) {
        fprintf(stderr,"ft5x06 device create fail\n");
        fprintf(stderr, "%s:%d (%s)- assert failed!\n", __FILE__, __LINE__, __FUNCTION__);
        abort();
    }
    fprintf(stderr,"ft5x06 device created\n");
}


void run_ft5x06(void *param)
{
    int i = 0;
    uint8_t press_received = 0;
    touch_info_t touch_info;
    uint8_t got_primary_touch = 0;
    while (1) {
        int8_t p0 = gpio_get_level(TOUCH_INT);
        int8_t p1 = iot_ft5x06_touch_report(dev, &touch_info);
        got_primary_touch = 0;
        if(p0 == 0 && p1 == ESP_OK) {
            if(touch_info.touch_event == TOUCH_EVT_RELEASE) {
                send_touch_to_micropython(last_touch_x[0], last_touch_y[0], 1);                 
            } else {
                press_received = 1;
                for(i = 0; i < touch_info.touch_point; i++) {
                    if(i<4) {
                        last_touch_x[i] = (H_RES - touch_info.curx[i]) + touch_x_delta;
                        if(last_touch_x[i] < 0) last_touch_x[i] = 0;
                        if(last_touch_x[i] >= H_RES) last_touch_x[i] = H_RES-1;

                        last_touch_y[i] = (touch_info.cury[i] + touch_y_delta)*touch_y_scale;
                        if(last_touch_y[i] < 0) last_touch_y[i] = 0;
                        if(last_touch_y[i] >= V_RES) last_touch_y[i] = V_RES-1;

                    }
                    if(i==0) got_primary_touch = 1;
                    //if(i==0) fprintf(stderr,"held %d evt %d touch i %d touch point %d  x:%d  y:%d became %d %d\n", touch_held, touch_info.touch_event, i, touch_info.touch_point, touch_info.curx[i], touch_info.cury[i], last_touch_x[i], last_touch_y[i]);
                }
                if(got_primary_touch) {
                    send_touch_to_micropython(last_touch_x[0], last_touch_y[0], 0); 
                } else {
                    send_touch_to_micropython(last_touch_x[0], last_touch_y[0], 1);                 
                }
            }
        } else {
            // Sometimes it doesn't return a full release event. So we watch for a transition from press to release
            //if(touch_info.touch_event == TOUCH_EVT_RELEASE && last_te == TOUCH_EVT_PRESS) {
            if(touch_info.touch_event == TOUCH_EVT_RELEASE && press_received) {
                send_touch_to_micropython(last_touch_x[0], last_touch_y[0], 1);
                press_received = 0;                 
            }
        }
        vTaskDelay(20/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

