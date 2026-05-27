# CarrierPress Architecture

CarrierPress uses a block DSP model. Callers own input, output, and scratch buffers, then pass interleaved mono or stereo frames into processor functions. Each processor keeps explicit state in a struct with `init`, `reset`, and `process` functions.

The v0.1 core is written in C17 because it is portable across Linux, FreeBSD, OpenBSD, Raspberry Pi, and embedded targets. C also keeps ABI boundaries simple for later host backends, test tools, and MCU ports.

The audio path is designed to avoid heap allocation. Process functions validate their arguments, use caller-provided buffers, and update only their own state. This keeps future real-time callbacks deterministic and easier to audit.

## Planned Host Backends

- PortAudio for Linux and FreeBSD live USB sound-card processing.
- sndio for OpenBSD live audio after the portable core is stable.
- CMSIS-DSP for STM32H753 after the float32 host chain is proven.

## State Ownership

There is no global mutable DSP state. A host program creates processor structs, initializes them, and controls buffer lifetime. This keeps offline tools, live audio backends, and embedded ports using the same core model.
