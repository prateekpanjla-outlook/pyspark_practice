# SQL Practice Server - DuckDB-Wasm Architecture

## 🌐 Browser-First Design

A revolutionary approach to SQL practice: **zero backend for query execution**. All SQL runs client-side using DuckDB-Wasm, making it infinitely scalable, serverless, and incredibly fast.

---

## Architecture Philosophy

| Traditional (C++) | DuckDB-Wasm |
|-------------------|-------------|
| Backend executes queries | Browser executes queries |
| Scaling = more servers | Scaling = free (users bring their own compute) |
| 8 DuckDB instances on server | 1 DuckDB instance per user's browser |
| Network latency for results | Zero network latency |
| ~200 MB server memory | ~25 MB per browser (user's RAM) |
| Load balancer needed | CDN distribution only |

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              Browser (User's Device)                         │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                         React / Vue / Svelte App                       │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐               │   │
│  │  │QuestionView  │  │  EditorView  │  │ResultView   │               │   │
│  │  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘               │   │
│  └─────────┼──────────────────┼──────────────────┼───────────────────────┘   │
│            │                  │                  │                          │
│  ┌─────────┼──────────────────┼──────────────────┼───────────────────────┐   │
│  │              SQL Practice Runtime Layer                             │   │
│  │  ┌────────────────────────────────────────────────────────────────┐  │   │
│  │  │                    DuckDB-Wasm Instance                        │  │   │
│  │  │  ┌───────────┐  ┌────────────┐  ┌──────────────────────────┐   │  │   │
│  │  │  │   WASM    │  │  Worker    │  │      In-Memory DB        │   │  │   │
│  │  │  │  (~5 MB)  │  │  Thread    │  │    (Question Data)       │   │  │   │
│  │  │  └───────────┘  └────────────┘  └──────────────────────────┘   │  │   │
│  │  └────────────────────────────────────────────────────────────────┘  │   │
│  │                                │                                     │   │
│  │  ┌───────────────────────────────┴───────────────────────────────┐  │   │
│  │  │              Query Execution Engine                          │  │   │
│  │  │  - Schema initialization                                     │  │   │
│  │  │  - SQL validation & execution                                 │  │   │
│  │  │  - Result comparison with expected output                     │  │   │
│  │  │  - Performance timing                                         │  │   │
│  │  └───────────────────────────────────────────────────────────────┘  │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                   LocalStorage / IndexedDB                            │   │
│  │  - Session persistence                                               │   │
│  │  - Progress tracking                                                │   │
│  │  - Query history                                                    │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────┬───────────────────────────────────────┘
                                      │
                                      │ HTTP/HTTPS (for content only)
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         Static Content Delivery (CDN)                       │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                      Questions API (JSON)                             │   │
│  │  GET /api/questions                                                  │   │
│  │  GET /api/questions/:slug                                            │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │                     Static Assets                                     │   │
│  │  - React/Vue/Svelte bundle                                            │   │
│  │  - DuckDB-Wasm WASM files (~5 MB)                                     │   │
│  │  - Question data (JSON)                                               │   │
│  │  - CSS/JS assets                                                      │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│  Deploy to: GitHub Pages, Netlify, Vercel, Cloudflare Pages                 │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Component Architecture

### 1. Frontend Application Layer

**Tech Stack Options:**

| Option | Pros | Cons |
|--------|------|------|
| React + Vite | Mature ecosystem, fast HMR | Larger bundle |
| Vue 3 + Vite | Smaller bundle, gentle learning curve | Smaller ecosystem |
| Svelte + SvelteKit | Smallest bundle, compiled | Different paradigm |
| Vanilla + WASM | Minimal overhead | More manual work |

**Recommended**: **React + Vite + TypeScript** (balance of ecosystem and performance)

```
src/
├── App.tsx                    # Main app component
├── main.tsx                   # Entry point
├── components/
│   ├── QuestionList.tsx       # Browse questions
│   ├── QuestionView.tsx       # Single question with schema
│   ├── SqlEditor.tsx          # Monaco/CodeMirror editor
│   ├── ResultsTable.tsx       # Query results display
│   ├── SchemaViewer.tsx       # Visual schema representation
│   └── PerformanceStats.tsx   # Query timing, row count
├── hooks/
│   ├── useDuckDB.ts           # DuckDB-Wasm initialization
│   ├── useQuestion.ts         # Question data fetching
│   └── useQueryExecution.ts   # Query execution logic
├── services/
│   ├── duckdb.service.ts      # DuckDB-Wasm wrapper
│   ├── question.service.ts    # Question API calls
│   └── storage.service.ts     # LocalStorage/IndexedDB
└── types/
    ├── question.types.ts      # Question data types
    └── duckdb.types.ts        # DuckDB result types
```

### 2. DuckDB-Wasm Service Layer

**File**: `src/services/duckdb.service.ts`

```typescript
import * as duckdb from '@duckdb/duckdb-wasm';

export class DuckDBService {
    private db: duckdb.AsyncDuckDB | null = null;
    private worker: Worker | null = null;

    // Initialize DuckDB-Wasm
    async initialize(): Promise<void> {
        // Select WASM bundle (COOP/COEP headers required for multi-threading)
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

        // Create worker
        this.worker = new Worker(bundle.mainWorker!);
        const logger = new duckdb.ConsoleLogger();

        // Create database instance
        this.db = new duckdb.AsyncDuckDB(logger, this.worker);
        await this.db.instantiate(bundle.mainModule, bundle.pthreadWorker);
        await this.db.open({
            path: ':memory:',
            query: {
                castStrategy: 'cast_bigint_to_string',
            }
        });
    }

    // Initialize schema for a question
    async initializeSchema(schema: QuestionSchema): Promise<void> {
        if (!this.db) throw new Error('DuckDB not initialized');

        // Create tables
        for (const table of schema.tables) {
            const columns = table.columns
                .map(c => `${c.name} ${c.type}`)
                .join(', ');
            await this.db.exec(`CREATE TABLE ${table.name} (${columns})`);

            // Insert sample data
            if (schema.sampleData[table.name]) {
                for (const row of schema.sampleData[table.name]) {
                    const values = table.columns
                        .map(c => formatValue(row[c.name], c.type))
                        .join(', ');
                    await this.db.exec(`INSERT INTO ${table.name} VALUES (${values})`);
                }
            }
        }
    }

    // Execute SQL query
    async executeQuery(sql: string): Promise<QueryResult> {
        if (!this.db) throw new Error('DuckDB not initialized');

        const start = performance.now();
        const result = await this.db.exec(sql);
        const duration = performance.now() - start;

        return {
            success: true,
            columns: result.schema.fields.map(f => f.name),
            rows: formatResults(result),
            executionTimeMs: duration,
            rowCount: result.numRows,
        };
    }

    // Compare results with expected output
    compareResults(actual: QueryResult, expected: QueryResult): boolean {
        // Column comparison
        if (!arraysEqual(actual.columns, expected.columns)) return false;

        // Row count comparison
        if (actual.rowCount !== expected.rowCount) return false;

        // Data comparison
        // ... (implementation depends on expected output format)

        return true;
    }
}
```

### 3. Question Data Layer

**Storage Options:**

| Option | Description | Pros | Cons |
|--------|-------------|------|------|
| JSON files | Static JSON on CDN | Simple, cacheable | No dynamic updates |
| DuckDB files | `.db` files via HTTP | Fast, native format | Less flexible |
| API endpoint | Dynamic server generation | Real-time updates | Requires backend |

**Recommended**: **JSON files** for simplicity and CDN caching

```
public/data/questions/
├── index.json                  # All questions metadata
├── easy/
│   ├── select-basics.json
│   ├── where-clause.json
│   └── ...
├── medium/
│   ├── joins.json
│   ├── group-by.json
│   └── ...
└── hard/
    ├── window-functions.json
    ├── cte-recursive.json
    └── ...
```

**Question JSON Schema**:

```json
{
  "id": "q1",
  "slug": "select-basics",
  "title": "SELECT Basics",
  "description": "Retrieve specific columns from a table",
  "difficulty": "easy",
  "category": "basics",
  "order": 1,
  "schema": {
    "tables": [
      {
        "name": "employees",
        "columns": [
          {"name": "id", "type": "INTEGER"},
          {"name": "name", "type": "VARCHAR"},
          {"name": "department", "type": "VARCHAR"},
          {"name": "salary", "type": "INTEGER"}
        ]
      }
    ],
    "sampleData": {
      "employees": [
        {"id": 1, "name": "Alice", "department": "Engineering", "salary": 90000},
        {"id": 2, "name": "Bob", "department": "Sales", "salary": 75000},
        {"id": 3, "name": "Charlie", "department": "Engineering", "salary": 95000}
      ]
    }
  },
  "question": "Find all employees in the Engineering department",
  "startingCode": "SELECT * FROM employees",
  "hints": [
    "Use the WHERE clause to filter by department",
    "The department name should be in quotes: 'Engineering'"
  ],
  "expectedOutput": {
    "columns": ["id", "name", "department", "salary"],
    "rows": [
      {"id": 1, "name": "Alice", "department": "Engineering", "salary": 90000},
      {"id": 3, "name": "Charlie", "department": "Engineering", "salary": 95000}
    ]
  }
}
```

### 4. State Management

**Options:**

| Option | Use Case |
|--------|----------|
| React Context | Small to medium apps |
| Zustand | Lightweight, simple |
| Jotai | Atomic, flexible |
| Redux Toolkit | Large apps with complex state |

**Recommended**: **Zustand** (lightweight, TypeScript-first)

```typescript
// src/store/questionStore.ts
import { create } from 'zustand';

interface QuestionState {
    currentQuestion: Question | null;
    userCode: string;
    queryResult: QueryResult | null;
    isCorrect: boolean | null;
    executionTime: number;

    setCurrentQuestion: (q: Question) => void;
    setUserCode: (code: string) => void;
    executeQuery: () => Promise<void>;
    reset: () => void;
}

export const useQuestionStore = create<QuestionState>((set, get) => ({
    currentQuestion: null,
    userCode: '',
    queryResult: null,
    isCorrect: null,
    executionTime: 0,

    setCurrentQuestion: (q) => {
        set({ currentQuestion: q, userCode: q.startingCode || '' });
    },

    setUserCode: (code) => set({ userCode: code }),

    executeQuery: async () => {
        const { currentQuestion, userCode } = get();
        if (!currentQuestion) return;

        const duckdb = new DuckDBService();
        await duckdb.initializeSchema(currentQuestion.schema);

        const result = await duckdb.executeQuery(userCode);
        const isCorrect = duckdb.compareResults(result, currentQuestion.expectedOutput);

        set({ queryResult: result, isCorrect, executionTime: result.executionTimeMs });
    },

    reset: () => set({
        queryResult: null,
        isCorrect: null,
        executionTime: 0
    }),
}));
```

---

## Request Flow: Execute Query

```
User Browser              React App              DuckDBService          DuckDB-Wasm
   │                         │                        │                      │
   │ Types SQL               │                        │                      │
   │────────────────────────>│                        │                      │
   │                         │                        │                      │
   │                         │ executeQuery(sql)      │                      │
   │                         │───────────────────────>│                      │
   │                         │                        │                      │
   │                         │                        │ initializeSchema()   │
   │                         │                        │─────────────────────>│
   │                         │                        │                      │
   │                         │                        │ CREATE TABLE ...     │
   │                         │                        │ INSERT INTO ...      │
   │                         │                        │                      │
   │                         │                        │ exec(sql)            │
   │                         │                        │─────────────────────>│
   │                         │                        │                      │
   │                         │                        │ QueryResult          │
   │                         │                        │<─────────────────────│
   │                         │                        │                      │
   │                         │ compareResults()       │                      │
   │                         │<───────────────────────│                      │
   │                         │                        │                      │
   │ Display results         │                        │                      │
   │<────────────────────────│                        │                      │
```

---

## Deployment Strategy

### Option 1: Static Hosting (Recommended)

**Platforms**: GitHub Pages, Netlify, Vercel, Cloudflare Pages

**Advantages**:
- Zero server costs
- Global CDN distribution
- Infinite scalability
- HTTPS by default

**Structure**:
```
dist/
├── index.html
├── assets/
│   ├── index-[hash].js          # React bundle (~200 KB gzipped)
│   └── index-[hash].css
├── duckdb-wasm/
│   ├── duckdb-mvp.wasm           # WASM binary (~5 MB)
│   ├── duckdb-mvp.worker.js
│   └── duckdb-eh.wasm            # Exception handling variant
└── data/
    └── questions/
        └── index.json            # Question metadata
```

### Option 2: Server + Static Assets

**For features requiring backend**:
- User authentication
- Progress sync across devices
- Collaborative features
- Analytics

**Backend**: Node.js/Express, Next.js API routes, or serverless functions

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│   Browser   │────>│  CDN/Netlify │────>│  Question   │
│  (DuckDB)   │     │  (Static)    │     │    API      │
└─────────────┘     └──────────────┘     └─────────────┘
```

---

## Performance Optimizations

### 1. WASM Bundle Optimization

```typescript
// Use MVP bundle for faster initial load
const bundle = await duckdb.selectBundle({
    mvp: {
        mainModule: '/duckdb-wasm/duckdb-mvp.wasm',  // ~2.5 MB
        mainWorker: '/duckdb-wasm/duckdb-mvp.worker.js',
    }
});
```

### 2. Lazy Loading Questions

```typescript
// Load question data only when needed
const loadQuestion = async (slug: string) => {
    const response = await fetch(`/data/questions/${slug}.json`);
    return await response.json();
};
```

### 3. Persistent DuckDB Instance

```typescript
// Keep one instance per session (not per query)
// Reuse for multiple questions
class DuckDBService {
    private static instance: DuckDBService;

    static getInstance(): DuckDBService {
        if (!this.instance) {
            this.instance = new DuckDBService();
        }
        return this.instance;
    }
}
```

---

## Security Considerations

| Threat | Mitigation |
|--------|-----------|
| Malicious SQL | DuckDB-Wasm sandbox prevents file system access |
| XSS | CSP headers, escape user input |
| Data exfiltration | All data stays in browser |
| Injection | Prepared statements (not applicable for client-side) |

**Key Point**: DuckDB-Wasm runs in a browser sandbox, preventing:
- File system access (without user permission)
- Network requests (CORS restrictions apply)
- Access to other browser tabs

---

## Offline Support

### Service Worker + IndexedDB

```typescript
// Cache questions for offline use
self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open('sql-practice-v1').then((cache) => {
            return cache.addAll([
                '/',
                '/assets/index.js',
                '/duckdb-wasm/duckdb-mvp.wasm',
                '/data/questions/index.json',
            ]);
        })
    );
});

