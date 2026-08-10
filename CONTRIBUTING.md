# Contributing

Contributions and reproducible bug reports are welcome.

1. Create a focused branch and keep unrelated generated files out of the change.
2. Update the smallest affected example or shared tool.
3. Run the relevant discovery tests and framework builds.
4. Describe the board revision, example path, framework version, and hardware validation performed.

Pull requests that change pins, display timing, touch orientation, audio routing, storage, IO polarity, partition tables, or board options must cite the schematic or hardware reference used. A successful compile is not sufficient hardware validation.

When adding an example, place ESP-IDF projects directly under `examples/esp-idf/` or Arduino sketches directly under `examples/arduino/`. Do not add generated `build/`, `sdkconfig`, `managed_components/`, dependency lock, packaged ZIP, or downloaded artifact files.

Reusable fixes belong in the corresponding managed component when possible. If a local workaround is necessary, document why it remains product-specific and what must happen before it can be removed.
