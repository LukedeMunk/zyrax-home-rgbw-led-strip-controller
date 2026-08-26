# AGENTS.md

## Purpose

This repository contains embedded C++ firmware targeting an ESP32\
microcontroller and the Arduino framework.

Agents must prioritize:

- Correct and predictable hardware behavior.
- Memory safety and efficient resource use.
- Non-blocking execution.
- Reliable recovery from failures.
- Compatibility with the configured ESP32 target.
- Maintainable, testable, and well-documented code.

Inspect the repository before making changes. Determine the target board,\
framework version, build system, dependencies, pin configuration, and existing\
coding conventions from the project files.

## Sources of truth

Use the following precedence when instructions conflict:

1. The explicit task requirements.
2. The nearest applicable `AGENTS.md`.
3. Project documentation and coding-convention files.
4. Build configuration such as `platformio.ini` or `CMakeLists.txt`.
5. Existing patterns in the affected module.
6. General guidance in this document.

If the repository contains a file such as `CPP_CONVENTIONS.md`, treat it as the\
authoritative source for C++ formatting and naming.

Do not perform large-scale formatting or renaming merely to make old code match\
new conventions.

## Project discovery

Before editing, inspect the relevant parts of the repository:

- `/.vscode`
- `CMakeLists.txt`
- Arduino `.ino` entry points
- `src/` and `include/`
- `lib/` and component directories
- `data/` or filesystem assets
- `test/`
- Board, partition, and SDK configuration files
- README and architecture documentation
- Existing coding-convention files

Identify:

- The exact ESP32 variant and board.
- Whether the project uses PlatformIO, Arduino CLI, ESP-IDF components, or a\
  combination.
- The configured Arduino core version.
- Required libraries and pinned versions.
- The flash partition layout.
- Whether a filesystem such as LittleFS is used.
- Hardware interfaces and assigned GPIO pins.
- Supported build environments and feature flags.

Do not assume that code written for one ESP32 variant is compatible with\
another.

## Code organization

Prefer a modular structure:

- Keep the Arduino sketch or application entry point small.
- Put hardware drivers in dedicated classes or modules.
- Separate protocol handling from physical input and output.
- Separate business logic from hardware access.
- Isolate persistence, networking, logging, and security concerns.
- Keep shared types and constants in clearly named headers.
- Give non-trivial classes matching `.h` and `.cpp` files.
- Avoid placing substantial implementations in headers unless templates or\
  compile-time behavior require it.

A typical module structure is:

```text
src/
    main.cpp
    HardwareDriver.cpp
    ProtocolHandler.cpp
include/
    HardwareDriver.h
    ProtocolHandler.h
data/
test/
platformio.ini
```

Existing repository layouts take precedence over this example.

## C++ conventions

Follow the repository's established C++ standard and coding conventions.

Unless the project specifies otherwise:

- Use four spaces for indentation.
- Do not use tabs for indentation.
- Use `PascalCase` for classes, structures, and enum types.
- Use `camelCase` for methods, functions, arguments, and variables.
- Use `MACRO_CASE` for preprocessor constants.
- Use meaningful names and avoid uncommon abbreviations.
- Prefix boolean identifiers with `is`, `has`, `can`, `should`, or a similar\
  predicate.
- Use fixed-width integer types such as `uint8_t`, `int16_t`, and `uint32_t`.
- Place opening braces at the end of declarations and statements.
- Add spaces around operators and after commas.
- Keep functions focused on one responsibility.
- Prefer early returns over deeply nested control flow.
- Use include guards or `#pragma once`, following the existing project style.
- Document public APIs and non-obvious hardware behavior.
- Keep comments focused on intent and constraints, not obvious syntax.

Do not introduce language features unsupported by the configured compiler or\
C++ standard.

## Types and interfaces

Use types that clearly represent their domain:

- Use fixed-width integer types for registers, counters, protocols, and stored\
  formats.
- Use `size_t` for container sizes and memory lengths when appropriate.
- Use `enum class` when scoped enums are compatible with the project style.
- Pass large objects by reference.
- Use `const` wherever it expresses actual immutability.
- Avoid returning pointers to temporary or short-lived data.
- Make ownership and object lifetime explicit.
- Avoid unsafe casts and implicit narrowing conversions.
- Validate data before converting it to a smaller type.