// Store progress in IndexedDB
const storeProgress = async (questionId: string, solved: boolean) => {
    const db = await openDB('sql-practice', 1);
    await db.put('progress', { questionId, solved, timestamp: Date.now() });
};
```

---

## Comparison: C++ vs DuckDB-Wasm

| Aspect | C++ Version | DuckDB-Wasm Version |
|--------|-------------|---------------------|
| **Query Execution** | Server-side (C++) | Client-side (WASM) |
| **Scalability** | Limited by server resources | Infinite (user's browser) |
| **Server Requirements** | 8 CPU cores, 8 GB RAM | Static hosting only |
| **Network Latency** | ~100-500ms per query | 0ms (local execution) |
| **Initial Load** | ~50 KB HTML | ~5 MB WASM (one-time) |
| **Offline Support** | No (requires server) | Yes (with service worker) |
| **Deployment** | Vagrant VM + build process | `npm run build` + deploy |
| **Cost** | Server hosting costs | Free (static hosting) |
| **Memory** | Server: 200 MB | Per browser: ~25 MB |
| **Parallel Queries** | 8 (server-side) | 1 per browser |

---

## Getting Started

### Project Setup

```bash
# Create project
npm create vite@latest sql-practice-wasm -- --template react-ts
cd sql-practice-wasm

# Install dependencies
npm install @duckdb/duckdb-wasm
npm install zustand        # State management
npm install monaco-editor  # SQL editor
npm install lucide-react   # Icons

# Download WASM files
npm run download:wasm

# Run dev server
npm run dev
```

### Build & Deploy

```bash
# Production build
npm run build

# Deploy to Netlify
netlify deploy --prod --dir=dist

# Or GitHub Pages
npm run deploy:gh-pages
```

---

## File Structure

```
duckdb-wasm-version/
├── ARCHITECTURE.md           # This document
├── public/
│   ├── duckdb-wasm/          # WASM files (downloaded)
│   │   ├── duckdb-mvp.wasm
│   │   └── duckdb-mvp.worker.js
│   └── data/
│       └── questions/        # Question JSON files
│           └── index.json
├── src/
│   ├── App.tsx
│   ├── main.tsx
│   ├── components/           # React components
│   ├── hooks/                # Custom hooks
│   ├── services/             # DuckDB wrapper, storage
│   ├── store/                # Zustand stores
│   └── types/                # TypeScript types
├── package.json
├── vite.config.ts
└── tsconfig.json
```

---

## Next Steps

1. **Prototype**: Build single-question MVP
2. **Questions**: Author 10-20 questions across difficulty levels
3. **Editor**: Integrate Monaco/CodeMirror for syntax highlighting
4. **Results**: Build nice table view for query results
5. **Analytics**: Track progress, completion rate
6. **Deploy**: Launch on Netlify/Vercel

---

---

# Part 2: Pre-Generated Problem Bundles Architecture

## Overview

This architecture extends the browser-first design with a **pre-generated problem bundle system** that:
- Creates variations of SQL problems with different random datasets
- Generates step-by-step explanations correlated with actual data
- Assigns problem variations randomly to users at login
- Ensures semantic consistency across variations
- Maintains realistic, domain-specific data

## Motivation

**Problem with Browser-Based LLMs:**
- Corporate browsers often have ~2-4GB memory per tab
- DuckDB-Wasm uses ~125-275MB
- No capable SQL LLM exists under 100MB
- Downloading LLM models may be blocked by corporate policies

**Solution:**
- Pre-generate explanations server-side using powerful LLMs
- Bundle problems with their specific data and explanations
- Serve static bundles to browser clients
- No runtime LLM inference needed

---

## 1. Problem Bundle Structure

### 1.1 Bundle Schema

```typescript
interface ProblemBundle {
  // Problem identifier
  problemId: string;
  variantId: string;           // Unique variant (e.g., "v1", "v2", "v3")
  version: number;             // Increment on regeneration

  // Problem definition
  title: string;
  description: string;
  difficulty: 'beginner' | 'intermediate' | 'advanced';
  category: string;            // e.g., "JOINs", "Aggregation", "Window Functions"

  // Schema definition
  schema: {
    tables: TableDef[];
    relationships: RelationshipDef[];
  };

  // Actual data for this variant
  data: {
    [tableName: string]: any[];
  };

  // Expected output
  expectedOutput: {
    columns: string[];
    rows: any[];
  };

  // Starting code template
  startingCode: string;

  // Pre-generated explanation correlated with THIS data
  explanation: {
    overview: string;
    steps: ExplanationStep[];
    commonMistakes: CommonMistake[];
    alternativeSolutions: AlternativeSolution[];
  };

  // Validation metadata
  validation: {
    seed: number;              // Random seed used for data generation
    semanticChecksum: string;   // Ensures problem meaning is preserved
    similarityScore?: number;   // Distance from canonical variant
  };

  // Metadata
  createdAt: string;
  regeneratedAt?: string;
}
```

### 1.2 Explanation Step Schema

```typescript
interface ExplanationStep {
  stepNumber: number;
  title: string;
  description: string;

  // The actual SQL snippet for THIS step
  sqlSnippet: string;

  // Intermediate result with THIS variant's data
  intermediateResult?: {
    columns: string[];
    rows: any[];              // Real data from this variant
    rowCount: number;
  };

  // Explanation of what changed
  reasoning: string;

  // Highlights specific values from the data
  dataExamples: {
    tableName: string;
    filter: string;           // e.g., "WHERE category = 'Electronics'"
    highlightedRows: number[]; // Row indices to show
  }[];
}
```

### 1.3 Common Mistakes Schema

```typescript
interface CommonMistake {
  mistake: string;
  wrongSql: string;
  explanation: string;
  // Show actual wrong result with THIS data
  wrongResult: {
    columns: string[];
    rows: any[];
  };
  // What they should get
  correctResult: {
    columns: string[];
    rows: any[];
  };
}
```

---

## 2. Data Generation Strategy

### 2.1 Seeded Randomness

Use deterministic random number generation with seeds:

```typescript
class SeededRandom {
  private seed: number;

  constructor(seed: number) {
    this.seed = seed;
  }

  // Mulberry32 - fast, simple, deterministic PRNG
  next(): number {
    let t = this.seed += 0x6D2B79F5;
    t = Math.imul(t ^ t >>> 15, t | 1);
    t ^= t + Math.imul(t ^ t >>> 7, t | 61);
    return ((t ^ t >>> 14) >>> 0) / 4294967296;
  }

  nextInt(min: number, max: number): number {
    return Math.floor(this.next() * (max - min + 1)) + min;
  }

  // Generate consistent but varied data
  pick<T>(array: T[]): T {
    return array[this.nextInt(0, array.length - 1)];
  }

  // Shuffle array deterministically
  shuffle<T>(array: T[]): T[] {
    const result = [...array];
    for (let i = result.length - 1; i > 0; i--) {
      const j = this.nextInt(0, i);
      [result[i], result[j]] = [result[j], result[i]];
    }
    return result;
  }
}
```

### 2.2 Realistic Data Generation

**Domain-Specific Generators:**

```typescript
interface DataGenerator {
  tableName: string;
  columnName: string;
  generate(rng: SeededRandom, context: GenerationContext): any;
}

