//===----------------------------------------------------------------------===//
//                         DuckDB
//
// generated_extension_headers.hpp
//
// Headers of the extensions linked into this build.
//
// DuckDB's CMake build generates this file; package_build.py (the amalgamated
// build this fork ships to duckdb-rs) does not, so it is written by hand. Each
// include is guarded by the same DUCKDB_EXTENSION_<NAME>_LINKED define the
// build sets.
//===----------------------------------------------------------------------===//

#pragma once

#if DUCKDB_EXTENSION_VSS_LINKED
#include "vss_extension.hpp"
#endif
