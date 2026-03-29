#include <zephyr/kernel.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>

// This is the global variable we pull into behavior_contextual.c using 'extern'
uint32_t last_pressed_keycode = 0;

int last_key_tracker_listener(const zmk_event_t *eh) {
    // Cast the generic event to a keycode_state_changed event
    const struct zmk_keycode_state_changed *ev = (const struct zmk_keycode_state_changed *)eh;
    
    // We only want to track PRESS events (state == true), not releases (state == false)
    if (ev && ev->state) {
        
        // --- THE MODIFIER MASK ---
        // ZMK keycodes are 32-bit integers. If you press Shift+Q, ZMK adds 
        // modifier flags to the upper bits of the keycode. 
        // The base character (the HID Usage ID for 'Q') lives in the lowest 8 bits.
        // By using the bitwise AND operator (&) with 0xFF, we strip away all modifiers.
        // Therefore, 'Shift+Q', 'Ctrl+Q', and 'Q' all become just 'Q'.
        
        last_pressed_keycode = ev->keycode & 0xFF;
    }
    
    // Return BUBBLE so the event continues to be processed by the rest of the keyboard.
    // If you returned ZMK_EV_EVENT_HANDLED, you would actually block the key from typing!
    return ZMK_EV_EVENT_BUBBLE; 
}

// --- THE HOOKS ---
// 1. Define the listener instance and point it to our function
ZMK_LISTENER(last_key_tracker, last_key_tracker_listener);

// 2. Subscribe our listener to ZMK's central keycode event stream
ZMK_SUBSCRIPTION(last_key_tracker, zmk_keycode_state_changed);