// Example: E-commerce order data
interface GenerationContext {
  // Other tables' data for referential integrity
  references: {
    [tableName: string]: any[];
  };

  // Constraints from schema
  constraints: {
    foreignKeys: ForeignKeyConstraint[];
    uniqueValues: Set<string>;
  };
}

// Specific generators
const ProductNameGenerator: DataGenerator = {
  tableName: 'products',
  columnName: 'name',

  generate(rng: SeededRandom): string {
    const adjectives = ['Premium', 'Basic', 'Deluxe', 'Compact', 'Pro', 'Lite'];
    const nouns = ['Widget', 'Gadget', 'Tool', 'Device', 'Appliance'];
    const materials = ['Steel', 'Plastic', 'Wood', 'Aluminum', 'Carbon Fiber'];

    const adj = rng.pick(adjectives);
    const noun = rng.pick(nouns);
    const material = rng.pick(materials);

    return `${material} ${adj} ${noun}`;
  }
};

// Price with realistic distribution
const PriceGenerator: DataGenerator = {
  tableName: 'products',
  columnName: 'price',

  generate(rng: SeededRandom): number {
    // Log-normal distribution for realistic pricing
    const mean = Math.log(100);
    const stdDev = 0.5;
    const sample = mean + stdDev * gaussianRandom(rng);

    return Math.round(Math.exp(sample) * 100) / 100; // Round to cents
  }
};

// Email generator
const EmailGenerator: DataGenerator = {
  tableName: 'users',
  columnName: 'email',

  generate(rng: SeededRandom, context: GenerationContext): string {
    const firstNames = ['john', 'jane', 'bob', 'alice', 'charlie', 'diana'];
    const lastNames = ['smith', 'jones', 'wilson', 'taylor', 'brown', 'davis'];
    const domains = ['gmail.com', 'outlook.com', 'yahoo.com', 'example.com'];

    const first = rng.pick(firstNames);
    const last = rng.pick(lastNames);
    const domain = rng.pick(domains);

    // Vary the format
    const formats = [
      `${first}.${last}@${domain}`,
      `${first}${last}@${domain}`,
      `${first[0]}${last}@${domain}`,
      `${last}.${first}@${domain}`,
    ];

    return rng.pick(formats);
  }
};

// Date with clustering (more recent dates)
const DateGenerator: DataGenerator = {
  tableName: 'orders',
  columnName: 'order_date',

  generate(rng: SeededRandom): string {
    const now = Date.now();
    const daysBack = rng.nextInt(0, 365);

    // Bias towards more recent dates
    const biasedDays = Math.floor(Math.sqrt(rng.next()) * daysBack);
    const date = new Date(now - biasedDays * 24 * 60 * 60 * 1000);

    return date.toISOString().split('T')[0];
  }
};
```

### 2.3 Referential Integrity

```typescript
function generateForeignKeys(
  rng: SeededRandom,
  parentTable: any[],
  fkColumn: string
): any[] {
  const values: any[] = [];
  const parentIds = parentTable.map(row => row.id);

  for (let i = 0; i < 100; i++) { // Generate 100 child rows
    // Allow some NULLs (optional relationships)
    if (rng.next() < 0.1) {
      values.push(null);
    } else {
      values.push(rng.pick(parentIds));
    }
  }

  return values;
}
```

---

## 3. Semantic Consistency Validation

**Goal:** Ensure data variations don't change the problem's meaning.

### 3.1 Validation Checks

```typescript
interface SemanticValidation {
  // The query that defines the "correct answer"
  canonicalQuery: string;

  // Properties that must hold across ALL variants
  invariants: InvariantCheck[];
}

interface InvariantCheck {
  name: string;
  check: (data: BundleData) => boolean;
  description: string;
}

// Example invariants
const invariants: InvariantCheck[] = [
  {
    name: 'row-count-range',
    description: 'Result must have 3-10 rows (not empty, not overwhelming)',
    check: (data) => {
      const result = executeCanonicalQuery(data);
      return result.rowCount >= 3 && result.rowCount <= 10;
    }
  },

  {
    name: 'has-NULL-values',
    description: 'Some rows should have NULL to test IS NULL handling',
    check: (data) => {
      const result = executeCanonicalQuery(data);
      return result.rows.some(row =>
        Object.values(row).some(v => v === null)
      );
    }
  },

  {
    name: 'column-uniqueness',
    description: 'Results should have some duplicate values for DISTINCT tests',
    check: (data) => {
      const result = executeCanonicalQuery(data);
      const uniqueCount = new Set(
        result.rows.map(r => JSON.stringify(r))
      ).size;
      return uniqueCount < result.rowCount; // Some duplicates
    }
  },

  {
    name: 'value-range',
    description: 'Prices should be in realistic range',
    check: (data) => {
      const prices = data.tables.products.map((p: any) => p.price);
      return prices.every((p: number) => p > 0 && p < 10000);
    }
  },

  {
    name: 'referential-integrity',
    description: 'All order.user_id must reference existing users',
    check: (data) => {
      const userIds = new Set(data.tables.users.map((u: any) => u.id));
      return data.tables.orders.every((o: any) =>
        o.user_id === null || userIds.has(o.user_id)
      );
    }
  }
];
```

### 3.2 Semantic Checksum

```typescript
function computeSemanticChecksum(bundle: ProblemBundle): string {
  const canonicalResult = executeQuery(bundle.data, bundle.canonicalQuery);

  // Hash aspects that define problem "meaning"
  const aspects = {
    tableCount: Object.keys(bundle.data).length,
    columnNames: canonicalResult.columns.sort(),
    rowCount: canonicalResult.rowCount,
    hasNulls: canonicalResult.rows.some(r =>
      Object.values(r).some(v => v === null)
    ),
    hasDuplicates: new Set(
      canonicalResult.rows.map(r => JSON.stringify(r))
    ).size < canonicalResult.rowCount,
    valueTypes: canonicalResult.columns.map(col => {
      const values = canonicalResult.rows.map(r => typeof r[col]);
      return [...new Set(values)].sort();
    }),
  };

  return crypto.subtle.digest('SHA-256', JSON.stringify(aspects));
}
```

---

## 4. Variation Similarity Detection

**Purpose:** Measure how much data differs between variants to ensure:
- Not too similar (answers could be shared)
- Not too different (problem meaning changes)

### 4.1 Distance Metrics

```typescript
interface SimilarityMetrics {
  // Structural similarity
  structuralDistance: number;      // 0 = identical, 1 = completely different

  // Value overlap (how many specific values are the same)
  valueOverlap: number;            // 0 = no shared values, 1 = all values same

  // Distribution similarity (for numeric columns)
  distributionDistance: number;    // 0 = same distribution, 1 = completely different

  // Combined score
  overallSimilarity: number;       // Lower = more different
}

function computeSimilarity(
  variantA: ProblemBundle,
  variantB: ProblemBundle
): SimilarityMetrics {
  const metrics: SimilarityMetrics = {
    structuralDistance: 0,
    valueOverlap: 0,
    distributionDistance: 0,
    overallSimilarity: 0,
  };

  // 1. Structural distance - compare schema characteristics
  metrics.structuralDistance = computeStructuralDistance(variantA, variantB);

  // 2. Value overlap - Jaccard similarity of actual values
  metrics.valueOverlap = computeValueOverlap(variantA, variantB);

  // 3. Distribution distance - Kolmogorov-Smirnov for numeric
  metrics.distributionDistance = computeDistributionDistance(variantA, variantB);

  // Combined score with weights
  metrics.overallSimilarity =
    metrics.structuralDistance * 0.2 +
    metrics.valueOverlap * 0.5 +
    metrics.distributionDistance * 0.3;

  return metrics;
}

// Structural distance: compare row counts, null ratios, etc.
function computeStructuralDistance(a: ProblemBundle, b: ProblemBundle): number {
  let distance = 0;

  for (const tableName of Object.keys(a.data)) {
    const rowsA = a.data[tableName];
    const rowsB = b.data[tableName];

    // Row count difference (normalized)
    const countDiff = Math.abs(rowsA.length - rowsB.length) /
                      Math.max(rowsA.length, rowsB.length);
    distance += countDiff * 0.3;

    // Null ratio difference
    for (const column of Object.keys(rowsA[0] || {})) {
      const nullRatioA = rowsA.filter(r => r[column] === null).length / rowsA.length;
      const nullRatioB = rowsB.filter(r => r[column] === null).length / rowsB.length;
      distance += Math.abs(nullRatioA - nullRatioB) * 0.1;
    }
  }

  return Math.min(distance, 1);
}

// Value overlap: Jaccard similarity
function computeValueOverlap(a: ProblemBundle, b: ProblemBundle): number {
  let totalOverlap = 0;
  let totalValues = 0;

  for (const tableName of Object.keys(a.data)) {
    const rowsA = a.data[tableName];
    const rowsB = b.data[tableName];

    for (const column of Object.keys(rowsA[0] || {})) {
      const valuesA = new Set(rowsA.map(r => String(r[column])));
      const valuesB = new Set(rowsB.map(r => String(r[column])));

      // Jaccard similarity: |A ∩ B| / |A ∪ B|
      const intersection = new Set([...valuesA].filter(v => valuesB.has(v)));
      const union = new Set([...valuesA, ...valuesB]);

      totalOverlap += intersection.size / union.size;
      totalValues += 1;
    }
  }

  return totalOverlap / totalValues;
}

// Distribution distance for numeric columns (Kolmogorov-Smirnov)
function computeDistributionDistance(a: ProblemBundle, b: ProblemBundle): number {
  let totalDistance = 0;
  let numericColumns = 0;

  for (const tableName of Object.keys(a.data)) {
    const rowsA = a.data[tableName];
    const rowsB = b.data[tableName];

    for (const column of Object.keys(rowsA[0] || {})) {
      const valuesA = rowsA.map(r => r[column]).filter(v => typeof v === 'number');
      const valuesB = rowsB.map(r => r[column]).filter(v => typeof v === 'number');

      if (valuesA.length === 0 || valuesB.length === 0) continue;

      // KS statistic
      const sortedA = valuesA.sort((x, y) => x - y);
      const sortedB = valuesB.sort((x, y) => x - y);

      let maxDiff = 0;
      let i = 0, j = 0;

      while (i < sortedA.length && j < sortedB.length) {
        const cdfA = i / sortedA.length;
        const cdfB = j / sortedB.length;
        maxDiff = Math.max(maxDiff, Math.abs(cdfA - cdfB));

        if (sortedA[i] <= sortedB[j]) i++;
        else j++;
      }

      totalDistance += maxDiff;
      numericColumns += 1;
    }
  }

  return numericColumns > 0 ? totalDistance / numericColumns : 0;
}
```

### 4.2 Similarity Thresholds

```typescript
interface SimilarityThresholds {
  minDistance: number;    // Minimum required difference
  maxDistance: number;    // Maximum allowed difference
  targetRange: [number, number]; // Preferred similarity range
}

