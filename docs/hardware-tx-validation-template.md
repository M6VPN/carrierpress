# Hardware TX Validation Evidence Template

This template is for a future hardware TX backend milestone. It is not
compliance proof, RF proof, legal-bandwidth proof, licence-compliance proof,
certification evidence, or permission for on-air testing. T7A does not add a
hardware backend.

## Test Record

- Date:
- CarrierPress commit:
- CarrierPress version:
- Build flags:
- Future backend type:
- Connected radio or interface:
- Local operator:
- Test location:

## Pre-test Gate

- Confirm ordinary builds remain non-transmit:
- Confirm hardware backend was explicitly selected for this future test:
- Confirm runtime arming starts disabled:
- Confirm emergency RX/drop was tested before backend calls:
- Confirm no profile/config/report/batch/playlist/selector path can arm
  transmit:
- Confirm no frequency/mode automation is enabled:

## Receive-only Result

- Receive-only setup used:
- Radio/interface remained RX/off:
- Readback result:
- Emergency RX/drop result:
- Failure or recovery notes:

## Dummy-load Result

- Dummy load or equivalent used:
- Operator confirmation before test:
- Backend assertion result:
- Emergency RX/drop result:
- Shutdown RX/off result:
- Failure or recovery notes:

## Evidence Paths

- Command log:
- Build log:
- Safety audit output:
- Test output:
- Operator notes:

## Operator Confirmation

- I confirm this record is engineering validation evidence only:
- I confirm this record does not claim compliance or legal authorization:
- I confirm no on-air test is represented by this record unless separately
  documented outside CarrierPress:

## Follow-up

- Issues found:
- Required fixes:
- Re-test required:
