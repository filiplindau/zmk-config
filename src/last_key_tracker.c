#include <zephyr/kernel.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>

// This is the global variable we pull into behavior_contextual.c using 'extern'
uint32_t last_pressed_keycode = 0;


// Pure Zbus callback function
static void last_key_tracker_cb(const struct zbus_channel *chan) {
    const struct zmk_keycode_state_changed *ev = zbus_chan_const_msg(chan);
    if (ev && ev->state) {
        // --- THE MODIFIER MASK ---
        // ZMK keycodes are 32-bit integers. If you press Shift+Q, ZMK adds 
        // modifier flags to the upper bits of the keycode. 
        // The base character (the HID Usage ID for 'Q') lives in the lowest 8 bits.
        // By using the bitwise AND operator (&) with 0xFF, we strip away all modifiers.
        // Therefore, 'Shift+Q', 'Ctrl+Q', and 'Q' all become just 'Q'.
        last_pressed_keycode = ev->keycode & 0xFF;
    }
}

// --- THE HOOKS ---
// Register the listener directly with Zephyr's Zbus system
ZBUS_LISTENER_DEFINE(last_key_tracker, last_key_tracker_cb);
ZBUS_CHAN_ADD_OBS(zmk_keycode_state_changed, last_key_tracker, 1);