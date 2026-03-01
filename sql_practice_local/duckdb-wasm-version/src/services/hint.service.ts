/**
 * SQL Hint Analyzer Service
 * Generates contextual hints based on schema analysis and query patterns
 */

import type { QuestionSchema, QuestionTable, QuestionColumn } from '../types/question.types';

export interface HintContext {
  schema: QuestionSchema;
  question: string;
  userQuery?: string;
  expectedColumns: string[];
}

export interface GeneratedHint {
  text: string;
  type: 'schema' | 'syntax' | 'logic' | 'debug';
  priority: number;
}

/**
 * Analyzes the question and schema to generate helpful hints
 */
export class HintService {
  /**
   * Generate hints based on schema and question context
   */
  generateHints(context: HintContext): GeneratedHint[] {
    const hints: GeneratedHint[] = [];
    const { schema, question, expectedColumns } = context;

    // Schema-based hints
    hints.push(...this.getSchemaHints(schema, question));

    // Expected output hints
    hints.push(...this.getOutputHints(schema, expectedColumns));

    // Question analysis hints
    hints.push(...this.getQuestionHints(question, schema));

    // Sort by priority and return top 3
    return hints.sort((a, b) => b.priority - a.priority).slice(0, 3);
  }

  /**
   * Generate hints based on available tables and columns
   */
  private getSchemaHints(schema: QuestionSchema, question: string): GeneratedHint[] {
    const hints: GeneratedHint[] = [];
    const tables = schema.tables;

    if (tables.length === 0) return hints;

    // List available tables
    if (question.toLowerCase().includes('join') || tables.length > 1) {
      const tableNames = tables.map(t => t.name).join(', ');
      hints.push({
        text: `Available tables: ${tableNames}`,
        type: 'schema',
        priority: 3
      });
    }

    // For single table queries, highlight key columns
    if (tables.length === 1) {
      const table = tables[0];
      const columnNames = table.columns.map(c => c.name).join(', ');
      hints.push({
        text: `Available columns in ${table.name}: ${columnNames}`,
        type: 'schema',
        priority: 2
      });
    }

    // Find foreign key relationships for JOIN hints
    const relationships = this.findRelationships(schema);
    if (relationships.length > 0 && question.toLowerCase().includes('join')) {
      relationships.forEach(rel => {
        hints.push({
          text: `Join hint: ${rel.fromTable}.${rel.fromColumn} matches ${rel.toTable}.${rel.toColumn}`,
          type: 'schema',
          priority: 4
        });
      });
    }

    return hints;
  }

  /**
   * Generate hints based on expected output columns
   */
  private getOutputHints(schema: QuestionSchema, expectedColumns: string[]): GeneratedHint[] {
    const hints: GeneratedHint[] = [];

    if (expectedColumns.length === 0) return hints;

    // Check if expected columns exist in schema
    const allColumns = new Map<string, string>();
    for (const table of schema.tables) {
      for (const col of table.columns) {
        allColumns.set(col.name.toLowerCase(), `${table.name}.${col.name}`);
      }
    }

    const missingColumns = expectedColumns.filter(
      col => !allColumns.has(col.toLowerCase()) && !col.includes('(')
    );

    if (missingColumns.length > 0) {
      hints.push({
        text: `Note: Some expected columns may be calculated: ${missingColumns.join(', ')}`,
        type: 'logic',
        priority: 2
      });
    }

    // Aggregate function hints
    const aggPatterns = ['total', 'sum', 'count', 'average', 'avg', 'max', 'min'];
    const hasAggregate = expectedColumns.some(col =>
      aggPatterns.some(pattern => col.toLowerCase().includes(pattern))
    );

    if (hasAggregate) {
      hints.push({
        text: 'Consider using aggregate functions like SUM(), COUNT(), AVG(), MAX(), or MIN()',
        type: 'syntax',
        priority: 3
      });
    }

    return hints;
  }

