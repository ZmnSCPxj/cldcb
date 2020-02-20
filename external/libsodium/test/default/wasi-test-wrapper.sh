#! /bin/sh

MAX_MEMORY_TESTS="67108864"

if [ -z "$WASI_RUNTIME" ] || [ "$WASI_RUNTIME" = "wasmtime" ]; then
  if command -v wasmtime >/dev/null; then
    wasmtime -o --dir=. "$1" && exit 0
  fi
fi

if [ -z "$WASI_RUNTIME" ] || [ "$WASI_RUNTIME" = "wasmer" ]; then
  if command -v wasmer >/dev/null; then
    wasmer run "$1" --backend "${WASMER_BACKEND:-cranelift}" --dir=. && exit 0
  fi
fi

if [ -z "$WASI_RUNTIME" ] || [ "$WASI_RUNTIME" = "wasmer-js" ]; then
  if command -v wasmer-js >/dev/null; then
    wasmer-js run "$1" --dir=. && exit 0
  fi
fi

if [ -z "$WASI_RUNTIME" ] || [ "$WASI_RUNTIME" = "wasm3" ]; then
  if command -v wasm3 >/dev/null; then
    wasm3 "$1" && exit 0
  fi
fi

echo "WebAssembly runtime failed" >&2
exit 1
