# Embedded Systems Laboratories

PlatformIO-based small labs and examples in C++ (embedded projects).

## Prerequisites
- PlatformIO (CLI or VS Code extension)
- A supported toolchain/board (configured in `platformio.ini`)

## Common commands
- Build: `pio run`
- Upload (flash): `pio run -t upload`
- Monitor serial: `pio device monitor`
- Clean: `pio run -t clean`

## Project layout

- `platformio.ini` — project configuration
- `include/` — public headers
- `src/` — source files and components
  - `components/` — drivers and platform code
  - `labs/` — individual lab examples

## Notes
- A `.gitignore` has been added to exclude PlatformIO build artifacts, editor files, and OS temporary files.
- Use VS Code + PlatformIO extension for an integrated workflow.

## License
Project contains personal lab code — add a license if you plan to publish.
