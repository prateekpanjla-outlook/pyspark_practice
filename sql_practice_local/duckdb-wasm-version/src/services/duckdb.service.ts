/**
 * DuckDB-Wasm Service
 * Wraps @duckdb/duckdb-wasm for SQL execution
 */

import * as duckdb from '@duckdb/duckdb-wasm';
import type { QuestionSchema, Question, QueryResult } from '../types/question.types';

export class DuckDBService {
  private db: duckdb.AsyncDuckDB | null = null;
  private conn: duckdb.AsyncDuckDBConnection | null = null;
  private worker: Worker | null = null;
  private initialized = false;

  /**
   * Initialize DuckDB-Wasm with COOP/COEP headers
   */
  async initialize(): Promise<void> {
    if (this.initialized) return;

    const startTime = performance.now();
    console.log('🔄 Initializing DuckDB-Wasm...');

    try {
      // Add timeout
      const timeoutPromise = new Promise<never>((_, reject) => {
        setTimeout(() => reject(new Error('DuckDB initialization timeout (15s)')), 15000);
      });

      // Select the appropriate bundle
      console.log('1️⃣ Selecting WASM bundle...');
      const bundle = await duckdb.selectBundle({
        mvp: {
          mainModule: '/duckdb-wasm/duckdb-mvp.wasm',
          mainWorker: '/duckdb-wasm/duckdb-mvp.worker.js',
        },
        eh: {
          mainModule: '/duckdb-wasm/duckdb-eh.wasm',
          mainWorker: '/duckdb-wasm/duckdb-eh.worker.js',
        },
      });
      console.log('✅ Bundle selected:', bundle.mainModule);

      // Create worker
      console.log('2️⃣ Creating worker...');
      this.worker = new Worker(bundle.mainWorker!);
      const logger = new duckdb.ConsoleLogger();

      // Create database instance
      console.log('3️⃣ Creating AsyncDuckDB instance...');
      this.db = new duckdb.AsyncDuckDB(logger, this.worker);

      console.log('4️⃣ Instantiating WASM module...');
      await Promise.race([
        this.db.instantiate(bundle.mainModule, bundle.pthreadWorker),
        timeoutPromise
      ]);

      console.log('5️⃣ Opening database...');
      await this.db.open({
        path: ':memory:',
        query: {
          castStrategy: 'cast_bigint_to_string',
        },
      });

      // Create a connection for queries
      console.log('6️⃣ Creating connection...');
      this.conn = await this.db.connect();

      this.initialized = true;
      const elapsed = (performance.now() - startTime).toFixed(0);
      console.log(`✅ DuckDB-Wasm initialized in ${elapsed}ms`);
    } catch (error) {
      const elapsed = ((performance.now() - startTime) / 1000).toFixed(1);
      console.error(`❌ Failed to initialize DuckDB-Wasm after ${elapsed}s:`, error);

      // Provide helpful error message
      if (error instanceof Error && error.message.includes('SharedArrayBuffer')) {
        console.error('💡 COOP/COEP headers are missing. This app requires them for DuckDB-Wasm.');
        console.error('💡 In production, Netlify/Vercel configs handle this automatically.');
        console.error('💡 In dev, Vite should set these - check vite.config.ts');
      }

      throw error;
    }
  }

  /**
   * Initialize database schema for a question
   * Drops existing tables first to ensure clean state
   */
  async initializeSchema(schema: QuestionSchema): Promise<void> {
    if (!this.conn) {
      throw new Error('DuckDB not initialized. Call initialize() first.');
    }

    try {
      // Drop existing tables first to ensure clean state
      for (const table of schema.tables) {
        try {
          await this.conn.query(`DROP TABLE IF EXISTS ${table.name} CASCADE`);
        } catch {
          // Table might not exist, ignore error
        }
      }

      // Create tables
      for (const table of schema.tables) {
        const columns = table.columns.map((c) => `${c.name} ${c.type}`).join(', ');
        await this.conn.query(`CREATE TABLE ${table.name} (${columns})`);
      }

      // Insert sample data
      for (const table of schema.tables) {
        const sampleData = schema.sampleData[table.name];
        if (!sampleData) continue;

        for (const row of sampleData) {
          const values = table.columns
            .map((c) => this.formatValue(row[c.name], c.type))
            .join(', ');
          await this.conn.query(`INSERT INTO ${table.name} VALUES (${values})`);
        }
      }

      console.log(`✅ Schema initialized: ${schema.tables.map((t) => t.name).join(', ')}`);
    } catch (error) {
      console.error('❌ Failed to initialize schema:', error);
      throw error;
    }
  }

