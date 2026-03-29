// THIS LINE IS CRITICAL: It links the C code to your YAML file's "compatible" string
#define DT_DRV_COMPAT zmk_behavior_contextual

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

// Pull in the global variable from last_key_tracker.c
extern uint32_t last_pressed_keycode;

// Pull the map and fallback directly from the Devicetree (Instance 0)
#define MAP_LEN DT_INST_PROP_LEN(0, map)
static const uint32_t ctx_map[] = DT_INST_PROP(0, map);
static const uint32_t fallback_key = DT_INST_PROP(0, fallback);

// We need to remember what we outputted, so we can release it properly
static uint32_t currently_held_output = 0;

// Standard dummy init function
static int behavior_contextual_init(const struct device *dev) {
    return 0; 
}

// FIX: A clean helper function that correctly outputs keys the "ZMK Way"
static void send_key(uint32_t keycode, bool state) {
    raise_zmk_keycode_state_changed((struct zmk_keycode_state_changed) {
        .usage_page = HID_USAGE_KEY,
        .keycode = keycode,
        .implicit_modifiers = 0,
        .explicit_modifiers = 0,
        .state = state,
    });
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
                                         
    uint32_t key_to_output = fallback_key; // Default to fallback

    // Iterate through the array in steps of 2 [match, output, match, output...]
    for (int i = 0; i < MAP_LEN; i += 2) {
        // ZMK keycodes include modifier flags. For safety, we mask out the base keycode 
        // to ensure a strict comparison (e.g., ignoring if Shift was held)
        if ((last_pressed_keycode & 0xFF) == (ctx_map[i] & 0xFF)) {
            key_to_output = ctx_map[i + 1];
            break;
        }
    }

    if (key_to_output != 0) {
        currently_held_output = key_to_output;
        send_key(key_to_output, true); // Use our new helper
    }
    
    return ZMK_BEHAVIOR_OPAQUE; 
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
                                          
    // Release whatever key we decided to output during the press event
    if (currently_held_output != 0) {
        send_key(currently_held_output, false); // Use our new helper
        currently_held_output = 0;
    }
    
    return ZMK_BEHAVIOR_OPAQUE; 
}

static const struct behavior_driver_api behavior_contextual_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0, behavior_contextual_init, NULL, NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_contextual_driver_api);