# Waveform Generation Constraints

Use this reference when the user does not yet have a waveform file and the
project uses a build system or test framework that needs exploration.

## What This Reference Provides

- **Safety boundaries** — what automated steps are safe vs. out of scope.
- **Decision rules** — how to classify the result as `success`, `skipped`, or
  `failed`.
- **Common pitfalls** — things that look like they should work but don't.

This reference does **not** contain step-by-step commands for specific
simulators or projects. The agent using this skill should inspect the
project's build configuration files on the fly and apply these constraints.

## Core Principle

> Only modify files that lie **on the call chain of the failing command**.
> Do not guess unrelated files.

## Detection Strategy

1. Start from the **failing command** the user provides.
2. Read the build script or configuration adjacent to that command.
3. Search for keywords related to waveform/trace output:
   - Flags: `--trace`, `--trace-fst`, `--dump`, `-v`, `+vcdplusfile`
   - Variables: `TRACE`, `WAVES`, `debug`, `EMU_TRACE`
   - File formats: `.vcd`, `.fst`, `.fsdb`, `.vpd`
4. Infer which tool is in use from the build structure (not from the directory
   name alone).

## Safe Modifications

These are safe to do automatically:

- Adding an existing trace flag that the build system already supports.
- Enabling an existing build variable that controls waveform output.
- Adding waveform dumping calls to the clearly identified testbench (only
  for plain HDL with no build system).
- Creating a local output directory for the waveform file.
- Running the minimum failing case and checking for output.

## Out of Scope

Do **not** attempt these in fully automated runs:

- Replacing or upgrading the simulator.
- Using a commercial simulator that requires an unavailable license.
- Rewriting the build system.
- Creating a new harness to replace the existing test.
- Modifying DUT / RTL behavior, stimulus, or expected outputs.
- Guessing SoC / firmware / co-simulation environments.
- Running a full regression just to generate one waveform.

## Common Pitfalls

### Build-Time vs Runtime Trace Enablement

Some simulators require trace support to be compiled in (build-time flags) AND
activated at runtime (execution flags). If you only enable one, no waveform is
produced. Check both:
1. Build configuration for trace-related compilation flags.
2. Runtime invocation for trace activation flags or arguments.

### Harness Initialization

In compiled simulation flows, the top-level harness must open a trace file at
runtime. This may be done in the main entry point or delegated to a utility
module. Inspect the harness source to confirm trace file opening exists in the
call chain.

### Output Format

The build configuration usually determines the default waveform format. Use
whatever the build system produces by default; do not attempt to change formats
unless the project clearly documents how to do so.

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

### `success`

- Waveform file exists and is non-empty.
- `waveform_path` points to the generated file.
- `validation.exists` = true, `validation.non_empty` = true.
- `next_action` = `continue_with_waveform`.

### `skipped`

Use when no safe automatic path exists and you have not attempted any
modification:

- Failing command not found.
- Simulation entry ambiguous.
- Only commercial simulator path found (no license).
- Waveform generation requires broad refactor.
- Harness too complex for safe automatic dump.

`next_action` = `continue_without_waveform`.

### `failed`

Use when a minimal safe modification was attempted but did not produce a valid
waveform file:

- Trace flag added but build failed.
- Simulation ran but no waveform file produced.
- Waveform file produced but empty.
- Output path not writable.
- Command timed out.

`next_action` = `continue_without_waveform`.

## Upper-Layer Handling

After receiving the result:

```
if waveform_dump_result.status == success:
    try:
        use MCP to read waveform_dump_result.waveform_path
        proceed with waveform querying
    except:
        report skipped(reason="MCP failed to read or convert waveform")
        continue with normal log / code debugging
else:
    record waveform_dump_result.reason
    continue with normal log / code debugging
```

## Quick Checklist

1. Did the modified command run without fatal errors?
2. Does the expected waveform file exist?
3. Is the waveform file non-empty (size > 0)?
4. Can MCP read it?

If any answer is no, move to `failed` or `skipped`.