Do not expose hardware-specific details through high-level interfaces unless\
callers genuinely need them.

## Memory constraints

Treat RAM, flash, stack, and heap as limited resources.

Agents must:

- Avoid unnecessary copies of strings, buffers, containers, and JSON documents.
- Reuse buffers where ownership and concurrency are clear.
- Validate lengths before copying, allocating, or reserving memory.
- Keep large objects out of small task stacks.
- Avoid unbounded containers and queues.
- Set explicit maximum sizes for externally supplied data.
- Consider fragmentation before repeatedly allocating variable-sized objects.
- Check allocation and file-open results.
- Prefer bounded processing over loading large files entirely into memory.
- Store constant data in flash where appropriate and supported.

Be especially careful with:

- Arduino `String` concatenation in frequently executed code.
- Large local arrays.
- Dynamic JSON documents.
- Pulse buffers and sample collections.
- Network response bodies.
- Recursive functions.

Do not replace clear, safe code with manual memory management unless profiling\
or a concrete constraint justifies it.

## Main loop and scheduling

Keep `loop()` and recurring processing functions responsive.

Avoid:

- Long blocking delays.
- Infinite or unbounded polling loops.
- Synchronous network operations without timeouts.
- Large filesystem operations in timing-sensitive paths.
- Repeated work on every loop iteration when event-driven processing is\
  possible.

Prefer:

- State machines.
- Timestamp-based scheduling with `millis()` or `micros()`.
- Bounded timeouts.
- Event queues.
- Small processing steps.
- ESP32 tasks only when concurrency provides a clear benefit.

Handle timer rollover correctly. Do not compare timestamps using arithmetic\
that fails when `millis()` or `micros()` wraps.

## Interrupt service routines

Interrupt service routines must be short, deterministic, and safe.

Inside an ISR:

- Do not allocate memory.
- Do not use `String` or dynamic containers.
- Do not access filesystems.
- Do not perform network operations.
- Do not log through normal serial or logging APIs.
- Do not call blocking functions.
- Avoid floating-point processing.
- Store only the minimum data required for later processing.
- Use the platform-required ISR attribute, such as `ARDUINO_ISR_ATTR`.
- Use ISR-safe synchronization primitives and APIs.

Variables shared between an ISR and normal code must use appropriate\
`volatile`, atomic, or critical-section protection.

Copy captured data to a stable buffer before performing parsing, decoding, or\
logging outside the ISR.

## FreeRTOS and concurrency

ESP32 Arduino applications may use FreeRTOS internally or explicitly.

When creating tasks:

- Document why a separate task is required.
- Assign a justified stack size and priority.
- Avoid unnecessary high-priority tasks.
- Use bounded queues and timeouts.
- Protect shared mutable state.
- Do not hold locks during delays, file operations, or network calls.
- Avoid calling non-thread-safe Arduino APIs concurrently.
- Make task shutdown and resource cleanup explicit when applicable.
- Watch for task starvation and priority inversion.

Do not add concurrency merely to avoid designing a clear state machine.

## Hardware configuration

Treat pin assignments and peripheral configuration as part of the hardware\
interface.

Do not change GPIO assignments unless the task explicitly includes a hardware\
or PCB change.

Before using a pin, verify:

- It exists on the configured ESP32 variant.
- It is exposed by the selected board.
- It does not conflict with flash, PSRAM, USB, JTAG, boot-strapping, or another\
  peripheral.
- It supports the required input, output, ADC, PWM, interrupt, or bus function.
- Its boot-time behavior is safe for the attached hardware.

Keep board-specific values centralized in configuration files.

Avoid hard-coded pin numbers, frequencies, addresses, and calibration values\
inside business logic.

## Hardware buses and peripherals

For GPIO, ADC, PWM, UART, I2C, SPI, I2S, RMT, timers, and other peripherals:

- Check every initialization result where the API provides one.
- Keep bus frequency and mode compatible with all attached devices.
- Protect shared buses when accessed from multiple tasks.
- Release chip-select and enable lines into safe states.
- Use explicit timeouts for peripheral communication.
- Handle disconnected, unresponsive, or partially initialized devices.
- Avoid reinitializing a shared bus from unrelated modules.
- Document timing requirements that are not obvious from the code.

Preserve initialization order when devices depend on power, reset, or bus\
sequencing.

## Timing-sensitive behavior

Do not casually change:

- Radio or carrier frequencies.
- Pulse lengths.
- Protocol timing tolerances.
- Sampling intervals.
- Debounce times.
- Watchdog timeouts.
- Retry intervals.
- Peripheral clock rates.

When changing timing:

- Explain the reason.
- Define acceptable ranges.
- Consider integer overflow and timer resolution.
- Verify the change on hardware when possible.
- Do not claim hardware validation based only on a successful build.

Use `delayMicroseconds()` and direct timing loops only when the blocking behavior\
is understood and acceptable.

## Networking

Network code must tolerate temporary failure.

Agents must:

- Configure connection and request timeouts.
- Avoid blocking indefinitely while connecting.
- Use bounded retry behavior.
- Apply backoff where repeated failures could overload the network.
- Validate response status, length, and content.
- Handle partial, malformed, or empty responses.
- Avoid assuming Wi-Fi remains connected.
- Keep local device behavior functional when remote services are unavailable,\
  where the product requirements allow it.
- Close clients and release resources after use.

Do not log passwords, tokens, authorization headers, private keys, or complete\
credentials.

## Persistent storage

Persistent configuration is part of the firmware interface.

This applies to:

- LittleFS or SPIFFS files.
- Preferences or NVS entries.
- EEPROM emulation.
- SD-card files.
- Stored JSON or binary records.

When changing stored data:

- Preserve backward compatibility when practical.
- Provide safe defaults for missing fields.
- Validate types, lengths, ranges, and collection sizes.
- Handle missing, empty, malformed, and partially written data.
- Do not silently rename or remove persisted keys.
- Document migrations when compatibility cannot be maintained.
- Prefer atomic or recoverable updates.
- Consider flash wear before writing frequently.
- Avoid writing unchanged values repeatedly.

Do not format or erase persistent storage automatically unless product behavior\
explicitly requires it and recovery is clearly defined.

## Filesystem assets

Directories such as `data/` may be uploaded separately from firmware.

When editing filesystem assets:

- Preserve paths referenced by firmware, HTML, CSS, and JavaScript.
- Check storage size against the configured partition.
- Avoid adding large dependencies without justification.
- Do not modify minified third-party files unless explicitly required.
- Do not embed secrets or environment-specific credentials.
- Remember that building firmware may not build or upload the filesystem image.

Report when a separate filesystem upload is required.

## Security

Treat all network, serial, radio, file, and user-controlled input as untrusted.

Agents must:

- Validate lengths and ranges before use.
- Reject malformed input safely.
- Avoid fixed-size buffer overflows.
- Avoid constructing commands or paths from unchecked data.
- Use established cryptographic libraries.
- Do not implement custom cryptographic algorithms.
- Do not hard-code production secrets.
- Avoid exposing sensitive values through logs or diagnostics.
- Preserve certificate and hostname verification unless the task explicitly\
  defines another trust model.
- Consider replay, authentication, and authorization for remote commands.

Do not weaken security checks merely to make development or testing easier.

## Logging

Use the existing project logging abstraction.

Choose log levels consistently:

- Debug for development details.
- Info for meaningful lifecycle events.
- Warning for recoverable abnormal conditions.
- Error for failed operations.

Do not:

- Log inside interrupt handlers.
- Log on every loop iteration.
- Log high-frequency events without throttling.
- Log secrets, credentials, or sensitive payloads.
- Write high-frequency logs to flash.
- introduce a second logging mechanism without a clear reason.

Include enough context to diagnose a failure without exposing sensitive data.

## Error handling and recovery

Embedded systems must fail safely.

For operations that can fail:

- Check the result.
- Preserve a safe hardware state.
- Release acquired resources.
- Emit an appropriate diagnostic.
- Retry only when retrying can reasonably help.
- Bound all retry loops.
- Decide whether degraded operation is possible.
- Avoid reboot loops caused by persistent failures.

Use watchdog resets as a last recovery mechanism, not as normal control flow.

## Power and reset behavior

Consider the behavior during:

- Power-on.
- Brownout.
- Software restart.
- Watchdog reset.
- Deep-sleep wake.
- Partial peripheral initialization.
- Interrupted storage writes.

GPIO outputs must enter safe states as early as necessary. Do not assume RAM,\
peripheral registers, or external device state survives a reset.

## Dependencies

Dependencies belong in the project's configured dependency manager.

Before adding a library:

- Confirm that existing framework functionality is insufficient.
- Verify ESP32 and Arduino compatibility.
- Prefer maintained libraries with clear licensing.
- Pin or constrain versions according to repository policy.
- Consider flash, RAM, build time, and transitive dependencies.
- Avoid multiple libraries that provide the same function.
- Do not edit downloaded dependency files.

Update documentation when a new dependency changes setup requirements.

## Build and verification

Determine the actual build system from the repository.

### Arduino CLI

Typical commands are:

```shell
arduino-cli compile --fqbn <board-fqbn> <sketch>
arduino-cli upload --fqbn <board-fqbn> -p <port> <sketch>
```

Do not invent an environment, board identifier, port, or upload method. Read it\
from the project or report that it must be supplied.

For each change:

1. Build every affected environment.
2. Review warnings in the changed code.
3. Run available automated tests.
4. Run static analysis or formatting checks configured by the repository.
5. Verify filesystem images separately when their contents changed.
6. Identify hardware tests required for the affected behavior.

A successful compile does not prove correct electrical, timing, RF, IR, Wi-Fi,\
Bluetooth, sleep, or flash behavior.

## Testing

Prefer testing hardware-independent logic outside the hardware layer.

Good candidates for unit tests include:

- Protocol parsing.
- Checksums and encoders.
- Filtering and deduplication.
- State machines.
- Configuration validation.
- Serialization and migration.
- Date and time calculations.
- Retry and timeout decisions.

Hardware-dependent tests should define:

- Required board and peripherals.
- Wiring and power requirements.
- Initial device state.
- Test actions.
- Expected serial output or physical behavior.
- Reset or cleanup steps.

Do not claim tests passed if they were not executed.

## Change discipline

Agents must:

- Inspect declarations, definitions, configuration, and call sites before\
  editing.
- Make the smallest coherent change that satisfies the task.
- Preserve public interfaces unless change is necessary.
- Update all callers when an interface changes.
- Avoid unrelated cleanup and formatting.
- Preserve feature flags and alternative build environments.
- Keep hardware-specific logic isolated.
- Document assumptions that cannot be verified.
- Report commands executed and verification results.
- Clearly distinguish build verification from hardware verification.

Do not overwrite unrelated user changes in the working tree.

## Prohibited actions

Unless explicitly requested and justified, do not:

- Change board or framework versions.
- Change GPIO assignments.
- Change partition layouts.
- Disable watchdogs or brownout protection.
- Weaken TLS or authentication.
- Erase persistent storage.
- Add production credentials.
- Modify vendored libraries.
- Introduce unbounded loops, queues, or allocations.
- Add blocking work to timing-sensitive paths.
- Perform broad refactoring during a focused bug fix.
- Claim hardware behavior was verified without using the hardware.

## Completion checklist

Before considering a change complete, confirm that:

- The requested behavior is implemented.
- The correct target and build environment were used.
- The code follows the applicable project conventions.
- Header declarations and source definitions agree.
- Interrupt and concurrent code remains safe.
- Main processing paths remain responsive.
- Memory and stack impact were considered.
- GPIO and peripheral behavior remain compatible with the hardware.
- Persistent data compatibility was considered.
- Input validation and security implications were considered.
- No credentials or secrets were introduced.
- Firmware builds successfully, or the exact blocker is reported.
- Tests were run where available.
- Required filesystem and hardware verification is clearly stated.
