---
name: upgrade-vss-extension
description: >
  Upgrade or maintain the statically-linked DuckDB VSS (vector similarity search / HNSW)
  extension in the spiceai/duckdb fork. Use when bumping the DuckDB version, bumping the
  vendored duckdb-vss commit, or debugging why HNSW indexes fail to build/load in the Spice
  runtime. Covers this C++ fork plus the companion changes in spiceai/duckdb-rs and spiceai/spiceai.
---

# Statically-linked VSS (HNSW) extension

## Why this exists

The Spice runtime offers a DuckDB vector engine (`vectors.engine: duckdb`) that builds HNSW
indexes via DuckDB's VSS extension. VSS is **out-of-tree** (`github.com/duckdb/duckdb-vss`,
depends on the header-only `usearch`/`fp16`/`simsimd` libraries), so a stock bundled DuckDB
does not include it. Relying on a runtime `INSTALL vss` is fragile: it needs network at
runtime, fails on air-gapped hosts, and 404s for custom DuckDB builds whose version hash has
no published extensions. To make HNSW work out of the box, VSS is **statically compiled into
the bundled DuckDB** and auto-loaded at database open.

## Layout (this repo: spiceai/duckdb)

```
extension/vss/
├── SKILL.md          # this file
├── vss_config.py     # package_build.py per-extension contract (source_files + include_directories)
└── src/              # VENDORED copy of duckdb-vss src/ at the ABI-matched commit (committed files, NOT a submodule)
    ├── vss_extension.cpp
    ├── hnsw/*.cpp
    └── include/{vss_extension.hpp, hnsw, usearch, fp16, simsimd, aggregate_function_matcher.hpp}
```

`vss_config.py` is the same mechanism in-tree extensions (parquet/json/icu) use: it lists
`source_files` (all `src/**/*.cpp`) and `include_directories` (`src/include`, which recursively
also pulls the header-only usearch/fp16/simsimd headers).

**Why vendored, not a git submodule:** duckdb-vss has its own nested `duckdb` and
`extension-ci-tools` submodules. Cargo recursively checks out *all* submodules of the duckdb-rs
git dependency, so a vss *submodule* drags in a full nested `duckdb` checkout — whose deeply-nested
Swift example paths exceed Windows' `MAX_PATH` (260 chars) and break the Windows build
(`path too long ... class=Filesystem`). Committing the source directly (no submodule) has nothing
for Cargo to recurse into. usearch/fp16/simsimd ship inside duckdb-vss as plain headers, so the
vendored copy is fully self-contained.

## The three-repo picture

A change here is consumed by two downstream repos:

1. **spiceai/duckdb** (this repo) — vendors the VSS source as `extension/vss`.
2. **spiceai/duckdb-rs** — `crates/libduckdb-sys/`:
   - `duckdb-sources` is a submodule pinned to a commit of THIS repo.
   - `update_sources.py` lists `"vss"` in `EXTENSIONS`; it runs `package_build.py` over the
     sources and regenerates the committed `duckdb.tar.gz` + `manifest.json` (the actual cc
     build input).
   - `build_bundled_cc.rs` marks vss enabled in `extension_enabled()` and defines
     `DUCKDB_USEARCH_USE_SIMSIMD=0`. DuckDB 1.5's `package_build.py` emits a loader guarded by
     `#if DUCKDB_EXTENSION_VSS_LINKED`; `build_bundled_cc.rs` sets that define, so vss is
     registered in `LinkedExtensions()` and auto-loaded at DB open. No loader rewrite.
3. **spiceai/spiceai** — consumes duckdb-rs and must NOT `INSTALL vss` at runtime (vss is
   statically linked). `crates/search/src/index/duckdb/mod.rs` keeps only the pool's
   `LOAD vss` connection-setup query (a no-op success when statically linked); the old
   `install_vss_once()` was removed.

## Upgrade procedure (when bumping the DuckDB version, e.g. 1.5.x -> 1.6.x)

1. **Find the ABI-matched duckdb-vss commit.** In the new DuckDB source tree, read
   `.github/config/extensions/vss.cmake` — its `GIT_TAG <sha>` is the duckdb-vss commit DuckDB's
   own CI links against for that version. Use exactly that commit.