  /**
   * Execute SQL query
   */
  async executeQuery(sql: string): Promise<QueryResult> {
    if (!this.conn) {
      throw new Error('DuckDB not initialized. Call initialize() first.');
    }

    const start = performance.now();

    try {
      const result = await this.conn.query(sql);
      const duration = performance.now() - start;

      // Convert DuckDB result to our format
      const columns: string[] = [];
      const rows: Record<string, string | number | null>[] = [];

      // Get column names
      for (let i = 0; i < result.schema.fields.length; i++) {
        columns.push(result.schema.fields[i].name);
      }

      // Get rows - Arrow toArray() returns objects with column names as keys
      for (const row of result.toArray()) {
        const rowObj: Record<string, string | number | null> = {};
        for (const col of columns) {
          const value = row[col];
          rowObj[col] = value === null ? null : String(value);
        }
        rows.push(rowObj);
      }

      return {
        success: true,
        columns,
        rows,
        executionTimeMs: duration,
        rowCount: result.numRows,
      };
    } catch (error) {
      const duration = performance.now() - start;
      return {
        success: false,
        columns: [],
        rows: [],
        executionTimeMs: duration,
        rowCount: 0,
        errorMessage: error instanceof Error ? error.message : String(error),
      };
    }
  }

  /**
   * Compare actual result with expected output using set-based comparison
   * - Column names must match exactly (same set, order-independent)
   * - Rows must match as multisets (order-independent, duplicates matter)
   * - No extra columns or rows allowed
   */
  compareResults(actual: QueryResult, expected: QueryResult): boolean {
    // Check column count
    if (actual.columns.length !== expected.columns.length) return false;

    // Check column names match as sets (order-independent)
    const actualCols = new Set(actual.columns);
    const expectedCols = new Set(expected.columns);
    for (const col of expectedCols) {
      if (!actualCols.has(col)) return false;
    }

    // Check row count
    if (actual.rowCount !== expected.rowCount) return false;

    // Convert rows to canonical string representation for multiset comparison
    const toCanonical = (row: Record<string, string | number | null>, cols: string[]): string => {
      return cols.map(col => {
        const val = row[col];
        return val === null ? '\x00NULL\x00' : String(val);
      }).join('\x01'); // Use special separator
    };

    // Count occurrences in expected (multiset)
    const expectedCounts = new Map<string, number>();
    for (const row of expected.rows) {
      const key = toCanonical(row, expected.columns);
      expectedCounts.set(key, (expectedCounts.get(key) || 0) + 1);
    }

    // Count occurrences in actual (multiset) and compare
    const actualCounts = new Map<string, number>();
    for (const row of actual.rows) {
      const key = toCanonical(row, expected.columns); // use expected column order
      actualCounts.set(key, (actualCounts.get(key) || 0) + 1);
    }

    // Compare multisets - every expected row must match with same count
    if (expectedCounts.size !== actualCounts.size) return false;
    for (const [key, count] of expectedCounts) {
      if (actualCounts.get(key) !== count) return false;
    }

    return true;
  }

  /**
   * Format a value for SQL INSERT statement
   */
  private formatValue(value: unknown, type: string): string {
    if (value === null || value === undefined) return 'NULL';
    if (typeof value === 'string') {
      // Escape single quotes
      const escaped = value.replace(/'/g, "''");
      return `'${escaped}'`;
    }
    return String(value);
  }

  /**
   * Shutdown the database connection
   */
  async shutdown(): Promise<void> {
    if (this.conn) {
      await this.conn.close();
      this.conn = null;
    }
    if (this.db) {
      await this.db.terminate();
      this.db = null;
    }
    if (this.worker) {
      this.worker.terminate();
      this.worker = null;
    }
    this.initialized = false;
  }

  isInitialized(): boolean {
    return this.initialized;
  }
}
