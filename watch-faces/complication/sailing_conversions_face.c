/*
 * MIT License
 *
 * Copyright (c) 2023 PrimmR
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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sailing_conversions_face.h"

#include "watch_common_display.h"

typedef struct
{
    char name[6];          // Name to display on selection
    double conv_factor; // Unit as represented in base units
    int16_t linear_factor; // Addition of constant (For temperatures)
} sailing_unit;

#define TICK_FREQ 4

#define MEASURES_COUNT 2 // Number of different measurement 'types'
#define SPEED 0
#define DISTANCE 1

// Names of measurements (classic & custom LCD)
static char sailing_measures[MEASURES_COUNT][7] = {"speed", "dist"};
static char sailing_measures_custom[MEASURES_COUNT][7] = {"speed", "dist"};

// Number of items in each category
#define SPEED_COUNT 4
#define DISTANCE_COUNT 2
const uint8_t sailing_units_count[2] = {SPEED_COUNT, DISTANCE_COUNT};

static const sailing_unit speeds[SPEED_COUNT] = {
    {" n&/s", 1.0, 0},
    {"kn&/h", 1000./3600., 0},
    {"  kn", 1852./3600., 0},
    {" bft", 1.0, 0},
};

static const sailing_unit distances[DISTANCE_COUNT] = {
    {" kn&", 1.0, 0},
    {" nn&", 1.852, 0},
};

static int8_t calc_success_seq[5] = {BUZZER_NOTE_G6, 10, BUZZER_NOTE_C7, 10, 0};
static int8_t calc_fail_seq[5] = {BUZZER_NOTE_C7, 10, BUZZER_NOTE_G6, 10, 0};

//static int8_t terry_sec[] = {BUZZER_NOTE_C5, 20, BUZZER_NOTE_G5, 20, BUZZER_NOTE_F5, 20, BUZZER_NOTE_D5, 10, BUZZER_NOTE_A4, 10, BUZZER_NOTE_C5, 30, BUZZER_NOTE_G4, 10, BUZZER_NOTE_G5, 20, BUZZER_NOTE_D5, 20, BUZZER_NOTE_C5, 40, BUZZER_NOTE_D5, 40, BUZZER_NOTE_G5, 20, BUZZER_NOTE_G5, 10, BUZZER_NOTE_G5, 10, BUZZER_NOTE_G4, 40, 0};
static int8_t terry_sec[] = {BUZZER_NOTE_C5, 15, BUZZER_NOTE_G5, 15, BUZZER_NOTE_F5, 15, BUZZER_NOTE_D5, 8, BUZZER_NOTE_A4, 8, BUZZER_NOTE_C5, 22, BUZZER_NOTE_G4, 8, BUZZER_NOTE_G5, 15, BUZZER_NOTE_D5, 15, BUZZER_NOTE_C5, 30, BUZZER_NOTE_D5, 30, BUZZER_NOTE_G5, 15, BUZZER_NOTE_G5, 8, BUZZER_NOTE_G5, 8, BUZZER_NOTE_G4, 30, BUZZER_NOTE_C5, 15, BUZZER_NOTE_G5, 15, BUZZER_NOTE_F5, 15, BUZZER_NOTE_D5, 8, BUZZER_NOTE_A4, 8, BUZZER_NOTE_C5, 22, BUZZER_NOTE_G4, 8, BUZZER_NOTE_G5, 15, BUZZER_NOTE_D5, 15, BUZZER_NOTE_C5, 30, BUZZER_NOTE_D5, 30, BUZZER_NOTE_G5, 15, BUZZER_NOTE_G5, 8, BUZZER_NOTE_G5, 8, BUZZER_NOTE_G4, 30, BUZZER_NOTE_G5, 15, BUZZER_NOTE_E5, 15, BUZZER_NOTE_C5, 15, BUZZER_NOTE_G5, 15, BUZZER_NOTE_C5, 15, BUZZER_NOTE_A4, 15, BUZZER_NOTE_F5, 15, BUZZER_NOTE_C5, 15, BUZZER_NOTE_C5, 8, BUZZER_NOTE_B4, 8, BUZZER_NOTE_C5, 8, BUZZER_NOTE_B4, 8, BUZZER_NOTE_G5, 22, BUZZER_NOTE_G5, 8, BUZZER_NOTE_G4, 30, BUZZER_NOTE_B4, 30, BUZZER_NOTE_G5, 15, BUZZER_NOTE_E5, 15, BUZZER_NOTE_C5, 15, BUZZER_NOTE_G5, 15, BUZZER_NOTE_C5, 15, BUZZER_NOTE_A4, 15, BUZZER_NOTE_F5, 15, BUZZER_NOTE_C5, 15, BUZZER_NOTE_C5, 8, BUZZER_NOTE_B4, 8, BUZZER_NOTE_C5, 8, BUZZER_NOTE_B4, 8, BUZZER_NOTE_G5, 22, BUZZER_NOTE_G5, 8, BUZZER_NOTE_G4, 30, BUZZER_NOTE_B4, 30, 0};

// Resets all state variables to 0
static void reset_state(sailing_conversions_state_t *state)
{
    state->pg = sailing_measurement;
    state->measurement_i = 0;
    state->from_i = 0;
    state->to_i = 0;
    state->selection_value = 0;
    state->selection_index = 0;
    state->alarm_held = false;
}

void sailing_conversions_face_setup(uint8_t watch_face_index, void **context_ptr)
{
    (void)watch_face_index;
    if (*context_ptr == NULL)
    {
        *context_ptr = malloc(sizeof(sailing_conversions_state_t));
        memset(*context_ptr, 0, sizeof(sailing_conversions_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void sailing_conversions_face_activate(void *context)
{
    sailing_conversions_state_t *state = (sailing_conversions_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.
    movement_request_tick_frequency(TICK_FREQ);

    reset_state(state);
}

// Increments index pointer by 1, wrapping
#define increment_wrapping(index, wrap) ({(index)++; index %= wrap; })

static uint32_t pow_10(uint8_t n)
{
    uint32_t result = 1;
    for (int i = 0; i < n; i++)
    {
        result *= 10;
    }
    return result;
}

// Returns correct list of units for the measurement index
static sailing_unit *get_unit_list(uint8_t measurement_i)
{
    switch (measurement_i)
    {
    case SPEED:
        return (sailing_unit *)speeds;
    case DISTANCE:
        return (sailing_unit *)distances;
    default:
        return (sailing_unit *)speeds;
    }
}

// Increment digit by 1 in input (wraps)
static void increment_input(sailing_conversions_state_t *state)
{
    uint8_t digit = state->selection_value / pow_10(DISPLAY_DIGITS - 1 - state->selection_index) % 10;
    if (digit != 9)
    {
        state->selection_value += pow_10(DISPLAY_DIGITS - 1 - state->selection_index);
    }
    else
    {
        state->selection_value -= 9 * pow_10(DISPLAY_DIGITS - 1 - state->selection_index);
    }
}

// Displays the list of units in the selected category
static void display_units(uint8_t measurement_i, uint8_t list_i)
{
    watch_display_text(WATCH_POSITION_BOTTOM, get_unit_list(measurement_i)[list_i].name);
}

static void display(sailing_conversions_state_t *state, uint8_t subsec)
{
    watch_clear_display();

    switch (state->pg)
    {
    case sailing_measurement:
    {
        watch_display_text_with_fallback(WATCH_POSITION_TOP, "Unit", "Un");
        char* measurement_name = (watch_get_lcd_type() == WATCH_LCD_TYPE_CUSTOM ? sailing_measures_custom : sailing_measures)[state->measurement_i];
        watch_display_text(WATCH_POSITION_BOTTOM, measurement_name);
    }
    break;

    case sailing_from:
        display_units(state->measurement_i, state->from_i);
        watch_display_text_with_fallback(WATCH_POSITION_TOP, "Frm", "Fr");
        break;

    case sailing_to:
        display_units(state->measurement_i, state->to_i);
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, " to", "to");
        break;

    case sailing_input:
    {
        char buf[7];
        sprintf(buf, "  %04u", state->selection_value);
        watch_display_text(WATCH_POSITION_BOTTOM, buf);

        // Blink digit (on & off) twice a second
        if (subsec % 2)
        {
            watch_display_character(' ', 6 + state->selection_index);
        }

        watch_display_text_with_fallback(WATCH_POSITION_TOP, "Input", "In");
    }
    break;

    case sailing_result:
    {
        sailing_unit froms = get_unit_list(state->measurement_i)[state->from_i];
        sailing_unit tos = get_unit_list(state->measurement_i)[state->to_i];

        // Converts
        double to_base = (state->selection_value * froms.conv_factor) + 100 * froms.linear_factor;
        if (state->measurement_i == SPEED && state->from_i == 3)
        {
            to_base = 0.836*pow(state->selection_value, 3./2.)/10.;
        }
        double conversion = ((to_base - 100 * tos.linear_factor) / tos.conv_factor);
        if (state->measurement_i == SPEED && state->to_i == 3)
        {
            conversion = 1.12684*pow(to_base*10, 2./3.);
        }

        // If number too large or too small
        if (conversion >= 1000000 || conversion < 0)
        {
            watch_set_indicator(WATCH_INDICATOR_BELL);
            watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, " Error", " Err");

            if (movement_button_should_sound())
                watch_buzzer_play_sequence(calc_fail_seq, NULL);
        }
        else
        {
            uint32_t rounded = conversion + .5;
            char buf[7];
            sprintf(buf, "%6u", rounded);
            watch_display_text(WATCH_POSITION_BOTTOM, buf);

            // Make sure LSDs always filled
            if (rounded < 10)
            {
                watch_display_character('0', 7);
                watch_display_character('0', 8);
            }
            else if (rounded < 100)
            {
                watch_display_character('0', 7);
            }

            if (movement_button_should_sound())
                watch_buzzer_play_sequence(calc_success_seq, NULL);
        }
        watch_display_text_with_fallback(WATCH_POSITION_TOP, "Res =", " =");
    }

    break;

    default:
        break;
    }
}

bool sailing_conversions_face_loop(movement_event_t event, void *context)
{
    sailing_conversions_state_t *state = (sailing_conversions_state_t *)context;

    switch (event.event_type)
    {
    case EVENT_ACTIVATE:
        // Initial UI
        watch_buzzer_play_sequence(terry_sec, NULL);
        display(state, event.subsecond);
        break;
    case EVENT_TICK:
        // Update for blink animation on input
        if (state->pg == sailing_input)
        {
            display(state, event.subsecond);

            // Increments input twice a second when light button held
            if (state->alarm_held && event.subsecond % 2)
                increment_input(state);
        }
        break;
    case EVENT_ALARM_BUTTON_UP:
        // Cycles options
        switch (state->pg)
        {
        case sailing_measurement:
            increment_wrapping(state->measurement_i, MEASURES_COUNT);
            break;

        case sailing_from:
            increment_wrapping(state->from_i, sailing_units_count[state->measurement_i]);
            break;

        case sailing_to:
            increment_wrapping(state->to_i, sailing_units_count[state->measurement_i]);
            if (state->from_i == state->to_i)
            {
                increment_wrapping(state->to_i, sailing_units_count[state->measurement_i]);
            }
            break;

        case sailing_input:
            increment_input(state);
            break;

        default:
            break;
        }

        // Light button does nothing on final screen
        if (state->pg != sailing_result)
            display(state, event.subsecond);

        state->alarm_held = false;

        break;

    case EVENT_LIGHT_BUTTON_DOWN:
        break;

    case EVENT_LIGHT_BUTTON_UP:
        // Increments selected digit
        if (state->pg == sailing_input)
        {

            // Moves between digits in input
            // Wraps at 4 digits unless Bft selected
            if (state->selection_index < (DISPLAY_DIGITS - 1) - 2 * (state->measurement_i == SPEED && state->from_i == 3))
            {
                state->selection_index++;
            }
            else
            {
                state->pg++;
                display(state, event.subsecond);
            }
        }
        // Moves forward 1 page
        else
        {
            if (state->pg == SCREEN_NUM - 1)
            {
                reset_state(state);
            } else if (state->pg == sailing_from && state->from_i == state->to_i)
            {
                increment_wrapping(state->to_i, sailing_units_count[state->measurement_i]);
                state->pg++;
            }
            else
            {
                state->pg++;
            }

            // Play boop
            if (movement_button_should_sound())
                watch_buzzer_play_note(BUZZER_NOTE_C7, 50);
        }

        display(state, event.subsecond);

        state->alarm_held = false;

        break;

    case EVENT_LIGHT_LONG_PRESS:
        // Moves backwards through pages, resetting certain values
        if (state->pg != sailing_measurement)
        {
            switch (state->pg)
            {
            case sailing_measurement:
                state->measurement_i = 0;
                break;

            case sailing_from:
                state->from_i = 0;
                break;

            case sailing_to:
                state->to_i = 0;
                break;

            case sailing_input:
                state->selection_index = 0;
                state->selection_value = 0;
                break;

            case sailing_result:
                state->selection_index = 0;
                break;

            default:
                break;
            }

            state->pg--;
            display(state, event.subsecond);

            // Play beep
            if (movement_button_should_sound())
                watch_buzzer_play_note(BUZZER_NOTE_C8, 50);

            state->alarm_held = false;
        }
        break;

    case EVENT_ALARM_LONG_PRESS:
         // Sets flag to increment input digit when alarm button held
        if (state->pg == sailing_input)
            state->alarm_held = true;

        break;

    case EVENT_ALARM_LONG_UP:
        state->alarm_held = false;
        break;

    case EVENT_TIMEOUT:
        movement_move_to_face(0);
        break;

    default:
        return movement_default_loop_handler(event);
    }

    return true;
}

void sailing_conversions_face_resign(void *context)
{
    (void)context;

    // handle any cleanup before your watch face goes off-screen.
}
