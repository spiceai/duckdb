import os

# VSS (vector similarity search) extension, vendored from github.com/duckdb/duckdb-vss
# (commit b833341c) into extension/vss/src. The source is committed directly rather than pulled
# via a git submodule: duckdb-vss has its own nested `duckdb`/`extension-ci-tools` submodules,
# and because Cargo recursively checks out submodules of the duckdb-rs git dependency, that nested
# duckdb checkout exceeded Windows' MAX_PATH and broke the Windows build. A vendored copy has no
# submodules to recurse into.
#
# This mirrors the per-extension `*_config.py` contract used by scripts/package_build.py for
# in-tree extensions (parquet/json/icu): paths are relative to the DuckDB source root, which
# package_build.py chdirs into before importing this module.
#
# usearch, fp16, and simsimd are header-only and live under src/include, so a single include
# directory covers both the recursive header copy (amalgamation.list_includes_files) and the
# compile-time include path (the sources #include "usearch/...", "hnsw/...", "fp16/...").
prefix = os.path.join('extension', 'vss', 'src')

include_directories = [
    os.path.join('extension', 'vss', 'src', 'include'),
]


def list_files_recursive(rootdir, suffix):
    file_list = []
    for root, _, files in os.walk(rootdir):
        file_list += [os.path.join(root, f) for f in files if f.endswith(suffix)]
    return file_list


# vss_extension.cpp + src/hnsw/*.cpp (usearch/fp16/simsimd contribute no .cpp; header-only).
source_files = list_files_recursive(prefix, '.cpp')
