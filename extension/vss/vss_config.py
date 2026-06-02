import os

# VSS (vector similarity search) extension, vendored from github.com/duckdb/duckdb-vss
# into extension/vss/upstream. This mirrors the per-extension `*_config.py` contract used
# by scripts/package_build.py for in-tree extensions (parquet/json/icu): paths are relative
# to the DuckDB source root, which package_build.py chdirs into before importing this module.
#
# usearch, fp16, and simsimd are header-only and live under upstream/src/include, so a single
# include directory covers both the recursive header copy (amalgamation.list_includes_files)
# and the compile-time include path (the sources #include "usearch/...", "hnsw/...", "fp16/...").
prefix = os.path.join('extension', 'vss', 'upstream', 'src')

include_directories = [
    os.path.join('extension', 'vss', 'upstream', 'src', 'include'),
]


def list_files_recursive(rootdir, suffix):
    file_list = []
    for root, _, files in os.walk(rootdir):
        file_list += [os.path.join(root, f) for f in files if f.endswith(suffix)]
    return file_list


# vss_extension.cpp + src/hnsw/*.cpp (usearch/fp16/simsimd contribute no .cpp; header-only).
source_files = list_files_recursive(prefix, '.cpp')
