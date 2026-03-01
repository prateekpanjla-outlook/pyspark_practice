/**
 * SQL Solution Generator Service
 * Inspired by Spider benchmark techniques for text-to-SQL generation
 * Uses IR-based (non-neural) approaches for solution generation
 */

import type { QuestionSchema, QuestionTable, QuestionColumn } from '../types/question.types';

export interface SolutionContext {
  question: string;
  schema: QuestionSchema;
  expectedOutput?: {
    columns: string[];
    rows: Record<string, unknown>[];
  };
}

export interface GeneratedSolution {
  sql: string;
  explanation: string[];
  confidence: number;
  steps: SolutionStep[];
}

export interface SolutionStep {
  type: 'schema_linking' | 'intent_detection' | 'clause_generation' | 'value_extraction';
  description: string;
  result: string | string[];
}

/**
 * Solution Generator using Spider-inspired IR techniques
 */
export class SolutionGeneratorService {
  /**
   * Generate a SQL solution with explanation
   */
  generateSolution(context: SolutionContext): GeneratedSolution {
    const steps: SolutionStep[] = [];
    const explanation: string[] = [];

    // Step 1: Schema Linking - Map question terms to schema
    const schemaLinks = this.schemaLinking(context.question, context.schema);
    steps.push({
      type: 'schema_linking',
      description: 'Map question terms to database schema',
      result: schemaLinks
    });

    // Step 2: Intent Detection - Determine query type
    const intent = this.detectIntent(context.question, schemaLinks);
    steps.push({
      type: 'intent_detection',
      description: 'Determine SQL query type',
      result: intent
    });

    // Step 3: Value Extraction - Extract values for WHERE/HAVING clauses
    const values = this.extractValues(context.question, schemaLinks);
    steps.push({
      type: 'value_extraction',
      description: 'Extract values from question',
      result: values
    });

    // Step 4: Clause Generation - Build SQL clause by clause
    const sql = this.generateSQL(intent, schemaLinks, values, context.schema);
    steps.push({
      type: 'clause_generation',
      description: 'Generate SQL query',
      result: [sql]
    });

    // Generate explanation
    explanation.push(...this.generateExplanation(intent, schemaLinks, values));

    return {
      sql,
      explanation,
      confidence: this.calculateConfidence(intent, schemaLinks, values),
      steps
    };
  }

  /**
   * Schema Linking - Map question terms to database columns/tables
   * Uses Jaccard similarity for fuzzy matching
   */
  private schemaLinking(question: string, schema: QuestionSchema): string[] {
    const links: string[] = [];
    const questionTokens = this.tokenize(question.toLowerCase());
    const linksMap = new Map<string, number>();

    // Find matching tables
    for (const table of schema.tables) {
      const tableTokens = this.tokenize(table.name.toLowerCase());
      const score = this.jaccardSimilarity(questionTokens, tableTokens);
      if (score > 0.3) {
        linksMap.set(`Table: ${table.name}`, Math.max(linksMap.get(`Table: ${table.name}`) || 0, score));
      }
    }

    // Find matching columns
    for (const table of schema.tables) {
      for (const column of table.columns) {
        const columnTokens = this.tokenize(column.name.toLowerCase());
        const score = this.jaccardSimilarity(questionTokens, columnTokens);

        // Also check for partial matches (e.g., "salary" matches "salaries")
        const partialScore = questionTokens.some(t =>
          columnTokens.some(ct => ct.includes(t) || t.includes(ct))
        ) ? 0.5 : 0;

        const finalScore = Math.max(score, partialScore);
        if (finalScore > 0.2) {
          linksMap.set(`${table.name}.${column.name}`, Math.max(linksMap.get(`${table.name}.${column.name}`) || 0, finalScore));
        }
      }
    }

    // Sort by score and return top matches
    return Array.from(linksMap.entries())
      .sort((a, b) => b[1] - a[1])
      .map(([link]) => link);
  }