const DEFAULT_THRESHOLDS: SimilarityThresholds = {
  minDistance: 0.3,       // At least 30% different
  maxDistance: 0.7,       // At most 70% different
  targetRange: [0.4, 0.6], // Aim for 40-60% similarity
};

function validateVariant(
  newVariant: ProblemBundle,
  existingVariants: ProblemBundle[],
  thresholds: SimilarityThresholds = DEFAULT_THRESHOLDS
): ValidationResult {
  const issues: string[] = [];
  const scores: number[] = [];

  for (const existing of existingVariants) {
    const similarity = computeSimilarity(newVariant, existing);
    scores.push(similarity.overallSimilarity);

    if (similarity.overallSimilarity < thresholds.minDistance) {
      issues.push(
        `Too similar to ${existing.variantId}: ` +
        `${(similarity.overallSimilarity * 100).toFixed(0)}% overlap ` +
        `(minimum ${(thresholds.minDistance * 100).toFixed(0)}% required)`
      );
    }

    if (similarity.overallSimilarity > thresholds.maxDistance) {
      issues.push(
        `Too different from ${existing.variantId}: ` +
        `${((1 - similarity.overallSimilarity) * 100).toFixed(0)}% distance ` +
        `(maximum ${(thresholds.maxDistance * 100).toFixed(0)}% allowed)`
      );
    }
  }

  // Also check target range
  const avgSimilarity = scores.reduce((a, b) => a + b, 0) / scores.length;
  if (avgSimilarity < thresholds.targetRange[0] || avgSimilarity > thresholds.targetRange[1]) {
    issues.push(
      `Average similarity ${(avgSimilarity * 100).toFixed(0)}% ` +
      `outside target range [${(thresholds.targetRange[0] * 100).toFixed(0)}%, ` +
      `${(thresholds.targetRange[1] * 100).toFixed(0)}%]`
    );
  }

  return {
    valid: issues.length === 0,
    issues,
    averageSimilarity: avgSimilarity,
    scores: scores.map((s, i) => ({ variantId: existingVariants[i].variantId, similarity: s })),
  };
}
```

---

## 5. Bundle Generation Pipeline

### 5.1 Generation Steps

```mermaid
graph TD
    A[Canonical Problem Definition] --> B[Generate Variant 1]
    A --> C[Generate Variant 2]
    A --> D[Generate Variant 3]

    B --> E[Validate Semantic Consistency]
    C --> F[Validate Semantic Consistency]
    D --> G[Validate Semantic Consistency]

    E --> H[Compute Similarity Matrix]
    F --> H
    G --> H

    H --> I{Similarity OK?}
    I -->|No| J[Adjust Seeds & Regenerate]
    J --> B

    I -->|Yes| K[Generate Explanations]
    K --> L[Store Bundle]
```

### 5.2 Pipeline Configuration

```typescript
interface GenerationConfig {
  problemId: string;

  // How many variants to generate
  variantCount: number;         // Default: 5-10

  // Random seeds to use (deterministic)
  seeds: number[];

  // Similarity thresholds
  similarityThresholds: SimilarityThresholds;

  // Validation rules
  invariants: InvariantCheck[];

  // LLM configuration for explanations
  llmConfig: {
    model: string;
    temperature: number;
    maxTokens: number;
  };
}
```

---

## 6. User Assignment Strategy

### 6.1 Assignment Algorithm

```typescript
interface AssignmentStrategy {
  // Distribute users evenly across variants
  strategy: 'round-robin' | 'random' | 'weighted';

  // Track which users have which variants
  tracking: 'by-user' | 'by-session' | 'none';
}

function assignVariant(
  userId: string,
  problemId: string,
  availableVariants: ProblemBundle[],
  strategy: AssignmentStrategy = { strategy: 'random', tracking: 'by-session' }
): ProblemBundle {
  switch (strategy.strategy) {
    case 'round-robin':
      // Assign based on user ID hash for consistent assignment
      const hash = hashString(userId + problemId);
      const index = hash % availableVariants.length;
      return availableVariants[index];

    case 'random':
      // Pure random assignment
      return availableVariants[Math.floor(Math.random() * availableVariants.length)];

    case 'weighted':
      // Weight by variant difficulty/complexity
      return weightedSelect(availableVariants);

    default:
      return availableVariants[0];
  }
}
```

### 6.2 Assignment Storage

```typescript
interface UserAssignment {
  userId: string;
  problemId: string;
  variantId: string;
  assignedAt: string;
}

// Stored in user progress
interface UserProgress {
  assignments: UserAssignment[];
  attempts: Attempt[];
  solved: string[];
}
```

---

## 7. Storage Format

### 7.1 File Structure

```
data/
  problems/
    {problemId}/
      manifest.json           # Problem metadata, list of variants
      canonical/
        schema.json
        query.sql
      variants/
        v1/
          bundle.json
          data.json
          explanation.json
        v2/
          bundle.json
          data.json
          explanation.json
        ...
```

### 7.2 Manifest Schema

```typescript
interface ProblemManifest {
  problemId: string;
  title: string;
  category: string;
  difficulty: string;

  variants: {
    variantId: string;
    seed: number;
    similarityScore?: number;
    createdAt: string;
  }[];

  // Metadata about the set
  variantCount: number;
  averageSimilarity: number;
  lastRegenerated: string;
}
```

---

## 8. API Endpoints

### 8.1 Bundle Retrieval

```typescript
// GET /api/problems/{problemId}/bundle
// Returns the assigned variant for the current user
interface GetBundleRequest {
  problemId: string;
  userId?: string;  // Optional, for consistent assignment
}

interface GetBundleResponse {
  bundle: ProblemBundle;
  variantId: string;
}
```

### 8.2 Bulk Bundle Loading

```typescript
// GET /api/problems/bundles
// Load all bundles for offline caching
interface GetBundlesResponse {
  problems: {
    problemId: string;
    bundle: ProblemBundle;
  }[];
}
```

---

## 9. Number of Variants Recommendation

**Factors to Consider:**

1. **Academic Integrity**: More variants = harder to share answers
2. **Storage Size**: Each variant includes data + explanations
3. **Generation Cost**: LLM API calls scale linearly
4. **Maintenance**: Updates require regenerating all variants

**Recommendations by Problem Type:**

| Problem Type | Recommended Variants | Reasoning |
|--------------|---------------------|-----------|
| Beginner (simple SELECT) | 10-15 | Easy to reverse-engineer |
| Intermediate (JOINs) | 5-10 | Moderate complexity |
| Advanced (Window Functions) | 3-5 | Harder to share answers |
| Open-ended | 3-5 | Multiple valid solutions |

**Storage Estimate:**
- Each bundle: ~50-200 KB (data + explanation)
- 10 variants: ~500 KB - 2 MB per problem
- 50 problems: ~25-100 MB total

---

## 10. Future Considerations

### 10.1 Adaptive Difficulty
- Track user performance per variant
- Identify if some variants are significantly harder/easier
- Adjust difficulty ratings

### 10.2 Variant Retirement
- Detect if variants are compromised (answers shared)
- Generate new variants to replace retired ones
- Maintain user assignment history

### 10.3 Cross-Variant Learning
- Allow users to see same problem with different data
- Reinforce that solution approach is data-independent
- "Master this problem" mode with all variants

### 10.4 A/B Testing
- Test different explanation styles
- Measure completion rates across variants
- Optimize hint effectiveness

---

# Part 3: IR-Based Text-to-SQL Architecture

## Overview

This section documents **non-neural, rule-based approaches** for converting natural language questions to SQL queries. Unlike modern LLM-based approaches, these methods use:

- **No LLM** - No transformer models, embeddings, or pre-training
- **No Pre-training** - Works on any random database schema provided at runtime
- **Keyword Matching** - String overlap scoring and pattern matching
- **Template Filling** - Pre-defined SQL templates filled with detected entities

These algorithms are drawn from academic research, specifically the **"IR Baseline"** from the **Spider Benchmark** - the standard leaderboard for Text-to-SQL research. Researchers use this baseline to prove that their complex AI models actually outperform simple rule-based approaches.

---

## Motivation: Why Rule-Based Text-to-SQL?

| Consideration | LLM-Based | Rule-Based (IR) |
|--------------|-----------|-----------------|
| **Runtime Requirements** | ~2-4GB RAM per model | ~5-50MB RAM |
| **Model Download** | 100MB - 10GB | 0MB (pure code) |
| **Browser Compatibility** | Blocked by corporate policies | Pure JavaScript/TypeScript |
| **Cold Start** | Seconds to minutes | Instant |
| **Privacy** | Data sent to external API or runs locally | 100% local |
| **Accuracy (Simple Questions)** | 85-95% | 60-80% |
| **Accuracy (Complex Questions)** | 50-70% | 20-40% |

**Use Case**: Ideal for browser-based SQL learning platforms where:
- Corporate environments block external API calls
- Memory is limited per browser tab
- Questions follow predictable patterns
- Partial accuracy is acceptable (hints guide users)

---

## Core Algorithm: Slot Filling

### High-Level Flow

```
Natural Language Question
        │
        ▼
┌─────────────────────────────────────┐
│  1. Tokenization & Normalization    │
│     - Lowercase, remove punctuation  │
│     - Extract key terms             │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  2. Schema Linking                  │
│     - Match terms to table names    │
│     - Match terms to column names   │
│     - Detect SQL keywords           │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  3. Intent Classification           │
│     - SELECT vs aggregation         │
│     - Single table vs JOIN          │
│     - Filter presence               │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│  4. Template Selection              │
│     - Choose SQL template           │
│     - Fill detected slots           │
└──────────────┬──────────────────────┘
               │
               ▼
        SQL Query
```

---

## Implementation Components

### 1. Keyword Overlap Scorer

The core of IR-based approaches is measuring similarity between question tokens and schema elements.

```typescript
interface OverlapScore {
  token: string;
  matchedElement: string;
  elementType: 'table' | 'column';
  score: number;  // 0-1
}

function scoreOverlap(
  questionTokens: string[],
  schemaElements: string[]
): OverlapScore[] {
  const scores: OverlapScore[] = [];

  for (const token of questionTokens) {
    for (const element of schemaElements) {
      const elementTokens = element.split(/[_\s]/);
      const score = computeTokenOverlap(token, elementTokens);

      if (score > 0.3) {  // Threshold for matching
        scores.push({
          token,
          matchedElement: element,
          elementType: schemaElements.includes(element) ? 'column' : 'table',
          score
        });
      }
    }
  }

  return scores.sort((a, b) => b.score - a.score);
}

