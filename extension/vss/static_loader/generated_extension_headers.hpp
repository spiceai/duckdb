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

// extension_helper.cpp includes this instead of the individual extension headers
// once GENERATED_EXTENSION_HEADERS is defined, so every linked extension belongs
// here — not only the out-of-tree one this file exists for.
#if DUCKDB_EXTENSION_CORE_FUNCTIONS_LINKED
#include "core_functions_extension.hpp"
#endif
#if DUCKDB_EXTENSION_PARQUET_LINKED
#include "parquet_extension.hpp"
#endif
#if DUCKDB_EXTENSION_JSON_LINKED
#include "json_extension.hpp"
#endif
#if DUCKDB_EXTENSION_ICU_LINKED
#include "icu_extension.hpp"
#endif
#if DUCKDB_EXTENSION_VSS_LINKED
#include "vss_extension.hpp"
#endif
