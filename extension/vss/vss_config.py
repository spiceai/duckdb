import os

# VSS (vector similarity search / HNSW), vendored from github.com/duckdb/duckdb-vss at the
# commit .github/config/extensions/vss.cmake pins for this DuckDB release.
#
# Same per-extension contract scripts/package_build.py uses for the in-tree extensions
# (parquet/json/icu): paths are relative to the DuckDB source root, which package_build.py
# chdirs into before importing this module.
#
# `src` is a verbatim copy of the upstream tree and is replaced wholesale when re-vendoring;
# `static_loader` is ours. usearch, fp16 and simsimd are header-only and ship inside
# duckdb-vss under src/include, so one include directory covers them all.

VENDORED = os.path.join('extension', 'vss', 'src')
STATIC_LOADER = os.path.join('extension', 'vss', 'static_loader')

include_directories = [
    os.path.join(VENDORED, 'include'),
    STATIC_LOADER,
]


def _cpp_files(root):
    return [
        os.path.join(dirpath, f) for dirpath, _, filenames in os.walk(root) for f in filenames if f.endswith('.cpp')
    ]


source_files = _cpp_files(VENDORED) + _cpp_files(STATIC_LOADER)
