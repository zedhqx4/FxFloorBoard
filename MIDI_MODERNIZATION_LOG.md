# GT-10 MIDI Modernization Work Log

Working branch: `gt-10-midi-modernization` (based on `gt-10`).

This log records the transport/build work on the GT-10 branch. The reference branch `gt-10` is not modified.

## Changes

### Build and Qt compatibility
- `015629d` — Vendor the unmodified upstream RtMidi 6.1.0 source, header, and license under `third_party/rtmidi-6.1.0/`.
- `e36e4e7` — Add a Linux Qt 5 / ALSA GitHub Actions build.
- `ea775f9`, `563f6b7`, `8eafd8f`, `25f7ea7`, `982cbe4`, `7f461a0`, `48cc99c` — Restore Qt 5 widget, PrintSupport, and painter includes while retaining the Qt 4 conditional path.
- `cf6b2a5`, `57220c0` — Replace removed Qt 4 `QChar::toAscii()` calls with `toLatin1()` for the patch-name conversion paths.

### MIDI transport
- `eb94b49` — Switch Linux qmake builds from the old `linux/RtMidi.cpp` implementation to vendored RtMidi 6.1.0 with the explicit ALSA sequencer API. Keep the existing platform backends on Windows/macOS.
- `eb94b49` — Pass the owning `midiIO` instance as RtMidi callback user data instead of allocating a new `midiIO`/QThread for every incoming message.
- `eb94b49` — Make the SysEx receive buffer instance-owned and protect callback append, receive reads, and retry clearing with `QMutex`.
- `eb94b49` — Explicitly enable SysEx input, install the callback before opening the input port, and cancel/close only resources that were actually installed/opened.
- `3e5c4f3` — Validate outgoing hex bytes and SysEx `F0...F7` framing; remove the leaked temporary `new[]` byte buffers and guard output-port construction/open/close errors.
- `7871852` — Clear MIDI input/output device lists before each enumeration to prevent duplicate entries after refresh.
- `63936f8` — Match GT-10 ports using both the legacy flat name and RtMidi ALSA's `client:port clientId:portId` name format.
- `5fa57bc` — Schedule each `midiIO` worker for deletion only after the real `QThread::finished()` signal; also release workers that cannot start because no output port is configured.
- `7432063` — Replace the two remaining `QString::toAscii()` uses in MIDI preference conversion with `toLatin1()`.
- `dbda7c0` — Make `msgType` and `loopCount` instance-owned, so concurrent `midiIO` workers cannot overwrite each other's expected response type or timeout.
- `aed89bf` — Set the Linux ALSA input decoder buffer to 4096 bytes before opening the port, sized for the GT-10 patch/system reply payloads.
- `8265790` — After the allowed retries, discard incomplete replies instead of forwarding partial data to the SysEx parser.
- `cbaf127` — Remove the fragile global QtWidgets preinclude and add the remaining explicit Qt 5 widget headers to `customLabelDisplay.h` and `customKnobRange.h`.
- `eb2c202` — Add the explicit Qt 5 widgets include to `customRenameDialog.h`, the final missing dialog header found after removing the global preinclude.
- `fba964c` — Validate patch-name replies as 29-byte GT-10 Roland DT1 frames with the expected model header, `F7` terminator and correct checksum before decoding the name.

## Validation status

- qmake configuration for the Linux Qt 5 CI job has succeeded in the observed runs.
- Earlier build runs reached the Qt compatibility errors listed above; those were corrected incrementally.
- qmake configuration succeeds for the current branch. The latest CI run for `fba964c` is compiling after the explicit Qt 5 dialog header and patch-name validation changes; its result must be checked before marking the Linux build as passing.
- No physical GT-10 read/write test has been performed by CI. Device-level validation remains required, starting with identity and read-only patch/name requests.
- No patch write test should be treated as validated until the written test patch is read back from the GT-10 and compared.
