# cyclic-buffer-driver

A minimal Linux character device driver that implements a fixed-size
circular (ring) buffer in kernel space, exposed to userspace as a regular
device file: `/dev/cyclic_buffer`.

## Project layout

```
cyclic-buffer-driver/
├── Makefile              — root Makefile, drives the two subprojects
├── kernel/
│   ├── cyclic_buffer.c   — the driver
│   └── Makefile
├── userspace/
│   ├── tool.c            — CLI for exercising the driver
│   └── Makefile
├── test.sh               — end-to-end smoke test
└── README.md
```

## Features

- `write()` — pushes bytes into the ring buffer
- `read()` — pops bytes out of the ring buffer (FIFO order)
- `ioctl(CYCLIC_IOC_CLEAR)` — reset the buffer (zero it, reset pointers)
- `ioctl(CYCLIC_IOC_AVAILABLE)` — get the number of bytes currently stored
- **Configurable buffer size** via the `buffer_size` module parameter
  (default 4096 bytes, read-only after load)
- **Live statistics in sysfs**, under
  `/sys/class/cyclic_buffer/cyclic_buffer/`:
  - `bytes_written` — cumulative bytes written since the module was loaded
  - `bytes_read` — cumulative bytes read since the module was loaded
  - `fill_percent` — current buffer fill level, 0–100
- `/dev/cyclic_buffer` is created automatically (no `mknod` needed),
  with permissions `0666`
- All buffer state is protected by a mutex
  (`mutex_lock_interruptible`, so blocked processes stay killable)

## Building

From the project root:

```bash
make all
```

This builds the kernel module (`kernel/cyclic_buffer.ko`) and the
userspace tool (`userspace/tool`).

## Loading the module

```bash
make load                              # default buffer size (4096 bytes)
```

To load with a custom buffer size, go into `kernel/` and call `insmod`
directly with a parameter:

```bash
cd kernel
sudo insmod cyclic_buffer.ko buffer_size=65536
```

Check what size the module actually loaded with:

```bash
cat /sys/module/cyclic_buffer/parameters/buffer_size
```

Unload:

```bash
make unload
```

## Using the CLI tool

```bash
cd userspace
./tool clear              # reset the buffer
./tool write "hello"      # write a string
./tool avail               # how many bytes are available to read
./tool read 5               # read 5 bytes
```

## Reading live stats from sysfs

```bash
cat /sys/class/cyclic_buffer/cyclic_buffer/bytes_written
cat /sys/class/cyclic_buffer/cyclic_buffer/bytes_read
cat /sys/class/cyclic_buffer/cyclic_buffer/fill_percent
```

## Running the test suite

```bash
chmod +x test.sh    # once
./test.sh
```

`test.sh` builds both subprojects, loads the module, drives the CLI tool
through `clear` / `write` / `avail` / `read`, and asserts the exact
values reported by `bytes_written`, `bytes_read`, and `fill_percent`
against what the operations should have produced. The module is always
unloaded on exit, even if a step fails, via a `trap` on `EXIT`.

## Known limitations / possible next steps

- `read()` on an empty buffer does not block — it simply returns 0 bytes.
  A proper blocking `read()` (and `poll()`/`select()` support) would
  require a `wait_queue_head_t`.
- `buffer_size` is fixed at load time; changing it requires `rmmod` +
  `insmod` with a new value (the module parameter is `0444`, read-only
  in sysfs, intentionally — the buffer is never resized at runtime).
- No limit on the number of concurrent opens of the device.

## License

GPL-2.0 — see the `SPDX-License-Identifier` header in `cyclic_buffer.c`.