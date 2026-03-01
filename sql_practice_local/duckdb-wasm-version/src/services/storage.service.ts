/**
 * Storage Service
 * Handles localStorage/IndexedDB for user progress persistence
 */

export interface QuestionProgress {
  slug: string;
  solved: boolean;
  attempts: number;
  lastAttemptAt: number;
  solvedAt?: number;
  bestTime?: number; // in ms
  code?: string; // Last submitted code
}

export interface UserProgress {
  questions: Record<string, QuestionProgress>;
  totalSolved: number;
  lastUpdated: number;
}

const STORAGE_KEY = 'sql-practice-progress';
const STORAGE_VERSION = 1;

class StorageService {
  private cache: UserProgress | null = null;

  /**
   * Get all user progress
   */
  getProgress(): UserProgress {
    if (this.cache) return this.cache;

    try {
      const data = localStorage.getItem(STORAGE_KEY);
      if (!data) {
        return this.getEmptyProgress();
      }

      const parsed = JSON.parse(data);

      // Migration check
      if (parsed.version !== STORAGE_VERSION) {
        return this.migrateProgress(parsed);
      }

      this.cache = parsed;
      return parsed;
    } catch (error) {
      console.error('Failed to load progress:', error);
      return this.getEmptyProgress();
    }
  }

  /**
   * Get progress for a specific question
   */
  getQuestionProgress(slug: string): QuestionProgress | null {
    const progress = this.getProgress();
    return progress.questions[slug] || null;
  }

  /**
   * Check if a question is solved
   */
  isQuestionSolved(slug: string): boolean {
    return this.getQuestionProgress(slug)?.solved ?? false;
  }

  /**
   * Record an attempt for a question
   */
  recordAttempt(slug: string, solved: boolean, timeMs?: number, code?: string): void {
    const progress = this.getProgress();
    const existing = progress.questions[slug];

    const questionProgress: QuestionProgress = {
      slug,
      solved,
      attempts: (existing?.attempts || 0) + 1,
      lastAttemptAt: Date.now(),
      ...(solved && { solvedAt: Date.now() }),
      ...(timeMs && (!existing?.bestTime || timeMs < existing.bestTime) && { bestTime: timeMs }),
      ...(code && { code }),
    };

    // Update question progress
    progress.questions[slug] = questionProgress;

    // Update totals
    progress.totalSolved = Object.values(progress.questions).filter((q) => q.solved).length;
    progress.lastUpdated = Date.now();

    this.cache = progress;
    this.saveProgress(progress);
  }

  /**
   * Reset progress for a specific question
   */
  resetQuestion(slug: string): void {
    const progress = this.getProgress();
    delete progress.questions[slug];
    progress.totalSolved = Object.values(progress.questions).filter((q) => q.solved).length;
    progress.lastUpdated = Date.now();

    this.cache = progress;
    this.saveProgress(progress);
  }

  /**
   * Reset all progress
   */
  resetAll(): void {
    this.cache = this.getEmptyProgress();
    this.saveProgress(this.cache);
  }

  /**
   * Get statistics
   */
  getStats() {
    const progress = this.getProgress();
    const questions = Object.values(progress.questions);
    const solved = questions.filter((q) => q.solved);

    return {
      total: questions.length,
      solved: solved.length,
      attempts: questions.reduce((sum, q) => sum + q.attempts, 0),
      averageAttempts: solved.length > 0
        ? solved.reduce((sum, q) => sum + q.attempts, 0) / solved.length
        : 0,
      bestTime: solved.length > 0
        ? Math.min(...solved.map((q) => q.bestTime || Infinity))
        : null,
    };
  }

  /**
   * Export progress as JSON
   */
  exportProgress(): string {
    return JSON.stringify(this.getProgress(), null, 2);
  }

  /**
   * Import progress from JSON
   */
  importProgress(json: string): boolean {
    try {
      const parsed = JSON.parse(json);
      if (parsed.questions) {
        this.cache = parsed;
        this.saveProgress(parsed);
        return true;
      }
      return false;
    } catch {
      return false;
    }
  }

  private getEmptyProgress(): UserProgress {
    return {
      questions: {},
      totalSolved: 0,
      lastUpdated: Date.now(),
    };
  }

  private saveProgress(progress: UserProgress): void {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(progress));
    } catch (error) {
      console.error('Failed to save progress:', error);
    }
  }

  private migrateProgress(oldData: any): UserProgress {
    // Handle future migrations here
    const progress: UserProgress = {
      questions: oldData.questions || {},
      totalSolved: oldData.totalSolved || 0,
      lastUpdated: Date.now(),
    };
    this.cache = progress;
    this.saveProgress(progress);
    return progress;
  }
}

// Singleton instance
export const storageService = new StorageService();
