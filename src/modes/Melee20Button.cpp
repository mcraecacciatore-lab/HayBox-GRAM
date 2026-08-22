#include "modes/Melee20Button.hpp"

#define ANALOG_STICK_MIN 48
#define ANALOG_STICK_NEUTRAL 128
#define ANALOG_STICK_MAX 208

Melee20Button::Melee20Button() : ControllerMode() {
    _horizontal_socd = false;
}

void Melee20Button::SetConfig(GameModeConfig &config, const MeleeOptions options) {
    InputMode::SetConfig(config);
    _options = options;
}

void Melee20Button::HandleSocd(InputState &inputs) {
    _horizontal_socd = inputs.lf3 && inputs.lf1;
    InputMode::HandleSocd(inputs);
}

void Melee20Button::UpdateDigitalOutputs(const InputState &inputs, OutputState &outputs) {
    outputs.a = inputs.rt1;
    outputs.b = inputs.rf1;
    outputs.x = inputs.rf2;
    outputs.y = inputs.rf6;
    outputs.buttonR = inputs.rf3;

    if (inputs.nunchuk_connected) {
        outputs.triggerLDigital = inputs.nunchuk_z;
    } else {
        outputs.triggerLDigital = inputs.lf4;
    }

    outputs.triggerRDigital = inputs.rf5;
    outputs.start = inputs.mb1;

    // Hidden Buttons for remapp.ing options
    // L3 / lf6 is repurposed as the dedicated shield-drop modifier.
    // outputs.leftStickClick = inputs.lf6;
    outputs.rightStickClick = inputs.lf7;
    outputs.capture = inputs.lf8;
    outputs.dpadUp = inputs.lf9;
    outputs.dpadDown = inputs.lf10;
    outputs.dpadLeft = inputs.lf11;
    outputs.dpadRight = inputs.lf12;
    outputs.select = inputs.mb2;
    outputs.home = inputs.mb3;

    // Activate D-Pad layer by holding Mod X + Mod Y or Nunchuk C button.
    if ((inputs.lt1 && inputs.lt2) || inputs.nunchuk_c || inputs.lf5) {
        outputs.dpadUp = inputs.rt4;
        outputs.dpadDown = inputs.rt2;
        outputs.dpadLeft = inputs.rt3;
        outputs.dpadRight = inputs.rt5;
    }
}

