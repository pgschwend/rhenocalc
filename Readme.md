# RhenoCalc

## Architecture

The project now separates UI from behavior/logic:

- `src/pages/` contains Qt widgets, layout, signal/slot wiring, and theme application.
- `src/core/` contains reusable logic and calculations, independent from page layout.

Current core modules:

- `src/core/calculatorcore.*` - calculator engine/state and arithmetic/bitwise behavior.
- `src/core/baseconvertercore.*` - base conversion parsing, bit formatting, and value interpretation helpers.
- `src/core/unitconvertercore.*` - unit category definitions and conversion formulas.
- `src/core/networkcalc.*` - IPv4 parsing, CIDR/mask conversion, subnet calculations, and IP classification.
- `src/core/crchashcore.*` - CRC/hash algorithms and formula metadata.

This makes the logic testable and easier to extend without touching widget code.