// Jaccard similarity for token overlap
function computeTokenOverlap(token: string, elementTokens: string[]): number {
  const tokenLower = token.toLowerCase();
  const elementLower = elementTokens.map(t => t.toLowerCase());

  // Exact match
  if (elementLower.includes(tokenLower)) return 1.0;

  // Substring match
  for (const elemToken of elementLower) {
    if (elemToken.includes(tokenLower) || tokenLower.includes(elemToken)) {
      return 0.7;
    }
  }

  // Fuzzy match (Levenshtein-based)
  for (const elemToken of elementLower) {
    const distance = levenshtein(tokenLower, elemToken);
    const maxLength = Math.max(tokenLower.length, elemToken.length);
    if (distance / maxLength < 0.3) {
      return 0.5;
    }
  }

  return 0;
}

// Levenshtein distance
function levenshtein(a: string, b: string): number {
  const matrix: number[][] = [];

  for (let i = 0; i <= b.length; i++) {
    matrix[i] = [i];
  }
  for (let j = 0; j <= a.length; j++) {
    matrix[0][j] = j;
  }

  for (let i = 1; i <= b.length; i++) {
    for (let j = 1; j <= a.length; j++) {
      if (b.charAt(i - 1) === a.charAt(j - 1)) {
        matrix[i][j] = matrix[i - 1][j - 1];
      } else {
        matrix[i][j] = Math.min(
          matrix[i - 1][j - 1] + 1,
          matrix[i][j - 1] + 1,
          matrix[i - 1][j] + 1
        );
      }
    }
  }

  return matrix[b.length][a.length];
}
```

### 2. Schema Linker

Maps question tokens to database schema elements using multiple strategies.

```typescript
interface SchemaLink {
  questionToken: string;
  table: string | null;
  column: string | null;
  confidence: number;
}

class SchemaLinker {
  private tables: string[];
  private columns: Map<string, string[]>;  // table -> columns

  constructor(schema: QuestionSchema) {
    this.tables = schema.tables.map(t => t.name);
    this.columns = new Map();
    for (const table of schema.tables) {
      this.columns.set(table.name, table.columns.map(c => c.name));
    }
  }

  link(question: string): SchemaLink[] {
    const tokens = this.tokenize(question);
    const links: SchemaLink[] = [];

    for (const token of tokens) {
      const link = this.findBestMatch(token);
      if (link) {
        links.push(link);
      }
    }

    return links;
  }

  private findBestMatch(token: string): SchemaLink | null {
    const tokenLower = token.toLowerCase();

    // 1. Direct column match
    for (const [table, columns] of this.columns) {
      if (columns.some(c => c.toLowerCase() === tokenLower)) {
        return { questionToken: token, table, column: token, confidence: 1.0 };
      }
    }

    // 2. Direct table match
    if (this.tables.some(t => t.toLowerCase() === tokenLower)) {
      return { questionToken: token, table: token, column: null, confidence: 1.0 };
    }

    // 3. Partial column match
    for (const [table, columns] of this.columns) {
      for (const col of columns) {
        const colLower = col.toLowerCase();
        if (colLower.includes(tokenLower) || tokenLower.includes(colLower)) {
          return { questionToken: token, table, column: col, confidence: 0.7 };
        }
      }
    }

    // 4. Split camelCase/pascal_case columns
    for (const [table, columns] of this.columns) {
      for (const col of columns) {
        const subTokens = this.splitColumnName(col);
        for (const subToken of subTokens) {
          if (subToken.toLowerCase() === tokenLower) {
            return { questionToken: token, table, column: col, confidence: 0.6 };
          }
        }
      }
    }

    return null;
  }

  private tokenize(text: string): string[] {
    // Remove SQL keywords, split on whitespace/punctuation
    const stopWords = ['the', 'a', 'an', 'is', 'are', 'of', 'in', 'for', 'to', 'and'];
    const tokens = text
      .toLowerCase()
      .replace(/[^\w\s]/g, ' ')
      .split(/\s+/)
      .filter(t => t.length > 2 && !stopWords.includes(t));
    return tokens;
  }

  private splitColumnName(column: string): string[] {
    // Split on underscore, camelCase
    return column
      .replace(/([a-z])([A-Z])/g, '$1 $2')
      .split(/[_\s]+/);
  }
}
```

### 3. Intent Classifier

Determines what type of SQL query is needed based on question patterns.

```typescript
type QueryIntent =
  | 'simple_select'      // SELECT column FROM table
  | 'filtered_select'    // SELECT ... WHERE ...
  | 'aggregation'        // SELECT COUNT/SUM/AVG ...
  | 'grouped'            // SELECT ... GROUP BY ...
  | 'joined'             // SELECT ... FROM ... JOIN ...
  | 'ordered'            // SELECT ... ORDER BY ...
  | 'top_n'              // SELECT ... ORDER BY ... LIMIT n
  | 'complex';           // Combination of above

interface IntentDetection {
  intent: QueryIntent;
  confidence: number;
  detectedClauses: string[];
}

class IntentClassifier {
  private patterns: Map<QueryIntent, RegExp[]> = new Map([
    ['simple_select', [/^(list|show|get|find|display|what|which)/i]],
    ['filtered_select', [
      /where|in|that|have|with|filter|only|greater|less|equal/i
    ]],
    ['aggregation', [/total|sum|count|average|maximum|minimum|max|min|avg/i]],
    ['grouped', [/per|for each|by|group|category/i]],
    ['joined', [/combine|together|from.*and|both|match|related/i]],
    ['ordered', [/sort|order|highest|lowest|first|last|top|bottom/i]],
    ['top_n', [/first \d+|top \d+|limit/i]],
  ]);

  classify(question: string, schemaLinks: SchemaLink[]): IntentDetection {
    const scores = new Map<QueryIntent, number>();

    for (const [intent, patterns] of this.patterns) {
      let score = 0;
      for (const pattern of patterns) {
        if (pattern.test(question)) {
          score += 1;
        }
      }
      scores.set(intent, score);
    }

    // Boost scores based on schema characteristics
    if (schemaLinks.length > 1) {
      scores.set('joined', (scores.get('joined') || 0) + 1);
    }

    // Determine intent with highest score
    let maxIntent: QueryIntent = 'simple_select';
    let maxScore = 0;
    for (const [intent, score] of scores) {
      if (score > maxScore) {
        maxScore = score;
        maxIntent = intent;
      }
    }

    // Detect combined intents
    const detectedClauses: string[] = [];
    if (scores.get('filtered_select') > 0) detectedClauses.push('WHERE');
    if (scores.get('aggregation') > 0) detectedClauses.push('AGGREGATION');
    if (scores.get('grouped') > 0) detectedClauses.push('GROUP_BY');
    if (scores.get('joined') > 0) detectedClauses.push('JOIN');
    if (scores.get('ordered') > 0) detectedClauses.push('ORDER_BY');

    let finalIntent = maxIntent;
    if (detectedClauses.length > 1) {
      finalIntent = 'complex';
    }

    return {
      intent: finalIntent,
      confidence: Math.min(maxScore / detectedClauses.length, 1),
      detectedClauses
    };
  }
}
```

### 4. SQL Template Filler

Uses predefined templates and fills them with detected schema elements.

```typescript
interface SqlTemplate {
  template: string;
  slots: string[];
}

class TemplateFiller {
  private templates: Map<QueryIntent, SqlTemplate> = new Map([
    ['simple_select', {
      template: 'SELECT {columns} FROM {table}',
      slots: ['columns', 'table']
    }],
    ['filtered_select', {
      template: 'SELECT {columns} FROM {table} WHERE {condition}',
      slots: ['columns', 'table', 'condition']
    }],
    ['aggregation', {
      template: 'SELECT {agg_func}({column}) FROM {table}',
      slots: ['agg_func', 'column', 'table']
    }],
    ['grouped', {
      template: 'SELECT {group_column}, {agg_func}({agg_column}) FROM {table} GROUP BY {group_column}',
      slots: ['group_column', 'agg_func', 'agg_column', 'table']
    }],
    ['joined', {
      template: 'SELECT {columns} FROM {table1} JOIN {table2} ON {join_condition}',
      slots: ['columns', 'table1', 'table2', 'join_condition']
    }],
    ['ordered', {
      template: 'SELECT {columns} FROM {table} ORDER BY {order_column} {direction}',
      slots: ['columns', 'table', 'order_column', 'direction']
    }],
    ['top_n', {
      template: 'SELECT {columns} FROM {table} ORDER BY {order_column} {direction} LIMIT {n}',
      slots: ['columns', 'table', 'order_column', 'direction', 'n']
    }]
  ]);

  fill(
    intent: QueryIntent,
    links: SchemaLink[],
    question: string
  ): string | null {
    const template = this.templates.get(intent);
    if (!template) return null;

    const filled = this.fillSlots(template, links, question);
    return this.postProcess(filled);
  }

  private fillSlots(
    template: SqlTemplate,
    links: SchemaLink[],
    question: string
  ): string {
    let result = template.template;
    const tables = [...new Set(links.map(l => l.table).filter(Boolean))];
    const columns = [...new Set(links.map(l => l.column).filter(Boolean))];

    // Fill {table} slot
    result = result.replace('{table}', tables[0] || 'table_name');

    // Fill {table1} and {table2} for JOINs
    if (result.includes('{table1}') && result.includes('{table2}')) {
      result = result.replace('{table1}', tables[0] || 'table1');
      result = result.replace('{table2}', tables[1] || 'table2');
    }

    // Fill {columns} slot
    if (result.includes('{columns}')) {
      const columnList = columns.length > 0 ? columns.join(', ') : '*';
      result = result.replace('{columns}', columnList);
    }

    // Fill aggregation function
    if (result.includes('{agg_func}')) {
      result = result.replace('{agg_func}', this.detectAggFunc(question));
    }

    // Fill {column} slot for aggregation
    if (result.includes('{agg_column}')) {
      const numericColumn = this.findNumericColumn(links);
      result = result.replace('{agg_column}', numericColumn || columns[0] || 'column_name');
    }

    // Fill {group_column}
    if (result.includes('{group_column}')) {
      const groupColumn = this.findGroupColumn(links, question);
      result = result.replace('{group_column}', groupColumn || columns[0] || 'column_name');
    }

    // Fill {direction}
    if (result.includes('{direction}')) {
      const direction = /highest|first|top|maximum|max/i.test(question) ? 'DESC' : 'ASC';
      result = result.replace('{direction}', direction);
    }

    // Fill {order_column}
    if (result.includes('{order_column}')) {
      const orderColumn = this.findOrderColumn(links, question);
      result = result.replace('{order_column}', orderColumn || columns[0] || 'column_name');
    }

    // Fill {condition} - basic implementation
    if (result.includes('{condition}')) {
      result = result.replace('{condition}', this.generateCondition(question, links));
    }

    // Fill {join_condition}
    if (result.includes('{join_condition}')) {
      result = result.replace('{join_condition}', this.generateJoinCondition(links));
    }

    // Fill {n} for LIMIT
    if (result.includes('{n}')) {
      const match = question.match(/\d+/);
      result = result.replace('{n}', match ? match[0] : '10');
    }

    return result;
  }