void Melee20Button::UpdateAnalogOutputs(const InputState &inputs, OutputState &outputs) {
    // Coordinate calculations to make modifier handling simpler.
    UpdateDirections(
        inputs.lf3, // Left
        inputs.lf1, // Right
        inputs.lf2, // Down
        inputs.rf4, // Up
        inputs.rt3, // C-Left
        inputs.rt5, // C-Right
        inputs.rt2, // C-Down
        inputs.rt4, // C-Up
        ANALOG_STICK_MIN,
        ANALOG_STICK_NEUTRAL,
        ANALOG_STICK_MAX,
        outputs
    );

    bool shield_button_pressed = inputs.lf4 || inputs.rf5 || inputs.rf7 || inputs.rf8;

    // Normal digital diagonals.
    //
    // Lower diagonal:
    // 67 / 43 = DBOOC-style geometry based on the keyboard-controller feel.
    //
    // Upper diagonal:
    // Keep stock 56 / 56 behavior.
    if (directions.diagonal) {
        if (directions.y == -1) {
            outputs.leftStickX = 128 + (directions.x * 67);
            outputs.leftStickY = 128 + (directions.y * 43);
        } else {
            outputs.leftStickX = 128 + (directions.x * 56);
            outputs.leftStickY = 128 + (directions.y * 56);
        }
    }

    /* Mod X */
    if (inputs.lt1) {
        // MX + Horizontal = 6625 = 53
        if (directions.horizontal) {
            outputs.leftStickX = 128 + (directions.x * 53);
        }

        // MX + Vertical = 5375 = 43
        if (directions.vertical) {
            outputs.leftStickY = 128 + (directions.y * 43);
        }

        if (directions.diagonal && shield_button_pressed) {
            // Use custom airdodge angle if set, otherwise B0XX standard default.
            // A later lower-diagonal override gives Mod X its dedicated max-WD
            // coordinate.
            if (_options.has_custom_airdodge) {
                outputs.leftStickX =
                    128 + (directions.x * _options.custom_airdodge.x);
                outputs.leftStickY =
                    128 + (directions.y * _options.custom_airdodge.y);
            } else {
                // MX + L, R, LS, and MS + diagonal = 6375 / 3750
                outputs.leftStickX = 128 + (directions.x * 51);
                outputs.leftStickY = 128 + (directions.y * 30);
            }
        }

        /* Up B angles */
        if (directions.diagonal && !shield_button_pressed) {
            // 22.9638 - 7375 3125 = 59 25
            outputs.leftStickX = 128 + (directions.x * 59);
            outputs.leftStickY = 128 + (directions.y * 25);

            // 27.37104 - 7000 3625 = 56 29
            if (inputs.rt2) {
                outputs.leftStickX = 128 + (directions.x * 56);
                outputs.leftStickY = 128 + (directions.y * 29);
            }

            // 31.77828 - 7875 4875 = 63 39
            if (inputs.rt3) {
                outputs.leftStickX = 128 + (directions.x * 63);
                outputs.leftStickY = 128 + (directions.y * 39);
            }

            // 36.18552 - 7000 5125 = 56 41
            if (inputs.rt4) {
                outputs.leftStickX = 128 + (directions.x * 56);
                outputs.leftStickY = 128 + (directions.y * 41);
            }

            // 40.59276 - 6125 5250 = 49 42
            if (inputs.rt5) {
                outputs.leftStickX = 128 + (directions.x * 49);
                outputs.leftStickY = 128 + (directions.y * 42);
            }

            /* Extended Up B Angles */
            if (inputs.rf1) {
                // 22.9638 - 9125 3875 = 73 31
                outputs.leftStickX = 128 + (directions.x * 73);
                outputs.leftStickY = 128 + (directions.y * 31);

                // 27.37104 - 8750 4500 = 70 36
                if (inputs.rt2) {
                    outputs.leftStickX = 128 + (directions.x * 70);
                    outputs.leftStickY = 128 + (directions.y * 36);
                }

                // 31.77828 - 8500 5250 = 68 42
                if (inputs.rt3) {
                    outputs.leftStickX = 128 + (directions.x * 68);
                    outputs.leftStickY = 128 + (directions.y * 42);
                }

                // 36.18552 - 7375 5375 = 59 43
                if (inputs.rt4) {
                    outputs.leftStickX = 128 + (directions.x * 59);
                    outputs.leftStickY = 128 + (directions.y * 43);
                }

                // 40.59276 - 6375 5375 = 51 43
                if (inputs.rt5) {
                    outputs.leftStickX = 128 + (directions.x * 51);
                    outputs.leftStickY = 128 + (directions.y * 43);
                }
            }
        }

        // Angled fsmash
        if (directions.cx != 0) {
            // 8500 5250 = 68 42
            outputs.rightStickX = 128 + (directions.cx * 68);
            outputs.rightStickY = 128 + (directions.y * 42);
        }
    }

    /*
     * Mod Y - Tilt + Shield Control
     *
     * Horizontal = 53
     * Vertical   = 43
     * Diagonal   = 51 / 30
     *
     * Same modifier can therefore be used for controlled tilts and
     * deliberate shield movement.
     */
    if (inputs.lt2) {
        // Horizontal: f-tilt / horizontal shield movement
        if (directions.horizontal) {
            outputs.leftStickX = 128 + (directions.x * 53);
        }

        // Vertical: up/down tilt / vertical shield movement
        if (directions.vertical) {
            outputs.leftStickY = 128 + (directions.y * 43);
        }

        // Diagonal: angled tilt / diagonal shield movement
        if (directions.diagonal) {
            outputs.leftStickX = 128 + (directions.x * 51);
            outputs.leftStickY = 128 + (directions.y * 30);
        }
    }

    /*
     * Mod X - dedicated maximum-wavedash lower diagonal.
     *
     * 75 / 26 is approximately 70.9 degrees measured from straight Down,
     * matching the ~70-72 degree convention we're using.
     *
     * This is deliberately placed AFTER the normal Mod X processing so it
     * gets final say whenever Mod X + a lower diagonal is held.
     */
    if (inputs.lt1 && directions.diagonal && directions.y == -1) {
        outputs.leftStickX = 128 + (directions.x * 75);
        outputs.leftStickY = 128 + (directions.y * 26);
    }

    /*
     * Dedicated shield-drop modifier.
     *
     * L3 / lf6 does nothing special by itself.
     * Shield + Down + L3 outputs the dedicated straight-down shield-drop
     * coordinate.
     */
    if (inputs.lf6 && shield_button_pressed &&
        directions.vertical && directions.y == -1) {
        outputs.leftStickX = 128;
        outputs.leftStickY = 73;
    }

    // C-stick ASDI Slideoff angle overrides any other C-stick modifiers
    // (such as angled fsmash).
    if (directions.cx != 0 && directions.cy != 0) {
        // 5250 8500 = 42 68
        outputs.rightStickX = 128 + (directions.cx * 42);
        outputs.rightStickY = 128 + (directions.cy * 68);
    }

    // Horizontal SOCD overrides X-axis modifiers
    // (for ledgedash maximum jump trajectory).
    if (!_options.disable_ledgedash_socd_override &&
        _horizontal_socd &&
        !directions.vertical) {
        outputs.leftStickX = 128 + (directions.x * 80);
    }

    if (inputs.rf7) {
        outputs.triggerRAnalog = 49;
    }

    if (inputs.rf8) {
        outputs.triggerRAnalog = 94;
    }

    if (outputs.triggerLDigital) {
        outputs.triggerLAnalog = 140;
    }

    if (outputs.triggerRDigital) {
        outputs.triggerRAnalog = 140;
    }

    // Shut off C-stick when using D-Pad layer.
    if ((inputs.lt1 && inputs.lt2) || inputs.nunchuk_c || inputs.lf5) {
        outputs.rightStickX = 128;
        outputs.rightStickY = 128;
    }

    // Nunchuk overrides left stick.
    if (inputs.nunchuk_connected) {
        outputs.leftStickX = inputs.nunchuk_x;
        outputs.leftStickY = inputs.nunchuk_y;
    }
}
