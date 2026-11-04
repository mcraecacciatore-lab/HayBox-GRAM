#include "comms/backend_init.hpp"
#include "config_defaults.hpp"
#include "core/CommunicationBackend.hpp"
#include "core/KeyboardMode.hpp"
#include "core/Persistence.hpp"
#include "core/mode_selection.hpp"
#include "core/pinout.hpp"
#include "core/state.hpp"
#include "input/DebouncedGpioButtonInput.hpp"
#include "input/NunchukInput.hpp"
#include "reboot.hpp"
#include "stdlib.hpp"

#include <config.pb.h>

Config config = default_config;

GpioButtonMapping button_mappings[] = {
    // Left fingers, pinky to index
    { BTN_LF13, 6 }, // L, combined into lf4 in loop1() (stock L)
    { BTN_LF3, 7 },  // Left
    { BTN_LF2, 8 },  // Down
    { BTN_LF1, 9 },  // Right

    // Left thumb
    { BTN_LT1, 10 }, // Mod X
    { BTN_LT2, 11 }, // Mod Y

    { BTN_MB1, 12 }, // Start

    // Right thumb
    { BTN_RT1, 28 }, // A
    { BTN_RT2, 27 }, // C-Down
    { BTN_RT3, 14 }, // C-Left
    { BTN_RT4, 13 }, // C-Up
    { BTN_RT5, 15 }, // C-Right

    // Right top row, index to pinky
    { BTN_RF2, 26 },  // X (stock R)
    { BTN_LF14, 22 }, // L, combined into lf4 in loop1() (stock Y)
    { BTN_RF8, 21 },  // Medium Shield (stock Light Shield)
    { BTN_LF6, 20 },  // Shield Drop (stock Medium Shield)

    // Right bottom row, index to pinky
    { BTN_RF5, 19 }, // R (stock B)
    { BTN_RF3, 18 }, // Z (stock X)
    { BTN_RF1, 17 }, // B (stock Z)
    { BTN_RF4, 16 }, // Up (stock Up)
};
const size_t button_count = sizeof(button_mappings) / sizeof(GpioButtonMapping);

const Pinout pinout = {
    .joybus_data = 2,
    .mux = -1,
    .nunchuk_detect = 3,
    .nunchuk_sda = 4,
    .nunchuk_scl = 5,
};

DebouncedGpioButtonInput<button_count> gpio_input(button_mappings);
NunchukInput *nunchuk = nullptr;

CommunicationBackend **backends = nullptr;
size_t backend_count;
KeyboardMode *current_kb_mode = nullptr;

void setup() {
    static InputState inputs;

    // Create GPIO input source and use it to read button states for checking button holds.
    gpio_input.UpdateInputs(inputs);

    // Check bootsel button hold as early as possible for safety.
    if (inputs.mb1) {
        reboot_bootloader();
    }

    // Turn on LED to indicate firmware booted.
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    // Attempt to load config, or write default config to flash if failed to load config.
    if (!persistence.LoadConfig(config)) {
        persistence.SaveConfig(config);
    }

    // Create Nunchuk input source.
    nunchuk = new NunchukInput(Wire, pinout.nunchuk_detect, pinout.nunchuk_sda, pinout.nunchuk_scl);

    // Create array of input sources to be used.
    static InputSource *input_sources[] = { nunchuk };
    size_t input_source_count = sizeof(input_sources) / sizeof(InputSource *);

    backend_count =
        initialize_backends(backends, inputs, input_sources, input_source_count, config, pinout);

    setup_mode_activation_bindings(config.game_mode_configs, config.game_mode_configs_count);
}

void loop() {
    select_mode(backends, backend_count, config);

    for (size_t i = 0; i < backend_count; i++) {
        backends[i]->SendReport();
    }

    if (current_kb_mode != nullptr) {
        current_kb_mode->SendReport(backends[0]->GetInputs());
    }
}

/* Button inputs are read from the second core */

void setup1() {
    while (backends == nullptr) {
        tight_loop_contents();
    }
}

void loop1() {
    if (backends != nullptr) {
        InputState &inputs = backends[0]->GetInputs();
        gpio_input.UpdateInputs(inputs);
        // Two physical L buttons. Combine them here so releasing one while the other is held
        // keeps L down, which two pins sharing BTN_LF4 in the table would not do.
        inputs.lf4 = inputs.lf13 || inputs.lf14;
    }
}