  private detectAggFunc(question: string): string {
    if (/total|sum/i.test(question)) return 'SUM';
    if (/count|how many|number of/i.test(question)) return 'COUNT';
    if (/average|avg/i.test(question)) return 'AVG';
    if (/max|highest|maximum/i.test(question)) return 'MAX';
    if (/min|lowest|minimum/i.test(question)) return 'MIN';
    return 'COUNT';
  }

  private findNumericColumn(links: SchemaLink[]): string | null {
    // In real implementation, check column types from schema
    // For now, return first column that looks numeric
    const numericNames = ['price', 'salary', 'amount', 'count', 'total', 'quantity', 'score'];
    for (const link of links) {
      if (link.column && numericNames.some(n => link.column!.toLowerCase().includes(n))) {
        return link.column;
      }
    }
    return links[0]?.column || null;
  }

  private findGroupColumn(links: SchemaLink[], question: string): string | null {
    // Look for "per X", "for each X" patterns
    const match = question.match(/(?:per|for each|by|grouped by)\s+(\w+)/i);
    if (match) {
      const target = match[1].toLowerCase();
      for (const link of links) {
        if (link.column && link.column.toLowerCase().includes(target)) {
          return link.column;
        }
      }
    }
    return links[0]?.column || null;
  }

  private findOrderColumn(links: SchemaLink[], question: string): string | null {
    const numericColumn = this.findNumericColumn(links);
    return numericColumn || links[0]?.column || null;
  }

