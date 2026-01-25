---
name: keyboard-spec-parser
description: "This skill parses keyboard product specification documents in Word format and generates structured, development-ready feature lists in Markdown format. Use this skill when the user provides a keyboard product definition document (docx) and requests extraction of technical requirements, feature specifications, or functional requirements. The skill ensures NO feature is omitted and actively clarifies ambiguous requirements through user questions."
---

# Keyboard Spec Parser

## Overview

Parse keyboard product specification documents and generate comprehensive, structured feature
lists suitable for firmware development and testing. The output organizes features by module,
assigns priorities, and identifies which components (hardware/firmware/APP) are involved.

## Critical Rules

1. **NO FEATURE OMISSION**: Every feature, parameter, and specification in the source document
   MUST appear in the output. Cross-check each section of the source document against the
   output before finalizing.

2. **CLARIFY AMBIGUITIES**: When encountering unclear, ambiguous, or potentially conflicting
   requirements, STOP and ask the user for clarification before proceeding.

3. **FOCUS ON TECHNICAL REQUIREMENTS**: Ignore marketing language and focus on implementable
   technical specifications.

## Workflow

### Step 1: Read the Source Document

Convert the Word document to readable format using pandoc:

```bash
pandoc "<input.docx>" -o "<output.md>"
```

Read the converted markdown file to analyze the content.

### Step 2: Extract Project Overview

Identify and extract basic product information:
- Product model number
- Connection method (Bluetooth/USB/2.4G)
- Supported operating systems
- Keyboard layout
- Supported language variants (US, DE, UK, JP, FR, etc.)
- Touchpad presence and type
- Backlight type
- Main controller chips
- Battery specifications
- Bluetooth device name

### Step 3: Identify All Functional Modules

Systematically scan the document for these module categories:

1. **System Compatibility** - Supported OS, typing, hotkeys, touchpad support
2. **Key Matrix** - Language variants, special keys, key combinations per variant
3. **Backlight** - Colors, brightness levels, timeout, memory behavior
4. **Bluetooth** - Pairing, reconnection, polling, timeout, power management
5. **Indicators** - LED positions, colors, flash patterns, meanings
6. **Touchpad** - Touch points, gestures, button types, special features
7. **Power Management** - Power on/off, battery, sleep, current consumption
8. **APP Features** - OTA updates, customization, settings

### Step 4: Clarify Ambiguities

For each unclear requirement, ask the user directly. Common areas requiring clarification:

- **Bluetooth channels**: If document mentions "channel switching" but also says "single channel",
  confirm with user which is correct
- **Reconnection vs Polling**: Clarify if "reconnection polling" means standard reconnection
  or advertising-based polling that accepts new connections
- **Custom key functions**: Get the complete list of assignable functions
- **Gesture customization**: Clarify which gestures are customizable and the gesture system used
- **Firmware upgrade method**: Confirm OTA vs wired upgrade support

Example clarification questions:
```
Before proceeding, I need to clarify a few points:

1. The document mentions both "Bluetooth channel switching" and "single channel".
   Which is correct - single channel only, or multi-channel support?

2. For "reconnection polling mechanism", does this mean:
   - Standard reconnection (only reconnects to paired device), or
   - Polling/advertising mode (can accept new connections while trying to reconnect)?

3. What functions can be assigned to the customizable key? Please provide the complete list.
```

### Step 5: Generate Feature List

Output the feature list following the template structure in `references/feature_list_template.md`.

Each feature entry must include:
- **Feature Name**: Concise, descriptive name
- **Description**: Brief technical description
- **Priority**: P0 (critical), P1 (important), P2 (nice-to-have)
- **Module**: Hardware / Firmware / APP

### Step 6: Verification Checklist

Before finalizing, verify coverage against source document sections:

- [ ] All product overview parameters captured
- [ ] All language variants and their special keys documented
- [ ] All key combinations listed (Fn+X, Opt+X, etc.)
- [ ] All backlight specifications included
- [ ] All Bluetooth parameters and behaviors documented
- [ ] All indicator states and patterns defined
- [ ] All touchpad features listed
- [ ] All power/battery specifications included
- [ ] All APP features documented
- [ ] All timing parameters (timeouts, delays) captured

## Domain Knowledge

Refer to `references/keyboard_domain_knowledge.md` for:
- Common Bluetooth keyboard terminology and defaults
- Standard HID usage codes for media keys
- Typical reconnection/polling behavior patterns
- Power management conventions

## Output Format

Save the generated feature list as Markdown to the user-specified path. The filename should
follow the pattern: `<ProductModel>_功能清单.md`

Example output path: `docs/N0046_功能清单.md`
