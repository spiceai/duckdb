//===----------------------------------------------------------------------===//
//                         DuckDB
//
// vss_static_loader.cpp
//
// Registers the statically linked out-of-tree extensions at database open.
//
// DuckDB's CMake build generates this loader; package_build.py does not, so the
// amalgamated build this fork ships to duckdb-rs supplies it here.
//
// Defining GENERATED_EXTENSION_HEADERS makes ExtensionHelper::LoadExtensionInternal
// return on TryLoadLinkedExtension rather than fall through to its in-tree ladder,
// so this function must serve every extension the build links, not only vss.
// LinkedExtensions() reports only vss: LoadAllExtensions already walks the in-tree
// names, and repeating them there would load them twice.
//===----------------------------------------------------------------------===//

#include "duckdb/main/extension/generated_extension_loader.hpp"

#if defined(GENERATED_EXTENSION_HEADERS) && GENERATED_EXTENSION_HEADERS

namespace duckdb {

bool TryLoadLinkedExtension(DuckDB &db, const string &extension) {
#if DUCKDB_EXTENSION_CORE_FUNCTIONS_LINKED
	if (extension == "core_functions") {
		db.LoadStaticExtension<CoreFunctionsExtension>();
		return true;
	}
#endif
#if DUCKDB_EXTENSION_PARQUET_LINKED
	if (extension == "parquet") {
		db.LoadStaticExtension<ParquetExtension>();
		return true;
	}
#endif
#if DUCKDB_EXTENSION_JSON_LINKED
	if (extension == "json") {
		db.LoadStaticExtension<JsonExtension>();
		return true;
	}
#endif
#if DUCKDB_EXTENSION_ICU_LINKED
	if (extension == "icu") {
		db.LoadStaticExtension<IcuExtension>();
		return true;
	}
#endif
#if DUCKDB_EXTENSION_VSS_LINKED
	if (extension == "vss") {
		db.LoadStaticExtension<VssExtension>();
		return true;
	}
#endif
	return false;
}

vector<string> LinkedExtensions() {
	return {
#if DUCKDB_EXTENSION_VSS_LINKED
	    "vss",
#endif
	};
}

vector<string> LoadedExtensionTestPaths() {
	return {};
}

} // namespace duckdb

#endif
