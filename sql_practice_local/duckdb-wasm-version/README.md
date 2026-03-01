# SQL Practice - DuckDB-Wasm Version

A browser-based SQL practice platform that runs entirely client-side using DuckDB-Wasm.

## Features

- 🌐 **100% Browser-Based** - No server required for query execution
- ⚡ **Zero Latency** - All queries run locally in your browser
- 💰 **Free Hosting** - Deploy to Netlify, Vercel, or GitHub Pages
- 📴 **Offline Support** - Works offline with service worker
- 🔒 **Privacy-First** - Your queries never leave your browser
- 🚀 **Fast Development** - Modern React + TypeScript + Vite stack

## Tech Stack

- **Frontend**: React 18 + TypeScript
- **Build Tool**: Vite
- **SQL Engine**: DuckDB-Wasm
- **State Management**: Zustand (optional)
- **Icons**: Lucide React
- **Styling**: Tailwind CSS

## Getting Started

### Prerequisites

- Node.js 18+
- npm or yarn

### Installation

```bash
# Install dependencies
npm install

# Download DuckDB-Wasm files
npm run download:wasm

# Start development server
npm run dev
```

### Download WASM Files

The DuckDB-Wasm files need to be downloaded separately. Run:

```bash
node scripts/download-wasm.js
```

This will download the WASM binaries to `public/duckdb-wasm/`.

## Project Structure

```
duckdb-wasm-version/
├── public/
│   ├── duckdb-wasm/          # WASM files (download via script)
│   └── data/
│       └── questions/        # Question JSON files
├── src/
│   ├── App.tsx               # Main application
│   ├── main.tsx              # Entry point
│   ├── components/           # React components
│   ├── services/             # DuckDB and question services
│   ├── hooks/                # Custom React hooks
│   ├── store/                # Zustand state management
│   └── types/                # TypeScript type definitions
├── ARCHITECTURE.md           # Detailed architecture documentation
├── COMPARISON.md             # C++ vs WASM comparison
└── README.md                 # This file
```

## Adding Questions

Questions are stored as JSON files in `public/data/questions/`. See the schema in [ARCHITECTURE.md](ARCHITECTURE.md).

Example question format:

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
          {"name": "department", "type": "VARCHAR"}
        ]
      }
    ],
    "sampleData": {
      "employees": [
        {"id": 1, "name": "Alice", "department": "Engineering"},
        {"id": 2, "name": "Bob", "department": "Sales"}
      ]
    }
  },
  "question": "Find all employees in Engineering",
  "startingCode": "SELECT * FROM employees",
  "hints": ["Use WHERE clause"],
  "expectedOutput": {
    "columns": ["id", "name", "department"],
    "rows": [
      {"id": 1, "name": "Alice", "department": "Engineering"}
    ]
  }
}
```

## Deployment

### Netlify

```bash
npm run build
netlify deploy --prod --dir=dist
```

### Vercel

```bash
npm run build
vercel --prod
```

### GitHub Pages

```bash
npm run build
# Push dist/ folder to gh-pages branch
```

**Important**: Your hosting must support COOP/COEP headers for multi-threading. See [vite.config.ts](vite.config.ts) for the required headers.

## Development

```bash
# Start dev server (with HMR)
npm run dev

# Build for production
npm run build

# Preview production build
npm run preview
```

## Architecture

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed architecture documentation.

## License

MIT