  /**
   * Intent Detection - Determine what type of SQL query is needed
   */
  private detectIntent(question: string, schemaLinks: string[]): string[] {
    const intents: string[] = [];
    const lowerQ = question.toLowerCase();

    // Detect SELECT type
    if (lowerQ.includes('all') || lowerQ.includes('every') || lowerQ.includes('list')) {
      intents.push('SELECT_ALL');
    } else if (lowerQ.includes('count') || lowerQ.includes('how many') || lowerQ.includes('number of')) {
      intents.push('SELECT_COUNT');
    } else if (lowerQ.includes('total') || lowerQ.includes('sum')) {
      intents.push('SELECT_SUM');
    } else if (lowerQ.includes('average') || lowerQ.includes('avg')) {
      intents.push('SELECT_AVG');
    } else if (lowerQ.includes('maximum') || lowerQ.includes('highest') || lowerQ.includes('max')) {
      intents.push('SELECT_MAX');
    } else if (lowerQ.includes('minimum') || lowerQ.includes('lowest') || lowerQ.includes('min')) {
      intents.push('SELECT_MIN');
    }

    // Detect WHERE clause
    if (lowerQ.includes('where') || lowerQ.includes('with') || lowerQ.includes('having') ||
        lowerQ.includes('in the') || lowerQ.includes('is a') || lowerQ.includes('that are')) {
      intents.push('WHERE_FILTER');
    }

    // Detect JOIN
    if (schemaLinks.some(l => l.includes('.')) && schemaLinks.filter(l => l.includes('.')).length > 1) {
      const tables = [...new Set(schemaLinks.map(l => l.split('.')[0]))];
      if (tables.length > 1) {
        intents.push('JOIN_MULTIPLE');
      }
    }

    // Detect GROUP BY
    if (lowerQ.includes('each') || lowerQ.includes('per') || lowerQ.includes('by') ||
        lowerQ.includes('group')) {
      intents.push('GROUP_BY');
    }

    // Detect ORDER BY
    if (lowerQ.includes('sort') || lowerQ.includes('order') ||
        lowerQ.includes('highest') || lowerQ.includes('lowest') ||
        lowerQ.includes('first') || lowerQ.includes('last')) {
      intents.push('ORDER_BY');
      if (lowerQ.includes('desc') || lowerQ.includes('highest') || lowerQ.includes('most')) {
        intents.push('ORDER_DESC');
      }
    }

    // Detect LIMIT
    if (lowerQ.includes('top') || lowerQ.includes('first') || lowerQ.includes('limit')) {
      intents.push('LIMIT');
    }

    return intents.length > 0 ? intents : ['SELECT_ALL'];
  }

  /**
   * Value Extraction - Extract values for WHERE/HAVING clauses
   */
  private extractValues(question: string, schemaLinks: string[]): Map<string, string> {
    const values = new Map<string, string>();
    const lowerQ = question;

    // Extract quoted strings
    const quotedMatches = lowerQ.match(/["']([^"']+)["']/g);
    if (quotedMatches) {
      for (const match of quotedMatches) {
        const value = match.slice(1, -1);
        // Find corresponding column from schema links
        for (const link of schemaLinks) {
          if (link.includes('.')) {
            const col = link.split('.')[1];
            if (lowerQ.toLowerCase().includes(value.toLowerCase()) || value.length > 2) {
              values.set(col, value);
            }
          }
        }
      }
    }

    // Extract department names, categories (common patterns)
    const commonPatterns = [
      { pattern: /(?:in|from|of|for) the ([A-Z][a-z]+(?: [A-Z][a-z]+)??) department/gi, type: 'department' },
      { pattern: /(?:in|from|of|for) ([A-Z][a-z]+(?: [A-Z][a-z]+)??) (?:category|type)/gi, type: 'category' },
    ];

    for (const { pattern, type } of commonPatterns) {
      const matches = [...question.matchAll(pattern)];
      for (const match of matches) {
        if (match[1]) {
          values.set(type, match[1]);
        }
      }
    }

    // Extract numbers
    const numberMatches = question.match(/\b(\d+)\b/g);
    if (numberMatches) {
      for (const num of numberMatches) {
        if (question.toLowerCase().includes(`top ${num}`) ||
            question.toLowerCase().includes(`first ${num}`) ||
            question.toLowerCase().includes(`limit ${num}`)) {
          values.set('limit', num);
        }
      }
    }

    return values;
  }

