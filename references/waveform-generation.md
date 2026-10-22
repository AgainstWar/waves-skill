# Generate a trace from an existing failing simulation

Read this only when no usable trace exists and the project's failing
simulation can already be run.

1. Start from the failing command. Check its adjacent build configuration or
   testbench for an existing trace flag, variable, or dump call. A simulator
   may require both build-time trace support and a runtime enable flag.
2. Enable the smallest existing trace option and run the focused failing case.
   A small correction to an obvious flag or output path is reasonable; stop
   if the next step requires extensive environment setup or repeated rebuilds.
3. Check that a new waveform file exists, is non-empty, and is readable with
   `wave_get_info` before querying signals.

Do not replace the simulator, create a new harness, run a full regression, or
change DUT behavior, stimulus, or expected outputs for trace generation. If
the entry point or trace option is unclear, or the toolchain is unavailable,
record the blocker and continue from logs and code.
