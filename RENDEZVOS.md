# RendezvOS initramfs (cpio) integration

User test ELFs are no longer embedded in the kernel `.user` section by default.
RendezvOS `script/config/user.json` sets `"filesystem": true`, which means:

1. `make user ARCH=x86_64` (from **RendezvOS_Linux** repo root) builds this tree.
2. `link_app.py` emits a **stub** `link_app.o` (`_num_app = 0`).
3. `pack_user_rootfs.py` copies `user/build/$ARCH/*` → `../rootfs/tests/` and writes
   `rootfs/tests/manifest` (paths like `/tests/test_echo`).
4. Kernel test runner loads each line from manifest via VFS and spawns the ELF.

## Layout

```text
rootfs/
  text.txt              # fixture data (open/read tests)
  mnt/
  tests/                # generated — static ELF64 binaries
    manifest            # one absolute path per line
    test_echo
    execve
    ...
```

## Build (from RendezvOS_Linux)

```bash
make user ARCH=x86_64      # or aarch64
make ARCH=x86_64 build run
```

Requires: cmake, cross gcc (`x86_64-linux-gnu-gcc` / `aarch64-linux-gnu-gcc`).

## Execve paths

Place targets under **`/tests/<name>`** in source:

```c
execve("/tests/test_echo", argv, envp);
```

The kernel also accepts bare names (`test_echo`) and resolves them to
`/tests/test_echo` for compatibility.

## Legacy embedded mode

Set `RENDEZVOS_FILESYSTEM_MODE=0` when running `link_app.py`, or use
`"filesystem": false` in RendezvOS `user.json` to restore `.incbin` link_app.o.

## Sync note

Changes under `user_payload/` should be pushed to **user_basic**; RendezvOS keeps
`script/config/user_payload_link_app.py` as a reference copy of `link_app.py`.
