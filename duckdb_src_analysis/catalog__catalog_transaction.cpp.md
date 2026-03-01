# catalog_transaction.cpp

## File Location
`duckdb/src/catalog/catalog_transaction.cpp`

## Purpose
Implementation of CatalogTransaction - a lightweight wrapper providing transaction context for catalog operations.

## Key Class: CatalogTransaction

### Purpose
Encapsulates transaction information needed for catalog operations without requiring a full transaction object.

### Member Variables:
- `db` - Pointer to DatabaseInstance
- `transaction` - Pointer to Transaction (optional)
- `context` - Pointer to ClientContext (optional)
- `transaction_id` - The transaction's identifier
- `start_time` - The transaction's start timestamp

### Key Methods:

**Constructors:**
- `CatalogTransaction(Catalog &, ClientContext &)` - Creates from catalog and client context
  - Extracts transaction from context
  - Handles both DuckDB and non-DuckDB transactions
  - Sets transaction_id and start_time from DuckTransaction

- `CatalogTransaction(DatabaseInstance &, transaction_id, start_time)` - Direct construction
  - Used for system transactions
  - No associated client context

**Accessors:**
- `GetContext()` - Returns ClientContext
  - Throws InternalException if context is null

**Static Factory Methods:**
- `GetSystemCatalogTransaction(ClientContext &)` - Gets transaction for system catalog
- `GetSystemTransaction(DatabaseInstance &)` - Creates system transaction with ID=1, start_time=1

## Important Concepts

### Transaction ID vs Start Time
- `transaction_id`: Unique identifier for the transaction (>= TRANSACTION_ID_START for active)
- `start_time`: Snapshot timestamp - what data the transaction can see

### Non-DuckDB Transactions
When the underlying transaction is not a DuckTransaction:
- `transaction_id` is set to -1
- `start_time` is set to -1
- These act as "no transaction" markers

### System Transaction
Special transaction with:
- `transaction_id = 1`
- `start_time = 1`
- Used for catalog initialization
- Can see all committed data

## Usage Pattern
```cpp
// Create catalog transaction from client context
CatalogTransaction catalog_trans(catalog, context);

// Use for catalog operations
auto entry = catalog_set.GetEntry(catalog_trans, "mytable");

// System transaction for initialization
auto sys_trans = CatalogTransaction::GetSystemTransaction(db);
```

## Notes
- Lightweight wrapper - minimal overhead
- Enables catalog operations to work with or without full transaction context
- Used extensively throughout catalog code for MVCC operations
- Critical for transactional consistency in catalog operations
