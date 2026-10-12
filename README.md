# optee_selftest — OP-TEE isolation self-test (BPI-F3 / K1)

A small, debug-only harness that proves two OP-TEE isolation properties on the
SpacemiT K1, modelled on the Andes "Hands-on OP-TEE" workshop labs but written
from scratch (no workshop code or binaries are reused).

## What it proves

- **L1 — secure storage.** The TA (`ta/`) seals a payload into an OP-TEE
  private storage object and reads it back. The plaintext only ever exists
  inside the TA; the blob OP-TEE writes to the REE filesystem is ciphertext,
  sealed with the per-device key. The CA (`host/optee_selftest`) seals a known
  marker, unseals it and compares.
- **L2 — CPU isolation.** The debug kernel module (`kmod/`) attempts, from
  S-mode (Linux), to read the OP-TEE secure RAM (the DT `optee@36000000`
  reserved region). OpenSBI programs that range out of the untrusted domain's
  PMP, so the read faults. The module catches the fault and reports `PASS`
  (isolated) or `LEAK` (not isolated).

## What it does NOT cover

- **L3 — DMA isolation.** The K1 has no IOPMP, so a DMA engine can still reach
  secure RAM. This is a hardware limit of the K1, documented, not tested here.
  The K3 has an IOPMP and is the target for a real L3 test.

## Build and run

TA + CA (built by the tree):

    SPACEMIT_OPTEE_SELFTEST=1 ./device/spacemit/common/build/build.sh k1
    # ship a userdebug image, then on the board:
    optee_selftest            # prints [L1] PASS

Debug kernel module (out-of-tree, never in the image):

    make -C kmod KDIR=/path/to/kernel/spacemit/common ARCH=riscv \
         CROSS_COMPILE=riscv64-linux-gnu-
    # push optee_isolation_test.ko to the board, then:
    insmod optee_isolation_test.ko     # self-unloads; read the verdict:
    dmesg | grep optee_isol            # PASS = TEE RAM isolated from S-mode

Override the probed address with `insmod optee_isolation_test.ko base=0x36000000`.

## Layout

- `ta/` — the trusted application (UUID `b8f1e2c4-9a3d-4b6e-a107-5e1f2c3d4e5f`).
- `host/` — the client application `optee_selftest`.
- `kmod/` — the debug S-mode isolation probe.
