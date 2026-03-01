# catalog_set.cpp

## File Location
`duckdb/src/catalog/catalog_set.cpp`

## Purpose
Implementation of CatalogSet - a container for catalog entries with MVCC (Multi-Version Concurrency Control) support.

## Key Classes

### CatalogEntryMap
Map container for catalog entries with case-insensitive lookup.

#### Key Methods:
- `AddEntry()` - Adds new entry (throws if duplicate exists)
- `UpdateEntry()` - Replaces entry, moving old entry to child chain
- `DropEntry()` - Removes entry from map, handling version chain
- `GetEntry()` - Retrieves entry by name

### CatalogSet
Thread-safe container for catalog entries with transactional support.

#### Member Variables:
- `catalog` - Reference to owning DuckCatalog
- `defaults` - DefaultGenerator for auto-creating default entries
- `map` - CatalogEntryMap containing entries
- `catalog_lock` - Mutex for read operations

#### Key Methods:

**Entry Creation:**
- `CreateEntry()` - Creates new catalog entry with transaction support
  - Validates entry invariants
  - Sets timestamp to transaction ID
  - Registers with dependency manager
  - Handles write-write conflict detection

- `CreateEntryInternal()` - Internal creation logic
- `CreateCommittedEntry()` - Creates entry with timestamp=0 (visible to all)
- `StartChain()` - Initializes version chain with dummy deleted entry
- `VerifyVacancy()` - Checks if entry slot is available

**Entry Retrieval:**
- `GetEntry()` - Retrieves entry visible to transaction
  - Returns nullptr if not found or deleted
  - Respects MVCC visibility rules

- `GetEntryDetailed()` - Returns detailed lookup with failure reason
  - SUCCESS, NOT_PRESENT, DELETED, INVISIBLE

- `GetEntryInternal()` - Gets entry for modification
- `GetEntryForTransaction()` - Traverses version chain for visible entry

**Entry Modification:**
- `AlterEntry()` - Alters existing entry
  - Creates new version in chain
  - Serializes AlterInfo for rollback
  - Handles rename specially

- `RenameEntryInternal()` - Handles rename with RENAMED_ENTRY tombstones
- `AlterOwnership()` - Changes object ownership

**Entry Deletion:**
- `DropEntry()` - Drops entry from catalog
  - Checks dependencies
  - Creates DELETED_ENTRY tombstone
  - Supports CASCADE drops

- `DropEntryInternal()` - Internal drop logic
- `DropDependencies()` - Validates and removes dependencies

**Transaction Operations:**
- `Undo()` - Rolls back entry modification
  - Restores old entry
  - Removes tombstone if applicable

- `CleanupEntry()` - Cleans up committed entry chain
- `CommitDrop()` - Verifies drop can be committed
- `VerifyExistenceOfDependency()` - Validates dependency references

**Scanning:**
- `Scan()` - Iterates all entries visible to transaction
- `ScanWithReturn()` - Scan with early exit capability
- `ScanWithPrefix()` - Scans entries with name prefix

**Default Entries:**
- `CreateDefaultEntry()` - Creates default entry if applicable
- `CreateDefaultEntries()` - Bulk creates all defaults
- `SetDefaultGenerator()` - Sets default entry generator

**Conflict Detection:**
- `HasConflict()` - Checks if timestamp conflicts with transaction
- `CreatedByOtherActiveTransaction()` - Check for concurrent uncommitted changes
- `CommittedAfterStarting()` - Check for committed changes after tx start
- `UseTimestamp()` - Determines if entry is visible to transaction

## Important Concepts

### MVCC Version Chain
Entries form a linked list from newest to oldest:
```
map["name"] → Entry_v3 (newest, timestamp=100)
              ↓ (child pointer)
              Entry_v2 (timestamp=0, committed)
              ↓ (child pointer)
              Entry_v1 (oldest, committed)
```

### Timestamp Visibility Rules
- `timestamp < TRANSACTION_ID_START` (committed before start): **Visible**
- `timestamp == transaction.transaction_id` (created by this tx): **Visible**
- `timestamp > transaction.start_time` (committed after start): **Not visible**

### Tombstone Entries
Special entries marking deleted/altered objects:
- `DELETED_ENTRY` - Object was dropped
- `RENAMED_ENTRY` - Object was renamed (preserves old name)
- `INVALID` - Dummy placeholder for chain initialization

### Write-Write Conflict Detection
When two transactions try to modify the same entry:
- First transaction wins
- Second transaction gets TransactionException
- Must retry after first transaction commits/aborts

### Catalog Invariants
`CheckCatalogEntryInvariants()` enforces:
- Internal entries only in system catalog
- Temporary entries only in temporary catalog
- Non-temporary entries not in temporary catalog
- Default schema is exception for system/internal rules

## Entry States
1. **Active** - Normal entry, not deleted
2. **Deleted** - Tombstone entry (deleted=true)
3. **Invisible** - Entry not visible to current snapshot

## Usage Pattern
```cpp
// Create entry
auto entry = make_uniq<TableCatalogEntry>(...);
catalog_set.CreateEntry(transaction, "mytable", std::move(entry), dependencies);

// Get entry
auto entry = catalog_set.GetEntry(transaction, "mytable");

// Alter entry
catalog_set.AlterEntry(transaction, "mytable", alter_info);

// Drop entry
catalog_set.DropEntry(transaction, "mytable", cascade=false);
```

## Notes
- Core of DuckDB's transactional catalog system
- Enables concurrent access without global locks
- Handles rollback through undo buffer
- Supports lazy creation of default entries
