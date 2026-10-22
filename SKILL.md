---
name: waves-debug
description: >
  Use WAVES MCP to inspect VCD/FST traces when debugging HDL runtime
  behavior, timing, state transitions, or failing simulations. Also use when
  the user asks to generate or inspect a waveform. Skip waveform work for
  compilation-only errors unless the user explicitly requests it.
---

# WAVES Debug

Use waveform evidence when it can clarify a specific failure. Keep the path
from failing behavior to a useful signal query short; do not spend the repair
budget building a new simulation environment solely to obtain a trace.

## Choose a path

1. **A trace from the failing run already exists:** use it. Call `wave_get_info` to check that WAVES
   can read the file, then find relevant signals with `wave_list_signals` and
   make a targeted query. An available trace should not be ignored merely
   because the source code suggests a plausible fix.
2. **No trace, but the failing simulation is runnable:** inspect its existing
   command and trace option. Enable tracing with the smallest change and run
   the same focused case once. Read [waveform generation](references/waveform-generation.md)
   only for this path.
3. **No clear, low-cost path to a trace:** continue with the available logs and
   code. Do not install a toolchain, invent a harness, or enter a prolonged
   build/debug loop just to satisfy this skill.

An MCP call requires a waveform path. If WAVES is unavailable or cannot read
the trace, record that fact and continue with non-waveform evidence.

## Query the failure, not the whole trace

Start from the observed symptom and one candidate explanation. Find exact
hierarchical names with `wave_list_signals`, then use the smallest useful
query:

- `wave_get_value` for a value at a known time;
- `wave_find_transition` or `wave_get_transitions` for an expected event;
- `wave_get_window` for related signals around the failing interval.

Include clock/reset or handshake signals only when they help distinguish
competing explanations. Interpret the returned values against the expected
behavior; state whether they support, contradict, or leave the hypothesis
unresolved. If the trace lacks the needed signal or failing interval, do not
claim that it establishes the root cause.

After one informative query, return to the repair task. Make further queries
only when they can resolve a concrete remaining ambiguity. Do not call tools
merely to increase the MCP call count.

## Report

Briefly name the trace or reason it was unavailable, the decisive signal
observation if one exists, and what it changes about the repair hypothesis.
If waveform analysis was infeasible, proceed with logs and code rather than
ending the repair attempt.