  /**
   * Generate SQL from detected components
   */
  private generateSQL(
    intents: string[],
    schemaLinks: string[],
    values: Map<string, string>,
    schema: QuestionSchema
  ): string {
    const parts: string[] = [];

    // Determine SELECT clause
    const selectColumns = this.generateSelectClause(intents, schemaLinks, values);
    parts.push(`SELECT ${selectColumns}`);

    // Determine FROM clause
    const tables = this.getPrimaryTable(schema, schemaLinks);
    parts.push(`FROM ${tables}`);

    // Add JOIN if needed
    if (intents.includes('JOIN_MULTIPLE')) {
      const joinClause = this.generateJoinClause(schemaLinks, schema);
      if (joinClause) parts.push(joinClause);
    }

    // Add WHERE clause
    if (intents.includes('WHERE_FILTER')) {
      const whereClause = this.generateWhereClause(values, schemaLinks);
      if (whereClause) parts.push(`WHERE ${whereClause}`);
    }

    // Add GROUP BY
    if (intents.includes('GROUP_BY')) {
      const groupBy = this.generateGroupByClause(intents, schemaLinks);
      if (groupBy) parts.push(`GROUP BY ${groupBy}`);
    }

    // Add ORDER BY
    if (intents.includes('ORDER_BY')) {
      const orderBy = this.generateOrderByClause(intents, schemaLinks);
      if (orderBy) parts.push(`ORDER BY ${orderBy}`);
    }

    // Add LIMIT
    if (intents.includes('LIMIT') && values.has('limit')) {
      parts.push(`LIMIT ${values.get('limit')}`);
    }

    return parts.join('\n');
  }

  private generateSelectClause(intents: string[], schemaLinks: string[], values: Map<string, string>): string {
    // Aggregate functions
    if (intents.includes('SELECT_COUNT')) return 'COUNT(*)';
    if (intents.includes('SELECT_SUM')) return `SUM(${this.getColumnFromLinks(schemaLinks, 'amount|total|price|cost|salary')})`;
    if (intents.includes('SELECT_AVG')) return `AVG(${this.getColumnFromLinks(schemaLinks, 'amount|total|price|cost|salary|score')})`;
    if (intents.includes('SELECT_MAX')) return `MAX(${this.getColumnFromLinks(schemaLinks, 'amount|total|price|cost|salary|score')})`;
    if (intents.includes('SELECT_MIN')) return `MIN(${this.getColumnFromLinks(schemaLinks, 'amount|total|price|cost|salary|score')})`;

    // Regular columns
    const columns = schemaLinks
      .filter(l => l.includes('.'))
      .map(l => l.split('.')[1])
      .slice(0, 3); // Limit to 3 columns

    return columns.length > 0 ? columns.join(', ') : '*';
  }

  private generateWhereClause(values: Map<string, string>, schemaLinks: string[]): string | null {
    const conditions: string[] = [];

    for (const [key, value] of values.entries()) {
      if (key === 'limit') continue; // Skip limit, used separately

      const column = this.findColumnForKey(key, schemaLinks);
      if (column) {
        const numValue = parseFloat(value);
        conditions.push(`${column} = ${isNaN(numValue) ? `'${value}'` : numValue}`);
      }
    }

    return conditions.length > 0 ? conditions.join(' AND ') : null;
  }

  private generateJoinClause(schemaLinks: string[], schema: QuestionSchema): string | null {
    const tables = [...new Set(schemaLinks.map(l => l.split('.')[0]))];
    if (tables.length < 2) return null;

    // Simple inner join based on common patterns
    const joins: string[] = [];
    for (let i = 1; i < tables.length; i++) {
      joins.push(`JOIN ${tables[i]} ON ${tables[0]}.id = ${tables[i]}.${tables[0].toLowerCase()}_id`);
    }

    return joins.join(' ');
  }

