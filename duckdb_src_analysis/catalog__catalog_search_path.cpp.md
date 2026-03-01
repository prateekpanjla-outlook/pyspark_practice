# catalog_search_path.cpp

## File Location
`duckdb/src/catalog/catalog_search_path.cpp`

## Purpose
Implementation of catalog search path - defines the order and scope of schema searching when resolving unqualified object names.

## Key Classes

### CatalogSearchEntry
Represents a single catalog.schema entry in the search path.

#### Member Variables:
- `catalog` - Catalog name (can be empty for default)
- `schema` - Schema name

#### Key Methods:

**Parsing:**
- `Parse()` - Parses a single entry from string (e.g., "catalog.schema" or just "schema")
- `ParseInternal()` - Internal parsing with state machine
  - Handles quoted identifiers with double quotes
  - Handles escaped quotes ("")
  - Throws ParserException on malformed input

- `ParseList()` - Parses comma-separated list of entries

**String Conversion:**
- `ToString()` - Converts entry to string representation
- `WriteOptionallyQuoted()` - Adds quotes if name contains special characters (.,,")

- `ListToString()` - Converts vector of entries to comma-separated string

### CatalogSearchPath
Manages the complete search path for a client context.

#### Member Variables:
- `context` - Associated ClientContext
- `paths` - Full search path including system paths
- `set_paths` - User-configured portion of search path

#### Key Methods:

**Path Management:**
- `Set()` - Sets new search paths with validation
  - Validates that schemas exist
  - Handles catalog-only references (converts to catalog.default_schema)
  - Prevents setting to internal schemas (TEMP, SYSTEM)

- `Reset()` - Resets to default search path
- `Get()` - Returns current search path (excluding empty schema entries)

**Querying:**
- `GetDefault()` - Returns the primary/default search entry
- `GetDefaultSchema()` - Gets default schema for a catalog
- `GetDefaultCatalog()` - Gets catalog for a schema
- `GetCatalogsForSchema()` - Returns all catalogs containing a schema
- `GetSchemasForCatalog()` - Returns all schemas in a catalog
- `SchemaInSearchPath()` - Checks if a schema is in the search path

**Internal:**
- `SetPathsInternal()` - Internal path setting with system path prepending
  - Always includes: TEMP, user paths, INVALID, SYSTEM.default, SYSTEM.pg_catalog

## Search Path Structure

The full search path (in order):
1. **TEMP_CATALOG / DEFAULT_SCHEMA** - Temporary objects
2. **User-configured paths** - Set via SET search_path
3. **INVALID_CATALOG / DEFAULT_SCHEMA** - Default database
4. **SYSTEM_CATALOG / DEFAULT_SCHEMA** - System objects
5. **SYSTEM_CATALOG / pg_catalog** - PostgreSQL compatibility

## CatalogSetPathType
Enum defining how the search path is set:
- `SET_SCHEMA` - Single schema (e.g., SET schema 'myschema')
- `SET_SCHEMAS` - Multiple schemas (e.g., SET search_path 'a,b,c')
- `SET_DIRECTLY` - Direct setting without verification

## Important Concepts

### Qualified vs Unqualified Names
- **Unqualified** (just "tablename"): Searches entire search path
- **Schema-qualified** ("schema.tablename"): Searches specified schema in all catalogs
- **Fully-qualified** ("catalog.schema.tablename"): Direct lookup

### Default Schema Resolution
When no schema is specified:
1. Search temporary catalog
2. Search user-configured paths
3. Search default database
4. Search system catalog

### PostgreSQL Compatibility
- `pg_catalog` is always included in search path
- Mirrors PostgreSQL's search_path behavior

## Usage Example
```sql
-- Set primary schema
SET schema = myschema;

-- Set multiple search paths
SET search_path = myschema, public, system;

-- Fully qualified query
SELECT * FROM mydb.myschema.table;

-- Schema qualified
SELECT * FROM myschema.table;

-- Unqualified (searches all paths)
SELECT * FROM table;
```

## Notes
- Search path is per-client-context (not global)
- Affects name resolution in all SQL statements
- Critical for PostgreSQL compatibility
