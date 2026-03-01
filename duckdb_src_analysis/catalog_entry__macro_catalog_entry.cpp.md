# catalog_entry__macro_catalog_entry.cpp.md

## File Location
`duckdb/src/catalog/catalog_entry/macro_catalog_entry.cpp`

## Purpose
Implements MacroCatalogEntry, ScalarMacroCatalogEntry, and TableMacroCatalogEntry for SQL macro functions.

## Key Classes and Their Purpose

### MacroCatalogEntry
- **Purpose**: Base class for macro catalog entries
- **Key Members**:
  - `macros` - Vector of macro function implementations
  - Distinguishes between SCALAR_MACRO and TABLE_MACRO

### ScalarMacroCatalogEntry
- **Purpose**: Catalog entry for scalar macros (return single value)
- Inherits from MacroCatalogEntry

### TableMacroCatalogEntry
- **Purpose**: Catalog entry for table macros (return table result)
- Inherits from MacroCatalogEntry

## Key Methods with Brief Descriptions

- `MacroCatalogEntry()` - Constructor from CreateMacroInfo
  - Determines macro type from first macro in list

- `GetInfo()` - Returns CreateMacroInfo with macro definitions
- `ToSQL()` - Generates CREATE MACRO SQL statement
- `Copy()` - Creates copy of the entry

## Important Concepts
- **Macro Functions**: User-defined SQL functions
- **Scalar vs Table Macros**: Scalar macros return values, table macros return result sets
- **Macro Overloading**: Multiple macros can have same name with different parameters