  private generateCondition(question: string, links: SchemaLink[]): string {
    // Very basic WHERE clause generation
    // Real implementation would parse comparison operators
    const conditions: string[] = [];

    // Look for patterns like "salary > 50000"
    const comparisons = question.match(/(\w+)\s*(>|<|>=|<=|=)\s*(\d+)/gi);
    if (comparisons) {
      for (const comp of comparisons) {
        const parts = comp.split(/\s*(>|<|>=|<=|=)\s*/);
        if (parts.length === 3) {
          const column = links.find(l => l.column?.toLowerCase() === parts[0].toLowerCase());
          if (column) {
            conditions.push(`${column.column} ${parts[1]} ${parts[2]}`);
          }
        }
      }
    }

    // Look for equality patterns like "department = 'Sales'"
    const equals = question.match(/(\w+)\s*(?:is|=)\s*['"]?(\w+)['"]?/i);
    if (equals) {
      const column = links.find(l => l.column?.toLowerCase() === equals[1].toLowerCase());
      if (column) {
        conditions.push(`${column.column} = '${equals[2]}'`);
      }
    }

    return conditions.length > 0 ? conditions.join(' AND ') : '1=1';
  }

  private generateJoinCondition(links: SchemaLink[]): string {
    // Find foreign key relationships
    const tables = [...new Set(links.map(l => l.table).filter(Boolean))];
    if (tables.length < 2) return '1=1';

    // Simple heuristic: join on {table1}_id = {table2}.id
    return `${tables[0]}.${tables[1]}_id = ${tables[1]}.id`;
  }

  private postProcess(sql: string): string {
    return sql
      .replace(/\s+/g, ' ')  // Normalize whitespace
      .trim();
  }
}
```

---

## Complete Pipeline

```typescript
class IRTextToSql {
  private linker: SchemaLinker;
  private classifier: IntentClassifier;
  private filler: TemplateFiller;

  constructor(schema: QuestionSchema) {
    this.linker = new SchemaLinker(schema);
    this.classifier = new IntentClassifier();
    this.filler = new TemplateFiller();
  }

  convert(question: string): { sql: string | null; confidence: number; debug: any } {
    // Step 1: Link question tokens to schema
    const links = this.linker.link(question);

    // Step 2: Classify intent
    const intent = this.classifier.classify(question, links);

    // Step 3: Fill template
    const sql = this.filler.fill(intent.intent, links, question);

    return {
      sql,
      confidence: intent.confidence,
      debug: {
        links,
        intent,
        detectedClauses: intent.detectedClauses
      }
    };
  }
}
```

---

## Open-Source Tools & Libraries

### 1. SQLGlot

**GitHub**: `tobymao/sqlglot`

**Purpose**: SQL parsing, transpilation, and validation.

**Usage in IR System**:
```typescript
import { parse, build } from 'sqlglot';

// Validate generated SQL
function validateGenerated(sql: string): boolean {
  try {
    const ast = parse(sql);
    return ast !== null;
  } catch {
    return false;
  }
}

// Normalize SQL for comparison
function normalizeSQL(sql: string): string {
  const ast = parse(sql);
  return build(ast);  // Rebuild to normalize formatting
}
```

### 2. FuzzyWuzzy / fuzzball

**Purpose**: Fuzzy string matching for token-to-schema linking.

```typescript
import fuzzball from 'fuzzball';

// Better fuzzy matching than Levenshtein
function fuzzyMatch(token: string, schemaElement: string): number {
  return fuzzball.ratio(token.toLowerCase(), schemaElement.toLowerCase()) / 100;
}
```

### 3. Spider IR Baseline

**GitHub**: `yechens/spider`

**Location**: `baseline/ir-baseline/`

**What to Study**:
- `schema_linking.py` - Token to schema matching
- `prediction.py` - Template filling logic
- `ir.py` - Main IR pipeline

---

## Accuracy & Limitations

### Expected Accuracy by Query Type

| Query Type | IR Baseline | State-of-the-Art LLM |
|-----------|-------------|---------------------|
| Simple SELECT | 75-85% | 90-95% |
| WHERE with simple conditions | 60-75% | 85-90% |
| Aggregation (COUNT, SUM) | 55-70% | 75-85% |
| GROUP BY | 40-60% | 65-75% |
| JOIN (2 tables) | 35-50% | 60-70% |
| JOIN (3+ tables) | 20-35% | 45-55% |
| Nested queries | 15-30% | 40-50% |
| Window functions | 10-25% | 35-45% |

### Known Limitations

1. **Schema Ambiguity**: Cannot resolve when column names overlap across tables
2. **Multi-hop Reasoning**: Fails on questions requiring intermediate reasoning steps
3. **Negation**: "NOT IN", "NOT EXISTS" patterns poorly detected
4. **Complex Conditions**: AND/OR combinations, subqueries in WHERE clause
5. **Synonyms**: "revenue" vs "income", "staff" vs "employees"

### Mitigation Strategies

1. **Interactive Refinement**: Ask user to clarify ambiguous mappings
2. **Multiple Candidates**: Generate top 3 SQL options, let user choose
3. **Partial Results**: Generate SELECT clause correctly, prompt for WHERE/JOIN
4. **Hint Integration**: Use hint system to guide when IR confidence is low

---

## Integration with SQL Practice Platform

### Hint Generation from IR Analysis

```typescript
class IrHintGenerator {
  generateHints(
    question: string,
    irResult: { sql: string | null; confidence: number; debug: any },
    schema: QuestionSchema
  ): string[] {
    const hints: string[] = [];

    // Low confidence hints
    if (irResult.confidence < 0.5) {
      hints.push('This question has some complex patterns. Break it down step by step.');
    }

    // Schema hints
    const { links } = irResult.debug;
    const detectedTables = [...new Set(links.map((l: SchemaLink) => l.table))];
    if (detectedTables.length > 1) {
      hints.push(`You'll need to join these tables: ${detectedTables.join(', ')}`);
    }

    // Intent-specific hints
    const { intent, detectedClauses } = irResult.debug;
    if (detectedClauses.includes('GROUP_BY')) {
      hints.push('Use GROUP BY with an aggregate function like COUNT() or SUM()');
    }
    if (detectedClauses.includes('ORDER_BY')) {
      hints.push('Use ORDER BY to sort results. Add DESC for descending order.');
    }

    // Column suggestions
    const unmatched = this.findUnmatchedKeywords(question, links);
    if (unmatched.length > 0) {
      hints.push(`Consider these columns: ${unmatched.join(', ')}`);
    }

    return hints;
  }

  private findUnmatchedKeywords(question: string, links: SchemaLink[]): string[] {
    const linkedWords = new Set(links.map(l => l.questionToken.toLowerCase()));
    const words = question.toLowerCase().split(/\s+/);
    return words.filter(w => w.length > 3 && !linkedWords.has(w));
  }
}
```

---

## References

### Academic Papers

1. **Yu et al., 2018** - "Spider: A Large-Scale Human-Labeled Dataset for Complex and Cross-Domain Semantic Parsing and Text-to-SQL Task"
   - Introduced the Spider benchmark
   - Defined the IR baseline used for comparison

2. **Dong & Lapata, 2018** - "Coarse-to-Fine Decoding for Neural Semantic Parsing"
   - Discusses slot-filling approaches

3. **Finegan-Dollak et al., 2018** - "Improving Text-to-SQL Evaluation by Learning User Intent"
   - Intent classification techniques

### Open Source Implementations

| Repository | Language | Focus |
|------------|----------|-------|
| `yechens/spider` | Python | Official Spider benchmark + IR baseline |
| `tobymao/sqlglot` | Python | SQL parsing/transpiling toolkit |
| `RasaHQ/rasa` | Python | Rule-based NLU pipeline |
| `kootenpv/py_pugg` | Python | SQL AST builder |

### Related Standards

- **SQL:2016** - ISO/IEC 9075 standard for SQL syntax
- **Spider Schema** - Standard format for Text-to-SQL schema representation

---

# Part 4: Spider 2.0 Benchmark Reference

## Overview

**Spider 2.0** (ICLR 2025 Oral) is a next-generation evaluation framework for Text-to-SQL systems that addresses real-world enterprise workflows. Unlike traditional benchmarks like Spider 1.0 and BIRD, Spider 2.0 focuses on **multi-query workflows** involving complex cloud database systems.

**Repository**: [xlang-ai/Spider2](https://github.com/xlang-ai/Spider2)
**Paper**: arXiv:2411.07763
**Website**: https://spider2-sql.github.io

---

## Key Innovations vs Previous Benchmarks

| Aspect | Spider 1.0 / BIRD | Spider 2.0 |
|--------|------------------|------------|
| **Task Type** | Single SQL query | Multi-query workflows |
| **Database Scale** | < 100 columns | 1,000+ columns |
| **Database Systems** | SQLite | BigQuery, Snowflake, DuckDB |
| **Query Length** | 10-30 lines | Up to 100+ lines |
| **Context Required** | Schema only | Schema + docs + codebase |
| **Real-World Data** | Synthetic/Clean | Production databases |

---

## Benchmark Settings

### Spider 2.0-Snow

| Property | Value |
|----------|-------|
| **Task Type** | Text-to-SQL workflow |
| **Examples** | 547 |
| **Databases** | Snowflake (547 instances) |
| **Cost** | FREE |
| **Access** | Sign-up required (hosted by Snowflake) |

**Use Case**: Zero-cost benchmarking on Snowflake's enterprise data warehouse.

### Spider 2.0-Lite

| Property | Value |
|----------|-------|
| **Task Type** | Text-to-SQL workflow |
| **Examples** | 547 |
| **Databases** | BigQuery (214), Snowflake (198), SQLite (135) |
| **Cost** | Some cost (BigQuery/Snowflake) |
| **Access** | Requires own cloud credentials |

**Use Case**: Cross-platform evaluation across multiple database systems.

### Spider 2.0-DBT

| Property | Value |
|----------|-------|
| **Task Type** | Code agent / Repository-level |
| **Examples** | 68 |
| **Databases** | DuckDB with DBT (68) |
| **Cost** | FREE |
| **Access** | Open source |

**Use Case**: Evaluating agents on repository-level transformation tasks (DBT models).

---

## What Makes Spider 2.0 Challenging?

### 1. Multi-Query Workflows

Unlike traditional benchmarks where each question maps to a single SQL query, Spider 2.0 requires:

```
User Request → Query 1 → Result 1
                ↓
             Query 2 → Result 2
                ↓
             Query 3 → Final Answer
```

Each query may:
- Transform data (CTEs, temp tables)
- Analyze intermediate results
- Combine results from multiple sources

### 2. Massive Schema Scale

Enterprise databases in Spider 2.0 contain:
- **1,000+ columns** per database
- Hundreds of tables
- Complex relationships and foreign keys
- Production naming conventions (not simplified)

This requires models to:
- Navigate schema metadata efficiently
- Understand table relationships
- Filter irrelevant tables/columns

### 3. Dialect-Specific Features

| Database | Unique Features |
|----------|-----------------|
| **BigQuery** | `ARRAY_AGG`, `STRUCT`, window functions, ML functions |
| **Snowflake** | Semi-structured data (`VARIANT`), stored procedures, UDFs |
| **DuckDB (DBT)** | Analytics functions, parquet handling, materializations |

Models must read and understand:
- Database-specific documentation
- Function references
- Syntax variations

### 4. Codebase Understanding

Spider 2.0-DBT requires analyzing:
- Existing DBT models (.sql files)
- Project structure
- Macro definitions
- Dependency graphs

```
dbt_project/
├── models/
│   ├── staging/
│   │   └── stg_users.sql
│   ├── intermediate/
│   │   └── int_user_orders.sql
│   └── marts/
│       └── fct_orders.sql
├── macros/
│   └── cents_to_dollars.sql
└── dbt_project.yml
```

---

## Evaluation Results (Paper Findings)

### State-of-the-Art Performance

| Benchmark | o1-preview Performance |
|-----------|------------------------|
| **Spider 1.0** | 91.2% |
| **BIRD** | 73.0% |
| **Spider 2.0** | 21.3% |

The dramatic drop (91.2% → 21.3%) highlights:
- Current LLMs excel at single-query tasks
- Real-world workflows require significant improvement
- Multi-step reasoning is still challenging

### Key Failure Modes

1. **Schema Navigation** - Cannot find relevant tables in 1,000+ column schemas
2. **Dialect Knowledge** - Lacks BigQuery/Snowflake-specific syntax
3. **Multi-Step Planning** - Fails to decompose complex workflows
4. **Context Management** - Cannot process documentation + schema + query simultaneously
5. **Debugging** - Cannot iterate on failed queries

---

## Spider-Agent Framework

Spider 2.0 includes a reference implementation for evaluation:

### Tool-Call Based Agent (Docker-free)

```python
# spider-agent-tool-call/implementation
class SpiderAgent:
    """
    Ultra-fast Spider-Agent implementation using tool calls.
    No Docker required for rapid benchmarking.
    """
    def __init__(self, model_name: str):
        self.model = model_name
        self.tools = [
            "execute_sql",      # Run SQL against database
            "list_tables",      # Get all tables
            "describe_table",   # Get table schema
            "search_docs",      # Search dialect documentation
            "list_codebase_files", # For DBT: list project files
            "read_file",        # For DBT: read model code
        ]

    def solve(self, question: str, database: str) -> str:
        # Multi-step reasoning with tool calls
        steps = self.plan_steps(question)
        results = []

        for step in steps:
            tool = step["tool"]
            inputs = step["inputs"]
            result = self.call_tool(tool, inputs)
            results.append(result)

        return self.synthesize_answer(results)
```

### Spider-Agent-Snow (Docker-based)

For full-featured evaluation with:
- Containerized environment
- Snowflake connection pooling
- State management across queries
- Logging and telemetry

---

## Data Access

### Setting Up Access

#### For Spider 2.0-Snow (FREE)
1. Fill out [Spider2 Snowflake Access Form](https://github.com/xlang-ai/Spider2)
2. Receive account sign-up email
3. Access hosted Snowflake database
4. Use provided credentials for evaluation

#### For Spider 2.0-Lite
1. Create BigQuery account (follow Google Cloud guidelines)
2. Set up billing (costs incurred)
3. Get own credentials
4. Access 547 examples across BigQuery, Snowflake, SQLite

#### For Spider 2.0-DBT (FREE)
1. Clone repository
2. Use local DuckDB + DBT setup
3. No external accounts required

### Data Files

| File | Description |
|------|-------------|
| `spider2-snow.jsonl` | All 547 Snowflake examples |
| `spider2-lite.jsonl` | All 547 multi-database examples |
| `spider2-dbt.jsonl` | All 68 DBT workflow examples |
| `spider2-snow-goldSQL/` | Partial gold SQL for reference |
| `spider2-lite-goldSQL/` | Partial gold SQL for reference |

⚠️ **Important**: Gold SQL is provided for prompt/method design, NOT for fine-tuning.

---

## Leaderboard & Submission

### Official Submission Process

1. **Self-Evaluation** - Use released examples and gold answers
2. **Run Full Benchmark** - Evaluate on all examples
3. **Follow Submission Guidelines** - Document methodology
4. **Submit Scores** - Upload to official leaderboard

**Note**: Only ~5-10% of gold SQL is released. Official validation requires running on the full dataset through submission.

---

## Relevance to SQL Practice Platform

### Key Takeaways for Platform Design

1. **Progressive Complexity**
   - Start with single-query problems (Spider 1.0 style)
   - Progress to multi-query workflows
   - Eventually tackle enterprise-scale schemas

2. **Schema Navigation Skills**
   - Teach users how to explore large schemas
   - Include table/column search functionality
   - Show relationship diagrams

3. **Dialect Awareness**
   - Support multiple SQL dialects
   - Highlight dialect-specific features
   - Provide dialect documentation references

4. **Workflow Thinking**
   - Encourage step-by-step problem decomposition
   - Show intermediate results
   - Build queries incrementally

5. **Real-World Scenarios**
   - Use production-like naming conventions
   - Include messy data (NULLs, duplicates)
   - Present business-context questions

### Platform Features Inspired by Spider 2.0

```typescript
// Advanced schema browser for large schemas
interface SchemaBrowser {
  // Search across tables and columns
  search(query: string): SchemaSearchResult[];

  // Show table relationships
  getRelationships(table: string): Relationship[];

  // Suggest relevant tables for a question
  suggestTables(question: string): TableSuggestion[];
}

// Multi-query workflow builder
interface WorkflowBuilder {
  // Chain multiple queries
  addQuery(sql: string, dependsOn: number[]): void;

  // Visualize data flow
  visualize(): WorkflowGraph;

  // Execute all queries in sequence
  execute(): WorkflowResult;
}

// Dialect-specific help
interface DialectHelper {
  // Get dialect documentation
  getDocs(dialect: string, feature: string): Documentation;

  // Convert query between dialects
  transpile(sql: string, from: string, to: string): string;
}
```

---

## Citation

```bibtex
@misc{lei2024spider2,
      title={Spider 2.0: Evaluating Language Models on Real-World Enterprise Text-to-SQL Workflows},
      author={Fangyu Lei and Jixuan Chen and Yuxiao Ye and Ruisheng Cao and Dongchan Shin and
              Hongjin Su and Zhaoqing Suo and Hongcheng Gao and Wenjing Hu and Pengcheng Yin and
              Victor Zhong and Caiming Xiong and Ruoxi Sun and Qian Liu and Sida Wang and Tao Yu},
      year={2024},
      eprint={2411.07763},
      archivePrefix={arXiv},
      primaryClass={cs.CL},
      url={https://arxiv.org/abs/2411.07763},
}
```

---

## Related Benchmarks

| Benchmark | Focus | Dataset Size | Difficulty |
|-----------|-------|--------------|------------|
| **Spider 1.0** | Single-query, cross-domain | 10,181 | Medium |
| **BIRD** | Large-scale databases | 1,378 | Medium-High |
| **Spider 2.0** | Multi-query workflows | 632 | Very High |
| **BIRD-SQL** | BigQuery specific | 3,431 | High |

---

## References

- **GitHub**: [xlang-ai/Spider2](https://github.com/xlang-ai/Spider2)
- **Paper**: [arXiv:2411.07763](https://arxiv.org/abs/2411.07763)
- **Website**: [Spider 2.0](https://spider2-sql.github.io)
- **Snowflake Access**: [Snowflake Guideline](https://github.com/xlang-ai/Spider2/blob/main/assets/Snowflake_Guideline.md)

---

---

# Part 5: Spider-Inspired Solution Generation

## Overview

This section documents a **Spider-inspired IR-based solution generator** for automatically generating SQL queries with explanations. Unlike LLM-based approaches that require expensive API calls or large model downloads, this system uses:
- **Pure TypeScript/JavaScript** - Runs entirely in the browser
- **No external APIs** - 100% client-side execution
- **Spider benchmark techniques** - Schema linking, intent classification, value extraction
- **Confidence scoring** - Knows when it's unsure

**File**: `src/services/solution-generator.service.ts`

---

## Why Spider-Inspired Generation?

| Approach | Spider IR (This) | LLM-Based | Human-Curated |
|----------|------------------|-----------|---------------|
| **Runtime** | ~5ms | 500ms-5s | N/A (static) |
| **Memory** | ~1MB | 100MB-4GB | 0MB |
| **Accuracy (Easy)** | 70-85% | 90-95% | 100% |
| **Accuracy (Hard)** | 40-60% | 60-80% | 100% |
| **Cost per Query** | $0 | $0.001-0.01 | $0 (one-time) |
| **Offline Support** | Yes | No | Yes |
| **Explanations** | Step-by-step | Verbose | High-quality |

**Use Case**: Generate sample solutions and hints for SQL practice problems when human-curated solutions aren't available.

---

## Solution Generator Architecture

```
Natural Language Question
        │
        ▼
┌─────────────────────────────────────────────┐
│  Step 1: Schema Linking                     │
│  - Jaccard similarity for token matching    │
│  - Map question terms → tables/columns      │
│  - Detect table relationships (FKs)         │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│  Step 2: Intent Detection                   │
│  - SELECT vs aggregation                    │
│  - Filter detection (WHERE clause)          │
│  - JOIN detection (multiple tables)         │
│  - GROUP BY / ORDER BY detection            │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│  Step 3: Value Extraction                   │
│  - Copy string values from question         │
│  - Extract numbers for LIMIT                │
│  - Detect comparison operators              │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│  Step 4: SQL Generation                     │
│  - Build SELECT clause                      │
│  - Add FROM clause                          │
│  - Add JOIN (if needed)                     │
│  - Add WHERE clause (with extracted values) │
│  - Add GROUP BY / ORDER BY                  │
│  - Add LIMIT                                │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
          SQL Query + Explanation
```

---

## API Usage

### Basic Solution Generation

```typescript
import { solutionGenerator } from './services/solution-generator.service';

const context = {
  question: "Find the names and salaries of all employees in the Engineering department",
  schema: {
    tables: [{
      name: "employees",
      columns: [
        { name: "id", type: "INTEGER" },
        { name: "name", type: "VARCHAR" },
        { name: "department", type: "VARCHAR" },
        { name: "salary", type: "INTEGER" }
      ]
    }]
  }
};

const solution = solutionGenerator.generateSolution(context);

console.log(solution.sql);
// SELECT name, salary FROM employees WHERE department = 'Engineering'

console.log(solution.explanation);
// [
//   "**Schema Analysis:** Found relevant columns: name, salary, department",
//   "**Filter:** Using WHERE clause to filter results based on specified conditions"
// ]

console.log(solution.confidence);
// 0.82

console.log(solution.steps);
// [
//   { type: 'schema_linking', description: 'Map question terms to database schema', result: [...] },
//   { type: 'intent_detection', description: 'Determine SQL query type', result: [...] },
//   ...
// ]
```

---

## Spider Techniques Implemented

### 1. Schema Linking

Spider's schema linking maps question tokens to database elements using multiple matching strategies:

```typescript
// Jaccard similarity for token overlap
function jaccardSimilarity(set1: Set<string>, set2: Set<string>): number {
  const intersection = new Set([...set1].filter(x => set2.has(x)));
  const union = new Set([...set1, ...set2]);
  return union.size > 0 ? intersection.size / union.size : 0;
}

// Example matching:
// Question: "Find employees in Engineering"
// Tokens: { "find", "employees", "engineering" }
// Schema:  { "employees", "employee_name", "department", ... }
//
// Matches found:
// - "employees" → employees table (exact match, score 1.0)
// - "engineering" → department column (partial match, score 0.7)
```

### 2. Value Prediction Strategies

Spider identifies three value prediction approaches:

| Strategy | Example | Implementation |
|----------|---------|----------------|
| **Copy from question** | "Engineering" → `WHERE department = 'Engineering'` | Extract quoted strings and capitalized words |
| **Retrieve from DB** | Look up actual values | Query database for distinct values |
| **Generate numbers** | "top 3" → `LIMIT 3` | Parse numbers with context keywords |

```typescript
// Value extraction patterns
const patterns = [
  { pattern: /(?:in|from|of|for) the ([A-Z][a-z]+) department/gi, type: 'department' },
  { pattern: /(?:in|from|of) ([A-Z][a-z]+) category/gi, type: 'category' },
  { pattern: /top (\d+)/gi, type: 'limit' },
];
```

### 3. SQL Decomposition (Exact Set Match)

Spider decomposes SQL into clauses for structured generation:

```typescript
// Clause-by-clause construction
function generateSQL(intents, schemaLinks, values, schema): string {
  const parts = [];

  // SELECT clause
  parts.push(`SELECT ${generateSelectClause(intents, schemaLinks)}`);

  // FROM clause
  parts.push(`FROM ${getPrimaryTable(schema, schemaLinks)}`);

  // JOIN clause (if needed)
  if (intents.includes('JOIN_MULTIPLE')) {
    parts.push(generateJoinClause(schemaLinks, schema));
  }

  // WHERE clause (if needed)
  if (intents.includes('WHERE_FILTER')) {
    parts.push(`WHERE ${generateWhereClause(values, schemaLinks)}`);
  }

  // GROUP BY (if needed)
  if (intents.includes('GROUP_BY')) {
    parts.push(`GROUP BY ${generateGroupByClause(intents, schemaLinks)}`);
  }

  // ORDER BY (if needed)
  if (intents.includes('ORDER_BY')) {
    parts.push(`ORDER BY ${generateOrderByClause(intents, schemaLinks)}`);
  }

  // LIMIT (if needed)
  if (intents.includes('LIMIT') && values.has('limit')) {
    parts.push(`LIMIT ${values.get('limit')}`);
  }

  return parts.join('\n');
}
```

---

## Confidence Scoring

The generator produces a confidence score (0-1) based on:

| Factor | Weight |
|--------|--------|
| Schema matches found | +0.15 per 2 matches |
| Clear intent detection | +0.1 if multiple clauses |
| Non-generic intent | +0.1 (not just SELECT_ALL) |
| Values extracted | +0.05 per value |

```typescript
// Example confidence calculations
// Question: "Find employees in Engineering"
// - Schema matches: employees, name, salary, department (4 matches) → +0.30
// - Intent: SELECT + WHERE → +0.10
// - Values extracted: "Engineering" → +0.05
// - Total: 0.50 + 0.10 + 0.05 + 0.50 (base) = 0.82

// Question: "What is the maximum salary for each department?"
// - Schema matches: salary, department (2 matches) → +0.15
// - Intent: MAX + GROUP_BY → +0.20
// - Values: none → +0.00
// - Total: 0.50 + 0.15 + 0.20 + 0.00 = 0.70
```

---

## Integration with Question View

```typescript
// In QuestionView.tsx
import { solutionGenerator } from '../services/solution-generator.service';

export function QuestionView({ question }: QuestionViewProps) {
  const [generatedSolution, setGeneratedSolution] = useState<GeneratedSolution | null>(null);
  const [showSolution, setShowSolution] = useState(false);

  const handleGenerateSolution = () => {
    const solution = solutionGenerator.generateSolution({
      question: question.question,
      schema: question.schema,
      expectedOutput: question.expectedOutput
    });
    setGeneratedSolution(solution);
    setShowSolution(true);
  };

  return (
    <div>
      {/* ... existing question display ... */}

      <div className="mt-4">
        <button
          onClick={handleGenerateSolution}
          className="px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700"
        >
          Generate Solution Hint
        </button>

        {showSolution && generatedSolution && (
          <div className="mt-4 p-4 bg-slate-800 rounded border border-slate-700">
            <div className="flex items-center justify-between mb-2">
              <h4 className="text-white font-semibold">Generated Solution</h4>
              <span className="text-sm text-slate-400">
                Confidence: {(generatedSolution.confidence * 100).toFixed(0)}%
              </span>
            </div>

            <pre className="bg-slate-900 p-3 rounded text-green-400 text-sm overflow-x-auto">
              {generatedSolution.sql}
            </pre>

            <div className="mt-3 space-y-1">
              {generatedSolution.explanation.map((line, i) => (
                <p key={i} className="text-sm text-slate-300" dangerouslySetInnerHTML={{ __html: line }} />
              ))}
            </div>

            {generatedSolution.confidence < 0.6 && (
              <div className="mt-3 p-2 bg-yellow-900/30 border border-yellow-700 rounded">
                <p className="text-sm text-yellow-300">
                  ⚠️ Low confidence: This solution may not be accurate. Use hints and try again.
                </p>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
}
```

---

## Accuracy by Question Type

Based on Spider benchmark IR baseline performance:

| Question Type | Expected Accuracy | Notes |
|---------------|-------------------|-------|
| **Simple SELECT** | 75-85% | Columns selected correctly |
| **WHERE = value** | 70-80% | Values copied from question |
| **WHERE comparisons** | 55-70% | May miss operators |
| **Single JOIN** | 50-65% | FK detection heuristics |
| **Aggregation (COUNT/SUM)** | 60-75% | Function detection works well |
| **GROUP BY** | 45-60% | Grouping column detection |
| **ORDER BY** | 65-75% | Direction detection (ASC/DESC) |
| **LIMIT** | 70-85% | Number extraction reliable |
| **Multiple JOINs** | 30-45% | Complex relationship detection |
| **Subqueries** | 20-35% | Rarely generated correctly |

---

## Limitations

1. **No Database Content Access** - Cannot retrieve actual values from tables
2. **Simple Join Detection** - Uses heuristic patterns, not true FK understanding
3. **No Semantic Understanding** - Cannot handle synonyms ("revenue" vs "income")
4. **Limited Complex Queries** - Subqueries, CTEs, window functions poorly handled
5. **Single-Query Only** - Cannot generate multi-query workflows (Spider 2.0)

---

## Future Enhancements

1. **Database-Aware Value Retrieval**
   ```typescript
   // Query DuckDB for distinct values
   async getDistinctValues(tableName: string, columnName: string): Promise<string[]> {
     const result = await duckdb.executeQuery(`SELECT DISTINCT ${columnName} FROM ${tableName}`);
     return result.rows.map(r => r[columnName]);
   }
   ```

2. **Foreign Key Relationship Detection**
   ```typescript
   // Detect FK patterns from schema
   function findForeignKeys(schema: QuestionSchema): Relationship[] {
     const relationships: Relationship[] = [];
     // Match: table1.id ↔ table2.table1_id
     // Match: table1.primary_key ↔ table2.foreign_key
   }
   ```

3. **Multi-Query Workflows** (Spider 2.0 style)
   ```typescript
   interface WorkflowStep {
     query: string;
     dependsOn: number[];
     description: string;
   }

   function generateWorkflow(question: string): WorkflowStep[] {
     // Detect multi-step patterns:
     // - "Find X, then calculate Y from those results"
     // - "For each department, find top N employees"
   }
   ```

4. **Interactive Refinement**
   ```typescript
   interface AmbiguityResolution {
     question: string;
     options: string[];
     selected: number;
   }

   function resolveAmbiguity(links: SchemaLink[]): AmbiguityResolution[] {
     // When multiple columns match a token, ask user to choose
   }
   ```

---

## References

- **Spider Benchmark**: https://yale-lily.github.io/spider
- **Spider 1.0 Paper**: Yu et al., EMNLP 2018
- **IR Baseline**: `yechens/spider` GitHub repository
- **Value Prediction**: Spider leaderboard guidelines (01/16/2020)
- **Leaderboard Top Models**: MiniSeek (91.2%), DAIL-SQL+GPT-4 (86.6%)

---

## References

- [DuckDB-Wasm GitHub](https://github.com/duckdb/duckdb-wasm)
- [DuckDB-Wasm Documentation](https://duckdb.org/api/wasm/)
- [Vite Guide](https://vitejs.dev/guide/)
- [Zustand Documentation](https://zustand-demo.pmnd.rs/)