  private generateGroupByClause(intents: string[], schemaLinks: string[]): string | null {
    // Find grouping column (usually the non-aggregated one)
    for (const link of schemaLinks) {
      if (link.includes('.')) {
        const col = link.split('.')[1];
        if (!this.isAggregateColumn(col)) {
          return col;
        }
      }
    }
    return null;
  }

  private generateOrderByClause(intents: string[], schemaLinks: string[]): string | null {
    const column = this.getColumnFromLinks(schemaLinks, 'amount|total|price|cost|salary|score|date');
    if (intents.includes('ORDER_DESC')) {
      return `${column} DESC`;
    }
    return column;
  }

  private generateExplanation(intents: string[], schemaLinks: string[], values: Map<string, string>): string[] {
    const explanation: string[] = [];

    if (schemaLinks.length > 0) {
      explanation.push(`**Schema Analysis:** Found relevant columns: ${schemaLinks.filter(l => l.includes('.')).map(l => l.split('.')[1]).join(', ')}`);
    }

    if (intents.includes('WHERE_FILTER')) {
      explanation.push('**Filter:** Using WHERE clause to filter results based on specified conditions');
    }

    if (intents.includes('GROUP_BY')) {
      explanation.push('**Aggregation:** Using GROUP BY to calculate values per category');
    }

    if (intents.includes('ORDER_BY')) {
      explanation.push('**Sorting:** Using ORDER BY to sort results');
    }

    if (intents.includes('JOIN_MULTIPLE')) {
      explanation.push('**Join:** Combining data from multiple tables using JOIN');
    }

    return explanation;
  }

  private calculateConfidence(intents: string[], schemaLinks: string[], values: Map<string, string>): number {
    let confidence = 0.5;

    // Higher confidence if we found good schema matches
    if (schemaLinks.length >= 2) confidence += 0.15;
    if (schemaLinks.length >= 3) confidence += 0.1;

    // Higher confidence if we detected clear intent
    if (intents.length > 1) confidence += 0.1;
    if (!intents.includes('SELECT_ALL')) confidence += 0.1;

    // Higher confidence if we extracted values
    if (values.size > 0) confidence += 0.05;

    return Math.min(confidence, 0.95);
  }

  // Helper methods

  private tokenize(text: string): Set<string> {
    return new Set(text.toLowerCase().split(/\s+/).filter(t => t.length > 2));
  }

  private jaccardSimilarity(set1: Set<string>, set2: Set<string>): number {
    const intersection = new Set([...set1].filter(x => set2.has(x)));
    const union = new Set([...set1, ...set2]);
    return union.size > 0 ? intersection.size / union.size : 0;
  }

  private getPrimaryTable(schema: QuestionSchema, schemaLinks: string[]): string {
    const linkedTables = schemaLinks.map(l => l.split('.')[0]);
    return schema.tables.find(t => linkedTables.includes(t.name))?.name || schema.tables[0].name;
  }

  private getColumnFromLinks(schemaLinks: string[], pattern: string): string {
    const regex = new RegExp(pattern, 'i');
    for (const link of schemaLinks) {
      if (link.includes('.')) {
        const col = link.split('.')[1];
        if (regex.test(col)) return col;
      }
    }
    return schemaLinks.find(l => l.includes('.'))?.split('.')[1] || '*';
  }

  private findColumnForKey(key: string, schemaLinks: string[]): string | null {
    // Try exact match first
    for (const link of schemaLinks) {
      if (link.includes('.')) {
        const col = link.split('.')[1].toLowerCase();
        if (col === key.toLowerCase() || col.includes(key.toLowerCase())) {
          return link.split('.')[1];
        }
      }
    }
    return null;
  }

  private isAggregateColumn(col: string): boolean {
    return /^(amount|total|sum|count|average|avg|max|min)$/i.test(col);
  }
}

// Export singleton instance
export const solutionGenerator = new SolutionGeneratorService();
