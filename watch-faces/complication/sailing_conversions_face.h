/*
 * MIT License
 *
 * Copyright (c) 2023 PrimmR
 * Copyright (c) 2025 buckket
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SAILING_CONVERSIONS_FACE_H_
#define SAILING_CONVERSIONS_FACE_H_

#include "movement.h"

/*
 * Kitchen Conversions
 * A face that allows the user to convert between common kitchen units of measurement
 *
 * How to use
 * ----------
 * Short press the light button to move forward through menus, and long press to move backwards
 *
 * Press the alarm button to cycle through options in the menus
 *
 * When inputting a number, the light button moves forward one place and the alarm button increments the value of the selected digit
 *
 * To convert between Imperial (GB) and US (A) measurements of volume, hold the alarm button
 *
 */

#define SCREEN_NUM 5

// Names of each page
typedef enum
{
    sailing_measurement,
    sailing_from,
    sailing_to,
    sailing_input,
    sailing_result,
} sailing_page_t;

#define DISPLAY_DIGITS 4

// Settings when app is running
typedef struct
{
    sailing_page_t pg;
    uint8_t measurement_i;
    uint8_t from_i;
    uint8_t to_i;
    uint32_t selection_value;
    uint8_t selection_index;
    bool alarm_held;
} sailing_conversions_state_t;

void sailing_conversions_face_setup(uint8_t watch_face_index, void **context_ptr);
void sailing_conversions_face_activate(void *context);
bool sailing_conversions_face_loop(movement_event_t event, void *context);
void sailing_conversions_face_resign(void *context);

#define sailing_conversions_face ((const watch_face_t){ \
    sailing_conversions_face_setup,                     \
    sailing_conversions_face_activate,                  \
    sailing_conversions_face_loop,                      \
    sailing_conversions_face_resign,                    \
    NULL,                                               \
})

#endif // SAILING_CONVERSIONS_FACE_H_
