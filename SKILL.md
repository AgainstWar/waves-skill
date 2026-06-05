---
name: waves-debug
description: >
  Use this skill for debugging digital designs with the WAVES MCP server.
  Trigger whenever the user reports HDL or digital design bugs, simulation failures,
  unexpected output values, or signals behaving incorrectly — including when
  they describe symptoms like "stuck", "never goes high", "wrong output",
  "testbench fails", "state machine not transitioning", or "simulation timeout".
  Also trigger for explicit VCD/FST/FSDB analysis, waveform generation, or
  trace enable requests. This skill provides waveform generation and MCP
  querying tools; the agent decides when signal-level data is needed. If the
  bug is obvious from code or logs, waveforms may not be necessary. Operate
  autonomously without asking the user for confirmation.
---

# WAVES Debug Skill

A workflow guide for using the WAVES MCP server to query and debug
waveform files generated from digital design simulations.

## What This Skill Covers

This skill guides you through the full waveform debugging workflow:

1. **Prerequisites** — Ensure WAVES MCP is installed and configured.
2. **Generate Waveforms** — Detect the user's simulation environment and
   generate a waveform file with minimal, safe modifications.
3. **Explore Signals** — Browse the waveform to find exact hierarchical signal
   names and understand the design structure.
4. **Query Values & Transitions** — Read signal values at specific times,
   capture transition histories, and correlate multi-signal behavior.
5. **Debug Patterns** — Apply systematic debugging strategies to narrow from
   symptoms to root cause.

## When to Use This Skill

Use this skill when the user is:
- Debugging a digital design after simulation.
- Inspecting a waveform file to understand why a module behaves incorrectly.
- Looking for specific signal transitions (e.g., "when does `ready` go high?").
- Comparing multiple signals in the same time window.
- Troubleshooting clock, reset, or state machine issues.
- **Generating a waveform file** from an existing simulation setup (no matter
  which simulator or build system they use).
- **Enabling wave dumps** in any simulation flow.

## When NOT to Use Waveforms

This skill is a supplement, not a replacement for normal debugging. **Skip
waveform analysis when:**

- The bug is obvious from code review (e.g., typo in assignment, wrong
  constant, missing case branch).
- The error message or log output already pinpoints the exact line and cause.
- The issue is purely compilation/syntax (e.g., missing delimiter, undeclared identifier).
- The user only needs help with build commands or environment setup.

In these cases, fix the obvious issue first. Only generate or query waveforms
when you need to verify signal-level behavior that cannot be determined from
static code analysis or log output alone.

## Prerequisites

Before querying waveforms, **attempt** to use MCP tools directly. Do not ask
the user whether WAVES is installed — the MCP call itself is the availability
check.

- If MCP tools respond normally, proceed.
- If MCP returns "tool not found" or connection error, record:
  `prerequisite_status: "WAVES MCP unavailable — continuing with log/code analysis"`
  and proceed with non-waveform debugging.

## Discretion Principle

The agent decides when waveform data is needed. This skill provides the tools;
it does not mandate their use. If you can resolve the issue through code
review, log analysis, or static inspection alone, do so without waveforms.

Only use waveform tools when:
- Signal timing or transition behavior is critical to understanding the bug.
- Static analysis or logs leave ambiguity about what actually happened.
- The user explicitly provided or requested a waveform file.

This skill runs in **single-turn, zero-interaction mode**. At no point should
you ask the user for confirmation, permission, or missing information. If the
correct action is unclear, make the safest reasonable attempt. If that fails,
report `skipped` with a specific reason and fall back to non-waveform debugging.

## Executing the Waveform Generation Workflow

When waveform data is needed, **execute the workflow autonomously** rather than
merely giving advice. Use your judgment at each step: the goal is to produce a
readable waveform file with the least invasive change possible.

**General strategy:**
- If the project has a clear build system or testbench entry point, follow it.
- If the structure is ambiguous, explore the project files to find the simulation
  entry point, then proceed.
- If enabling waveforms would require rewriting the build system, modifying
  design source files, or making broad structural changes, report `skipped`
  and fall back to non-waveform debugging.

### Locate the Simulation Entry Point

Find how the project runs its simulation:
- Look for build configuration files in the project root and adjacent to any
  command the user mentioned.
- If the user mentioned a specific module or testbench name, search for it in
  the project files.
- If multiple candidates exist, use your judgment to pick the most likely one
  based on naming conventions, file contents, or README instructions. If you
  truly cannot determine the correct entry point, report `skipped`.

### Enable Waveform Dumping

Once you have identified the simulation entry point, figure out how to turn on
waveform output **without rewriting the project**:
- Inspect the build configuration for trace-related keywords: `--trace`,
  `--dump`, `TRACE`, `WAVES`, `debug`, or file extensions like `.vcd`, `.fst`,
  `.fsdb`.
- Check the testbench or test harness source for any existing waveform dumping
  calls that may be gated by a parameter, commented out, or disabled by default.