2. **Re-vendor the source** (in this repo, on a branch off the new `spiceai-<version>`). Clone
   duckdb-vss at the ABI-matched commit somewhere temporary (do NOT `--recursive`; we don't want
   its nested submodules), then replace `extension/vss/src` with its `src/`:
   ```bash
   git clone https://github.com/duckdb/duckdb-vss /tmp/duckdb-vss && \
     git -C /tmp/duckdb-vss checkout <GIT_TAG-from-step-1>
   rm -rf extension/vss/src && cp -R /tmp/duckdb-vss/src extension/vss/src
   git add -A extension/vss/src
   ```
   (Keep it a plain copy — never re-introduce a submodule here; see "Why vendored" above.)

3. **Audit duckdb-vss for structural / API drift** at the new commit:
   - `VssExtension` class name + `src/include/vss_extension.hpp` location unchanged (the
     generated loader does `#include "vss_extension.hpp"` and
     `db.LoadStaticExtension<VssExtension>()`). If renamed, the loader codegen (camel-cased
     extension name) breaks.
   - Source layout still `src/**/*.cpp` so `vss_config.py`'s glob covers it; add include dirs
     to `vss_config.py` if new header roots appear.
   - `src/include/usearch/duckdb_usearch.hpp` for new/renamed `DUCKDB_USEARCH_*` / `USEARCH_*`
     defines. Today only `DUCKDB_USEARCH_USE_SIMSIMD` must be set externally (the wrapper does
     `#define USEARCH_USE_SIMSIMD DUCKDB_USEARCH_USE_SIMSIMD`); `USEARCH_USE_FP16LIB=1` and
     `USEARCH_USE_OPENMP=0` are hardcoded. Any new external define must be added to
     `build_bundled_cc.rs` (`cfg.define(...)`).
   - C++ standard: usearch gates C++17 features behind `#if __cplusplus >= 201703L` and
     otherwise compiles under DuckDB's standard (c++11). If a new usearch hard-requires c++17,
     bump the `-std=` flag in `build_bundled_cc.rs`.
   - DuckDB extension API: `Extension::Load(ExtensionLoader&)` signature / `ExtensionLoadResult`
     enum. If DuckDB changes the static-extension API, the `package_build.py` loader template
     (`extension/generated_extension_loader.cpp.in`) and the duckdb-rs codegen must follow.

4. **Regenerate + validate in duckdb-rs** (after this repo's commit is referenced by the
   `duckdb-sources` submodule there):
   ```bash
   git submodule update --init crates/libduckdb-sys/duckdb-sources   # vss is vendored files; no --recursive needed
   python3 crates/libduckdb-sys/update_sources.py                    # regenerates duckdb.tar.gz + manifest.json
   cargo build -p libduckdb-sys --features bundled                               # compile check
   nm target/debug/build/libduckdb-sys-*/out/libduckdb.a | grep -i 'VssExtension\|HNSWModule'   # symbols present
   ```
   Then build the Spice runtime and confirm: with NO `vss.duckdb_extension` file in
   `~/.duckdb/extensions`, spiced auto-creates the HNSW index at load with no
   INSTALL/download, and `vector_search` / `/v1/search` return results.

## Gotchas

- **Keep vss vendored as files — never a submodule.** duckdb-vss's nested `duckdb`/`extension-ci-tools`
  submodules, pulled in by Cargo's recursive submodule checkout of the duckdb-rs git dep, break the
  Windows build via `MAX_PATH` (see "Why vendored" above). A submodule here regresses Windows.
- The cc build compiles all sources with one flag set, so usearch defines are global; that's
  fine (only usearch headers read them).
- On the spiceai side, patching duckdb to a local/fork source requires patching BOTH
  `[patch.crates-io]` AND `[patch."https://github.com/spiceai/duckdb-rs.git"]` — datafusion-table-providers
  depends on duckdb via the git source, and a split produces two `libduckdb-sys` packages →
  `links = "duckdb"` conflict.
- The runtime search score column is `_score` (not `score`).
