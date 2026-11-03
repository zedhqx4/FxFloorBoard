# GT-10 MIDI Modernization Work Log

Working branch: `gt-10-midi-modernization` (based on `gt-10`).

This log records the transport/build work on the GT-10 branch. The reference branch `gt-10` is not modified.

## Changes

### Build and Qt compatibility
- `015629d` — Vendor the unmodified upstream RtMidi 6.1.0 source, header, and license under `third_party/rtmidi-6.1.0/`.
- `e36e4e7` — Add a Linux Qt 5 / ALSA GitHub Actions build.
- `ea775f9`, `563f6b7`, `8eafd8f`, `25f7ea7`, `982cbe4`, `7f461a0`, `48cc99c` — Restore Qt 5 widget, PrintSupport, and painter includes while retaining the Qt 4 conditional path.
- `cf6b2a5`, `57220c0` — Replace removed Qt 4 `QChar::toAscii()` calls with `toLatin1()` for patch-name conversion.
- `eb2c202`, `16b63e1`, `d0e621e` — Add explicit Qt 5 widget module includes to the remaining rename and summary dialog headers.
- `b3ab5da` — Include `QtGlobal` before `QT_VERSION_CHECK` in the three summary headers so MOC can evaluate the Qt 4/5 guards.

### MIDI transport and SysEx handling
- `eb94b49` — Switch Linux qmake builds from the old `linux/RtMidi.cpp` implementation to vendored RtMidi 6.1.0 with the explicit ALSA sequencer API. Keep the existing platform backends on Windows/macOS.
- `eb94b49` — Pass the owning `midiIO` instance as RtMidi callback user data instead of allocating a new `midiIO`/QThread for every incoming message.
- `eb94b49` — Make the SysEx receive buffer instance-owned and protect callback append, receive reads, and retry clearing with `QMutex`.
- `eb94b49` — Explicitly enable SysEx input, install the callback before opening the input port, and cancel/close only resources that were actually installed/opened.
- `3e5c4f3` — Validate outgoing hex bytes and SysEx `F0...F7` framing; remove leaked temporary `new[]` byte buffers and guard output-port construction/open/close errors.
- `7871852` — Clear MIDI input/output device lists before each enumeration to prevent duplicate entries after refresh.
- `63936f8` — Match GT-10 ports using both the legacy flat name and RtMidi ALSA's `client:port clientId:portId` name format.
- `5fa57bc` — Schedule each `midiIO` worker for deletion only after the real `QThread::finished()` signal; also release workers that cannot start because no output port is configured.
- `dbda7c0` — Make `msgType` and `loopCount` instance-owned, so concurrent workers cannot overwrite each other's expected response type or timeout.
- `aed89bf` — Set the Linux ALSA input decoder buffer to 4096 bytes before opening the port, sized for GT-10 patch/system replies.
- `8265790` — After the allowed retries, discard incomplete replies instead of forwarding partial data to the SysEx parser.
- `fba964c` — Validate patch-name replies as 29-byte GT-10 Roland DT1 frames with the expected model header, `F7` terminator, and checksum.
- `7868687` — Fix the receive handoff: `receiveMsg()` stores the completed reply in the worker's member buffer, and `run()` emits that same completed buffer instead of an empty local copy.
- `cf4ed1c` — Remove a duplicate final progress signal from the SysEx run path.
- `d3b5217` — Keep patch and System writes marked unverified until a subsequent readback confirms the device data.

### SysxIO write lifecycle
- `636d8b2` — Keep the device reserved until asynchronous patch/System writes complete; remove the fake blocking progress sleeps from `writeToBuffer()`. Device readiness is released by `resetDevice()` after the worker reply.
- `98fe613` — Reset the System-read error flag at the start of a fresh request, so a prior failed transfer does not poison later reads.
- `de1de54` — Keep permanent patch writes explicitly unverified after the send; the UI no longer claims synchronization merely because the write thread finished.
- `f27f4f4` — Verify writes to the temporary patch buffer by reading address `0x60:00` back with `patchRequest(0, 0)`, checking DT1 checksums and comparing payload bytes before setting sync true.
- `bb09a78` — Verify System writes by requesting the full System area after transmission and comparing the reconstructed DT1 payload with the expected System data before setting sync true.
- `99ccf58` — Verify permanent user-patch writes by requesting the exact bank/patch again, comparing the compact bulk-reply payload, and emitting a completion signal only after the comparison.
- `f7e9bcc` — Keep temporary-buffer UI state pending until the explicit readback result arrives, rather than reporting completion when the send starts.
- `e711bc6` — Require patch-name and patch-readback SysEx messages to match the requested Roland memory address.
- `0919d12` — Remove the obsolete display-level send-completion reset slot; the UI now responds to readback result signals.

## Validation status

- **Linux Qt 5 / ALSA CI passed** for commit `b3ab5da`: qmake configuration, full build, and the executable verification step all succeeded. Run: https://github.com/zedhqx4/FxFloorBoard/actions/runs/41669746927
- **Linux Qt 5 / ALSA CI also passed** for commit `f27f4f4`, including the full build and executable check. Run: https://github.com/zedhqx4/FxFloorBoard/actions/runs/41670981644
- Linux Qt 5 / ALSA CI passed for commit `99ccf58`, which adds permanent patch-memory readback verification. Run: https://github.com/zedhqx4/FxFloorBoard/actions/runs/41672969722
- This validates the Linux build and link, not physical MIDI communication.
- No physical GT-10 read/write test has been performed by CI. Device-level validation remains required, starting with identity, patch-name, and read-only patch/System requests.
- Temporary patch-buffer writes, System writes, and permanent user-patch writes now use automatic readback comparison in code. The UI does not mark these writes synchronized merely because bytes were sent. CI validates compilation only; actual device-level readback still needs testing on a physical GT-10.