- If the exact flag or procedure is unclear, read the project's README or any
  documentation comments in the build scripts.
- Make the **minimal safe change** — enable an existing flag or variable, or
  activate whatever waveform dumping mechanism the project already provides.
  Do not rewrite the build system or edit design source files.
- If no obvious mechanism exists and adding one would require significant
  refactoring, report `skipped` with the reason.

### Run the Simulation

After enabling the trace option, run the simulation command:
- If the build system needs to regenerate the simulator binary, run the build
  step first.
- Then run the simulation with the trace option enabled.
- If the command fails, retry up to 3 times with adjustments (e.g., different
  flags, correcting minor parameter issues). If it still fails after 3 attempts,
  report `skipped`.
- Check the output directory for a newly generated `.vcd`, `.fst`, `.fsdb`, or
  `.lxt` file.
- If no file appears, check if the simulator binary accepts runtime arguments
  for trace enablement and try the most likely one based on the backend
  (consult the project's docs or README; do not guess blindly).
- If still no trace file is produced after reasonable attempts, report `skipped`
  with a specific reason.

### Verify and Query with MCP

Once a waveform file is produced:
1. Call `wave_get_info` on it to confirm MCP can read it.
   - If successful, note the `timescale`, `start_time`, `end_time`, and
     `signal_count`.
   - If MCP returns a parse or conversion error, report `skipped`.
2. If the user mentioned specific signals to investigate, call
   `wave_list_signals` with an appropriate filter to find the exact hierarchical
   names.
3. Query the signals using `wave_get_window`, `wave_get_transitions`, or
   `wave_get_value` to gather the data needed to debug the issue.

### Report the Result

If waveform generation and analysis succeed, report what you enabled, the command
you ran, MCP confirmation, the signals you found, and your diagnosis.

If any step fails and you report `skipped`, state that waveform analysis could
not be completed and briefly explain why (e.g., "no build system found",
"trace enable requires significant refactoring", or "simulation command failed
after 3 retries"), then fall back to non-waveform debugging.

Read `references/waveform-generation.md` for safety boundaries, common pitfalls,
and additional result-format details. The reference does not contain
project-specific commands — you must discover them by inspecting the project's
files on the fly.

## MCP Tool Reference

WAVES exposes 6 tools:

### `wave_get_info`

**Purpose:** Understand the scope of the waveform file.

Call this first on any new waveform file to learn:
- `timescale` — the simulation time unit.
- `start_time` / `end_time` — the total time range.
- `signal_count` — how many signals are recorded.

This helps you set realistic `time` and `start_time`/`end_time` values in
subsequent queries.

**Example:**
```json
{"vcd_path": "tests/fixtures/sample.vcd"}
```

### `wave_list_signals`

**Purpose:** Find the exact hierarchical signal names.

WAVES requires **exact** hierarchical names (e.g., `tb.dut.clk`). Use
`wave_list_signals` with a `filter` substring to narrow down candidates.

**Parameters:**
- `vcd_path` — path to the waveform file.
- `filter` (optional) — substring to match in signal names.
- `limit` (optional) — maximum signals to return.

**Example:**
```json
{"vcd_path": "tests/fixtures/sample.vcd", "filter": "clk"}
```

**Strategy:** If the user mentions a signal like "the clock", filter for
`clk` to get candidates. Try the shortest hierarchical path first with a small
`wave_get_transitions` query. If it succeeds, use that name. If it fails with
"signal not found", try the next shortest match. If all candidates fail, report
`skipped(signal_ambiguous)` and list the candidates found.

### `wave_get_value`

**Purpose:** Read a signal's value at a specific time.

This tool uses **at-or-before** semantics: if no transition exists exactly at
`time`, it returns the most recent value recorded at or before that timestamp.
If the signal has no recorded value before `time`, it returns `null`.

**Parameters:**
- `vcd_path` — path to the waveform file.
- `signal` — exact hierarchical name (from `wave_list_signals`).
- `time` — raw VCD integer timestamp.
- `value_format` (optional) — display format: `raw`, `bin`, `hex`, `uint`, `sint`, or `ascii`.
  Default is `raw`. If not `raw`, the response includes `value_raw` with the
  original value.

**Example:**
```json
{"vcd_path": "tests/fixtures/sample.vcd", "signal": "tb_pmic_fsm.clk", "time": 100000}
```

**Output:**
```json
{"signal": "tb_pmic_fsm.clk", "time": 100000, "value": "0"}
```

### `wave_get_transitions`

**Purpose:** List all transitions of a signal within a time range.

**Parameters:**
- `vcd_path` — path to the waveform file.
- `signal` — exact hierarchical name.
- `start_time` / `end_time` — inclusive range.
- `edge` (optional) — filter by `posedge`, `negedge`, or `any`.
- `value` (optional) — filter transitions to only those with this exact resulting value.
- `limit` (optional) — max transitions to return. Default 50.
- `value_format` (optional) — display format: `raw`, `bin`, `hex`, `uint`, `sint`, or `ascii`.

**Example:**
```json
{"vcd_path": "tests/fixtures/sample.vcd", "signal": "tb_pmic_fsm.clk",
 "start_time": 0, "end_time": 200000, "edge": "posedge"}
```

**Output:**
```json
{
  "signal": "tb_pmic_fsm.clk",
  "start_time": 0,
  "end_time": 200000,
  "transitions": [{"time": 10000, "value": "1"}],
  "truncated": false,
  "value_format": "raw"
}
```

**Note:** Empty `transitions` is normal data, not an error. It simply means
no transitions matching the filter occurred in that window.

### `wave_get_window`

**Purpose:** Query multiple signals in the same time window.

This is powerful for correlating behavior across signals (e.g., checking that
`valid` goes high on the same clock cycle as `data` changes).

**Parameters:**
- `vcd_path` — path to the waveform file.
- `signals` — array of exact hierarchical names. Max 20 signals, no duplicates.
- `start_time` / `end_time` — explicit time window (inclusive).
- `center_time` + `before` / `after` — alternative centered window.
  These two window styles are **mutually exclusive**.
- `format` (optional) — `"structured"` (per-signal lists) or `"table"`
  (time-aligned text rows). Default `"structured"`.
- `limit_per_signal` (optional) — max transitions per signal. Default 50.
- `value_format` (optional) — display format: `raw`, `bin`, `hex`, `uint`, `sint`, or `ascii`.

**Example (structured):**
```json
{"vcd_path": "tests/fixtures/sample.vcd",
 "signals": ["tb_pmic_fsm.clk", "tb_pmic_fsm.rst_n"],
 "start_time": 0, "end_time": 50000}
```

**Example (table):**
```json
{"vcd_path": "tests/fixtures/sample.vcd",
 "signals": ["tb_pmic_fsm.clk", "tb_pmic_fsm.rst_n"],
 "start_time": 0, "end_time": 50000, "format": "table"}
```

**Strategy:** Prefer `"table"` format when the user wants to see how signals
relate to each other at the same timestamps. Prefer `"structured"` when the
user wants to analyze each signal independently.

### `wave_find_transition`

**Purpose:** Find the nearest transition before or after a given time.

Useful for "when did X happen?" questions.

**Parameters:**
- `vcd_path` — path to the waveform file.
- `signal` — exact hierarchical name.
- `time` — reference timestamp.
- `direction` — `"next"` or `"prev"`.
- `edge` (optional) — `"posedge"`, `"negedge"`, or `"any"`.

**Example:**
```json
{"vcd_path": "tests/fixtures/sample.vcd", "signal": "tb_pmic_fsm.rst_n",
 "time": 5000, "direction": "next", "edge": "posedge"}
```

## Data-Gathering Strategies

This skill provides the tools. The agent's own reasoning should draw conclusions
from the data. Use these strategies to collect relevant waveform data:

- **Correlate multiple signals** — `wave_get_window` with `"table"` format to
  see how signals relate at the same timestamps.
- **Find exact signal names** — `wave_list_signals` with substring filters.
- **Track transitions** — `wave_get_transitions` across the full range or a
  suspect window.
- **Locate specific events** — `wave_find_transition` to jump to the next or
  previous change.
- **Read values at exact times** — `wave_get_value` for at-or-before semantics.

## Error Handling

WAVES raises domain errors as `WavesQueryError`. The MCP server converts these
to `ToolError`. Common categories:

| Category | Cause | What to do |
|----------|-------|------------|
| Waveform file error | File missing, unreadable, or invalid format | Check the path. If the file is missing, attempt waveform generation if the context allows. Otherwise, report `skipped(waveform_missing)` and continue with log/code analysis. |
| Signal error | Exact signal name not found | Immediately follow up with `wave_list_signals` and try the closest match autonomously. Do not ask the user to pick. |
| Parameter error | Negative time, `limit <= 0`, invalid enum | Fix the parameters (e.g., clamp time to 0, set limit to 50) and retry automatically. |

When you hit a signal error, **always** follow up with `wave_list_signals` and
present the closest matches.

## Example

**User:** "My `ready` signal never goes high."

**Agent:** `wave_get_info` → `wave_list_signals(filter="ready")` → `wave_get_transitions` → (no transitions) `wave_get_window` on clock, reset, and `ready` → (reset held low) concludes the module is stuck in reset.

## Summary

This skill provides waveform generation and MCP querying capabilities. Its job is
to make waveform data available to the agent, not to prescribe diagnoses. The
agent uses its own reasoning to interpret the data and identify root causes.

Workflow:
1. Start with `wave_get_info` to understand the file scope.
2. Use `wave_list_signals` to discover exact signal names.
3. Combine `wave_get_value`, `wave_get_transitions`, `wave_get_window`, and
   `wave_find_transition` to gather signal data.
4. Let the agent's own logic analyze the gathered data and draw conclusions.
5. Handle tool errors by retrying or skipping — never ask the user for help.
