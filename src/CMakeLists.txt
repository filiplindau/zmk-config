# This directly injects your files into the main ZMK app target, bypassing library walls
target_sources(app PRIVATE last_key_tracker.c behavior_contextual.c)