  /**
   * Generate hints based on question text analysis
   */
  private getQuestionHints(question: string, schema: QuestionSchema): GeneratedHint[] {
    const hints: GeneratedHint[] = [];
    const lowerQ = question.toLowerCase();

    // Filter hints
    if (lowerQ.includes('where') || lowerQ.includes('find') || lowerQ.includes('filter')) {
      hints.push({
        text: 'Use the WHERE clause to filter rows based on a condition',
        type: 'syntax',
        priority: 2
      });
    }

    // Sorting hints
    if (lowerQ.includes('sort') || lowerQ.includes('order') || lowerQ.includes('highest') || lowerQ.includes('lowest')) {
      hints.push({
        text: 'Use ORDER BY to sort results. Add DESC for descending (highest first) or ASC for ascending (lowest first)',
        type: 'syntax',
        priority: 3
      });
    }

    // Join hints
    if (lowerQ.includes('combine') || lowerQ.includes('together') || lowerQ.includes('match')) {
      hints.push({
        text: 'Use JOIN to combine data from multiple tables. Specify the join condition using ON',
        type: 'syntax',
        priority: 3
      });
    }

    // Grouping hints
    if (lowerQ.includes('each') || lowerQ.includes('per') || lowerQ.includes('group')) {
      hints.push({
        text: 'Use GROUP BY to aggregate data by category. Columns in SELECT must either be in GROUP BY or use aggregate functions',
        type: 'syntax',
        priority: 3
      });
    }

    // Limit hints
    if (lowerQ.includes('top') || lowerQ.includes('first') || lowerQ.includes('maximum')) {
      const hasLimit = lowerQ.includes('top 1') || lowerQ.includes('first 1');
      if (!hasLimit) {
        hints.push({
          text: 'Consider using LIMIT to restrict the number of rows returned',
          type: 'syntax',
          priority: 1
        });
      }
    }

    return hints;
  }

  /**
   * Find potential foreign key relationships between tables
   */
  private findRelationships(schema: QuestionSchema): Array<{
    fromTable: string;
    fromColumn: string;
    toTable: string;
    toColumn: string;
  }> {
    const relationships: Array<{
      fromTable: string;
      fromColumn: string;
      toTable: string;
      toColumn: string;
    }> = [];

    const tables = schema.tables;

    for (let i = 0; i < tables.length; i++) {
      for (let j = i + 1; j < tables.length; j++) {
        const tableA = tables[i];
        const tableB = tables[j];

        // Find columns with matching names
        for (const colA of tableA.columns) {
          for (const colB of tableB.columns) {
            const nameA = colA.name.toLowerCase().replace(/_?id$/, '');
            const nameB = colB.name.toLowerCase().replace(/_?id$/, '');

            // Check for common foreign key patterns
            if (colA.name.toLowerCase() === `${tableB.name.toLowerCase()}_id` ||
                colB.name.toLowerCase() === `${tableA.name.toLowerCase()}_id`) {
              relationships.push({
                fromTable: tableA.name,
                fromColumn: colA.name,
                toTable: tableB.name,
                toColumn: colB.name
              });
            }
            // Check for id columns that match table names
            else if (colA.name.toLowerCase() === 'id' && tableB.name.toLowerCase() === nameB) {
              relationships.push({
                fromTable: tableA.name,
                fromColumn: colA.name,
                toTable: tableB.name,
                toColumn: colB.name
              });
            }
          }
        }
      }
    }

    return relationships;
  }

  /**
   * Analyze user query and provide specific debugging hints
   */
  analyzeQuery(query: string, context: HintContext): GeneratedHint[] {
    const hints: GeneratedHint[] = [];
    const lowerQuery = query.toLowerCase();

    // Check for common mistakes
    if (!lowerQuery.includes('select')) {
      hints.push({
        text: 'Your query must start with SELECT to specify columns',
        type: 'syntax',
        priority: 5
      });
    }

    if (!lowerQuery.includes('from')) {
      hints.push({
        text: 'Your query must include FROM to specify the table',
        type: 'syntax',
        priority: 5
      });
    }

    // Check for GROUP BY without aggregate
    if (lowerQuery.includes('group by')) {
      const hasAggregate = /sum|count|avg|min|max/i.test(query);
      if (!hasAggregate) {
        hints.push({
          text: 'When using GROUP BY, you typically need an aggregate function like SUM(), COUNT(), etc.',
          type: 'logic',
          priority: 3
        });
      }
    }

    // Check for JOIN without ON clause
    if (lowerQuery.includes('join') && !lowerQuery.includes(' on ')) {
      hints.push({
        text: 'JOIN requires an ON clause to specify how tables are related',
        type: 'syntax',
        priority: 4
      });
    }

    return hints;
  }

  /**
   * Enhance static hints with schema context
   */
  enrichHints(staticHints: string[], context: HintContext): string[] {
    return staticHints.map(hint => this.enrichHint(hint, context));
  }

  private enrichHint(hint: string, context: HintContext): string {
    let enriched = hint;

    // Replace table/column placeholders with actual schema values
    const tables = context.schema.tables.map(t => t.name);
    const allColumns = context.schema.tables.flatMap(t =>
      t.columns.map(c => `${t.name}.${c.name}`)
    );

    // Simple placeholder replacement (can be extended)
    if (hint.includes('{table}')) {
      enriched = enriched.replace('{table}', tables[0] || 'table_name');
    }
    if (hint.includes('{tables}')) {
      enriched = enriched.replace('{tables}', tables.join(', '));
    }

    return enriched;
  }
}

// Export singleton instance
export const hintService = new HintService();
