/**
 * Question Service
 * Loads question data from JSON files
 */

import type { Question } from '../types/question.types';

export class QuestionService {
  private questionsCache: Map<string, Question> = new Map();
  private indexCache: Question[] | null = null;

  /**
   * Load all questions from index
   */
  async loadQuestions(): Promise<Question[]> {
    if (this.indexCache) return this.indexCache;

    try {
      const response = await fetch('/data/questions/index.json');
      if (!response.ok) {
        throw new Error(`Failed to load questions: ${response.statusText}`);
      }

      const questions: Question[] = await response.json();

      // Cache questions
      questions.forEach((q) => this.questionsCache.set(q.slug, q));
      this.indexCache = questions;

      return questions;
    } catch (error) {
      console.error('Failed to load questions:', error);
      throw error;
    }
  }

  /**
   * Load a single question by slug
   */
  async loadQuestion(slug: string): Promise<Question | null> {
    // Check cache first
    if (this.questionsCache.has(slug)) {
      return this.questionsCache.get(slug)!;
    }

    try {
      const response = await fetch(`/data/questions/${slug}.json`);
      if (!response.ok) {
        return null;
      }

      const question: Question = await response.json();
      this.questionsCache.set(slug, question);

      return question;
    } catch (error) {
      console.error(`Failed to load question "${slug}":`, error);
      return null;
    }
  }

  /**
   * Get questions by difficulty
   */
  async getQuestionsByDifficulty(difficulty: 'easy' | 'medium' | 'hard'): Promise<Question[]> {
    const all = await this.loadQuestions();
    return all.filter((q) => q.difficulty === difficulty);
  }

  /**
   * Get questions by category
   */
  async getQuestionsByCategory(category: string): Promise<Question[]> {
    const all = await this.loadQuestions();
    return all.filter((q) => q.category === category);
  }

  /**
   * Search questions
   */
  async searchQuestions(query: string): Promise<Question[]> {
    const all = await this.loadQuestions();
    const lowerQuery = query.toLowerCase();

    return all.filter(
      (q) =>
        q.title.toLowerCase().includes(lowerQuery) ||
        q.description.toLowerCase().includes(lowerQuery) ||
        q.tags?.some((t) => t.toLowerCase().includes(lowerQuery))
    );
  }

  /**
   * Clear cache
   */
  clearCache(): void {
    this.questionsCache.clear();
    this.indexCache = null;
  }
}
