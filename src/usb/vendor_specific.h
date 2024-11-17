// SPDX-FileCopyrightText: 2024 ShinagwaKazemaru
// SPDX-License-Identifier: MIT

/**
 *   This file is based on
 *   pico-examples/usb/device/dev_lowlevel --------------+
 *   | Copyright (c) 2020 Raspberry Pi (Trading) Ltd.    |
 *   | SPDX-License-Identifier: BSD-3-Clause             |
 *   | URL: https://github.com/raspberrypi/pico-examples |
 *   +---------------------------------------------------+
 */

#ifndef _USB_VENDOR_SPECIFIC_H
#define _USB_VENDOR_SPECIFIC_H

#include "common.h"
#include "../sound.h"

#define USB_DATA_SIZE 64

static uint16_t *usb_sending_sound;
static uint16_t usb_sending_checker = 0;
static uint16_t usb_sent_offset;

static bool usb_need_send = false;
static bool usb_is_sending_sound = false;

static usb_endpoint_t *usb_ep_cmd;
static usb_endpoint_t *usb_ep_data;

static uint8_t usb_rest = 0;

static inline void usb_try_start_send_sound(void) {
    if (usb_rest != 0 && ! usb_is_sending_sound && usb_sending_checker != sound_checker) {
        usb_sending_sound = sound_bufs[sound_front];
        usb_sending_checker = sound_checker;

        usb_rest--;
        usb_is_sending_sound = true;
        usb_start_transfer(usb_ep_data, (uint8_t*)usb_sending_sound, USB_DATA_SIZE);
        usb_sent_offset = USB_DATA_SIZE;
    }
}


void usb_ep_cmd_handler(uint8_t *buf, uint16_t len) {
    if (len == 2 && buf[0] == 0x01 && buf[1] != 0) {
        usb_rest = buf[1];
        usb_is_sending_sound = false;
        usb_try_start_send_sound();
    }
    usb_start_transfer(usb_ep_cmd, NULL, 2);
}


void usb_ep_data_handler(uint8_t* buf, uint16_t len) {
    if (usb_sent_offset < SOUND_BUF_SIZE) {
        usb_start_transfer(usb_ep_data, (uint8_t*)usb_sending_sound + usb_sent_offset, USB_DATA_SIZE);
        usb_sent_offset += USB_DATA_SIZE;
    } else {
        usb_is_sending_sound = false;
    }
}

static inline void usb_interface_init(void) {
    usb_ep_data = &usb_eps[0];
    usb_ep_cmd = &usb_eps[1];

    usb_ep_data->handler = &usb_ep_data_handler;
    usb_ep_cmd->handler = &usb_ep_cmd_handler;

    usb_start_transfer(usb_ep_cmd, NULL, 2);
}

#endif