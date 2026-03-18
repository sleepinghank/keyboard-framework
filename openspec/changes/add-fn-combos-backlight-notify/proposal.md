# Change: Add activity-aware Fn combo actions

## Why
The keyboard combo layer already supports Fn-driven shortcuts, but the requested shortcuts and feedback actions are not wired together yet. We also have a real signature drift in `earth_post_loop_decision`, which is currently called with three arguments while the header still declares two.

## What Changes
- Add four Fn combinations for `Fn + Right Shift`, `Fn + Right Enter`, `Fn + Right Cmd`, and `Fn + KC_APPLICATION`.
- Add new Fn actions for backlight level up, backlight color next, battery check, and Siri invoke.
- Add a keyboard-level `keyboard_note_backlight_activity()` wrapper so combo code can notify activity without including application-layer headers.
- Fix the `earth_post_loop_decision` declaration/definition mismatch so the combo engine and Fn action module agree on the same signature.

## Impact
- Affected specs: `keyboard-combos`
- Affected code:
  - `middleware/keyboard/combo/kb_combo_map.c`
  - `middleware/keyboard/combo/kb_fn_action.c`
  - `middleware/keyboard/combo/kb_fn_action.h`
  - `middleware/keyboard/keyboard.c`
  - `middleware/keyboard/keyboard.h`

