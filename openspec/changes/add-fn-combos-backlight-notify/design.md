## Context
We are extending the combo subsystem with a small set of Fn shortcuts that interact with backlight, battery status, and Siri/assistant invocation. The implementation must stay within the middleware layer boundaries and avoid dragging application headers into the combo handlers.

## Goals / Non-Goals
- Goals:
  - Add the four requested Fn combo entry points.
  - Keep combo actions self-contained and driver-facing.
  - Provide a narrow keyboard-layer wrapper for backlight activity notification.
  - Fix the existing `earth_post_loop_decision` signature mismatch.
- Non-Goals:
  - No changes to unrelated combo families.
  - No cross-layer refactor outside the five requested files.
  - No new storage or settings persistence for these shortcuts.

## Decisions
- Decision: Map the four requested Fn combos to the four requested actions one-to-one.
  - `Fn + Right Shift` -> `Backlight_Level_Up`
  - `Fn + Right Enter` -> `Backlight_Color_Next`
  - `Fn + Right Cmd` -> `Siri_Invoke`
  - `Fn + KC_APPLICATION` -> `Battery_Check`
- Decision: Implement `keyboard_note_backlight_activity()` in `keyboard.c` as the only combo-side notification entry point.
  - Rationale: combo actions can call a middleware function without depending on application headers.
  - Likely backing call: `lpm_note_activity()`, which already exists in middleware and fits the "activity" semantics.
- Decision: Define `M_SIRI` locally in the Fn action module if the codebase does not already provide it.
  - Rationale: keeps the consumer usage explicit while staying inside the allowed file scope.
- Decision: Fix the Earth post-loop declaration to accept both `key_list` and `key_list_extend`.
  - Rationale: this matches the current call site in `kb_combo_engine.c` and preserves Apple-platform Earth handling.

## Risks / Trade-offs
- Battery blink count semantics are slightly ambiguous from the request text.
  - Proposed interpretation: use a 1-4 blink count based on battery percentage bands.
- The wrapper name suggests a backlight-specific notification, but the immediate implementation may route to the generic activity timer first.
  - This is low risk because it stays inside middleware and preserves future extensibility.

## Migration Plan
- Update the combo map and Fn action APIs first.
- Add the keyboard wrapper and signature fix next.
- Keep the consumer usage definition local unless a broader shared macro is later requested.

## Open Questions
- Should the battery check map percentage to blink count by quartiles, or do you want a different threshold table?
- Do you want `keyboard_note_backlight_activity()` to remain a thin alias for `lpm_note_activity()`, or should it also notify an output-layer hook later?

