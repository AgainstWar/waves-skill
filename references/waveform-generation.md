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

## Result Format

Always report the waveform generation attempt with this structure:

```yaml
waveform_dump_result:
  status: success | skipped | failed
  reason: <specific reason string>
  failing_command: <the command that was failing or null>
  toolchain: <detected tool name or "unknown">
  insertion_point: <file modified or null>
  waveform_command: <command used to generate or null>
  waveform_path: <absolute path or null>
  validation:
    exists: true | false
    non_empty: true | false
  next_action: continue_with_waveform | continue_without_waveform
```

- `success`: a non-empty waveform exists; set `waveform_path`, both validation
  fields to `true`, and `next_action` to `continue_with_waveform`.
- `skipped`: no modification or simulation attempt was made because no safe
  path exists; set `next_action` to `continue_without_waveform`.
- `failed`: a focused attempt was made but did not yield a usable waveform;
  set `next_action` to `continue_without_waveform`.

After `success`, query the file with WAVES. If MCP cannot read it, explain
that failure and continue with logs and code.
