# Repository Structure

This repository separates maintained source examples, immutable hardware references, factory recovery files, and generated CI artifacts.

| Path | Maintained content |
| --- | --- |
| `examples/esp-idf/` | One first-party ESP-IDF project per direct child directory |
| `examples/arduino/` | One first-party sketch per direct child directory |
| `examples/arduino/libraries/` | Repository-pinned libraries used by the sketches |
| `config/` | Machine-readable CI and factory-image metadata |
| `scripts/` | Discovery and repository validation utilities |
| `releases/` | Firmware packagers and artifact download helpers |
| `firmware/` | Factory recovery image and runtime SD-card content |
| `hardware/schematics/` | Product schematic PDFs |
| `hardware/dimensions/` | 2D and 3D mechanical references |
| `docs/` | Maintainer and user documentation |
| `.github/` | CI workflows and collaboration templates |

Only direct ESP-IDF projects and direct Arduino sketches are product CI inputs. Nested `CMakeLists.txt` files and sketches below `examples/arduino/libraries/` belong to vendored dependencies and are not independently built.

`build/`, `managed_components/`, dependency locks, packaged archives, and downloaded workflow artifacts are generated locally or by CI and must remain untracked. Factory files under `firmware/` are checked-in release inputs, not outputs of the source-build workflow.

Large reusable drivers currently remain copied in several examples. Their presence is documented in [Components](components.md); moving an example to a managed BSP must be done as a separately validated migration rather than as a directory-only cleanup.
