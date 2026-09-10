// Thin LittleFS wrapper: fixed-size binary blobs by path. Used for the touch
// calibration now, and for save games / high scores in later phases.
// NVS is only ~20 KB on this board (target ref §3), so everything lives here.
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace storage {

// Mounts LittleFS, formatting on first use. Safe to call more than once.
bool begin();

// Reads exactly `len` bytes of `path` into `out`. Returns false if the file is
// missing, the wrong size, or FS is unavailable — `out` is left untouched then.
bool loadBlob(const char* path, void* out, size_t len);

// Writes `len` bytes to `path`, replacing it. Returns false on any FS error.
bool saveBlob(const char* path, const void* data, size_t len);

// Removes `path` if present.
void remove(const char* path);

}  // namespace storage
