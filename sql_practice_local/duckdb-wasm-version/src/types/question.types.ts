/**
 * Question data types for the SQL practice platform
 */

export interface QuestionColumn {
  name: string;
  type: string; // INTEGER, VARCHAR, FLOAT, etc.
}

export interface QuestionTable {
  name: string;
  columns: QuestionColumn[];
}

export interface QuestionSchema {
  tables: QuestionTable[];
  sampleData: Record<string, Record<string, string | number | null>[]>;
}

export interface Question {
  id: string;
  slug: string;
  title: string;
  description: string;
  difficulty: 'easy' | 'medium' | 'hard';
  category: string;
  order: number;
  company?: string;
  tags?: string[];
  schema: QuestionSchema;
  question: string;
  startingCode?: string;
  hints?: string[];
  expectedOutput: {
    columns: string[];
    rows: Record<string, string | number | null>[];
  };
}

export interface QueryResult {
  success: boolean;
  columns: string[];
  rows: Record<string, string | number | null>[];
  executionTimeMs: number;
  rowCount: number;
  errorMessage?: string;
}
