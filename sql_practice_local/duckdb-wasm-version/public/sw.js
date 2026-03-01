/**
 * Service Worker for SQL Practice App
 * Enables offline functionality by caching static assets and data
 */

const CACHE_NAME = 'sql-practice-v1';
const CACHE_VERSION = 1;

// Assets to cache immediately (install)
const PRECACHE_ASSETS = [
  '/',
  '/index.html',
  '/data/questions/index.json',
  '/duckdb-wasm/duckdb-mvp.wasm',
  '/duckdb-wasm/duckdb-mvp.worker.js',
];

// Assets to cache on demand (runtime)
const CACHE_PATTERNS = [
  /^\/duckdb-wasm\/.*/, // WASM files
  /^\/data\/.*/,         // Question data
  /\.js$/,               // JavaScript bundles
  /\.css$/,              // Stylesheets
  /\.wasm$/,             // WASM files
  /\.json$/,             // JSON data
];

// Never cache these patterns
const BYPASS_PATTERNS = [
  /^\/api\//,            // API calls (if added later)
];

/**
 * Install event - cache core assets
 */
self.addEventListener('install', (event) => {
  console.log('[SW] Installing service worker...');

  event.waitUntil(
    (async () => {
      const cache = await caches.open(CACHE_NAME);
      console.log('[SW] Precaching assets:', PRECACHE_ASSETS);

      try {
        await cache.addAll(PRECACHE_ASSETS);
        console.log('[SW] Precache complete');
      } catch (error) {
        console.error('[SW] Precache failed:', error);
        // Don't fail install if some assets are missing
        // They'll be cached on first request
      }

      // Activate immediately
      await self.skipWaiting();
    })()
  );
});

/**
 * Activate event - clean up old caches
 */
self.addEventListener('activate', (event) => {
  console.log('[SW] Activating service worker...');

  event.waitUntil(
    (async () => {
      // Clean up old cache versions
      const cacheNames = await caches.keys();
      const deletions = cacheNames
        .filter((name) => name !== CACHE_NAME)
        .map((name) => {
          console.log('[SW] Deleting old cache:', name);
          return caches.delete(name);
        });

      await Promise.all(deletions);
      await self.clients.claim();
      console.log('[SW] Service worker activated');
    })()
  );
});

/**
 * Fetch event - serve from cache, fallback to network
 */
self.addEventListener('fetch', (event) => {
  const { request } = event;
  const url = new URL(request.url);

  // Skip for non-GET requests
  if (request.method !== 'GET') return;

  // Skip for chrome extensions, etc.
  if (!url.protocol.startsWith('http')) return;

  // Check if should bypass cache
  if (BYPASS_PATTERNS.some((pattern) => pattern.test(url.pathname))) {
    return;
  }

  // Check if should cache
  const shouldCache = CACHE_PATTERNS.some((pattern) => pattern.test(url.pathname));

  if (shouldCache) {
    // Cache-first strategy for WASM and data files
    event.respondWith(cacheFirst(request));
  } else if (url.pathname === '/' || url.pathname.endsWith('.html')) {
    // Network-first for HTML (always get latest)
    event.respondWith(networkFirst(request));
  } else {
    // Try cache first, fallback to network for other assets
    event.respondWith(cacheFirst(request));
  }
});

/**
 * Cache-first strategy
 * Try cache first, if miss, fetch from network and cache
 */
async function cacheFirst(request) {
  const cache = await caches.open(CACHE_NAME);
  const cached = await cache.match(request);

  if (cached) {
    console.log('[SW] Cache hit:', request.url);
    return cached;
  }

  console.log('[SW] Cache miss, fetching:', request.url);

  try {
    const response = await fetch(request);

    // Clone response before caching
    if (response.ok) {
      const clone = response.clone();
      cache.put(request, clone).catch((err) => {
        console.warn('[SW] Failed to cache:', request.url, err);
      });
    }

    return response;
  } catch (error) {
    console.error('[SW] Fetch failed:', request.url, error);

    // Return a custom offline page for navigation requests
    if (request.mode === 'navigate') {
      return caches.match('/index.html');
    }

    throw error;
  }
}

/**
 * Network-first strategy
 * Try network first, if fail, fallback to cache
 */
async function networkFirst(request) {
  const cache = await caches.open(CACHE_NAME);

  try {
    console.log('[SW] Network first:', request.url);
    const response = await fetch(request);

    // Update cache
    if (response.ok) {
      const clone = response.clone();
      cache.put(request, clone).catch(() => {});
    }

    return response;
  } catch (error) {
    console.log('[SW] Network failed, trying cache:', request.url);
    const cached = await cache.match(request);

    if (cached) {
      return cached;
    }

    // Return offline fallback for navigation
    if (request.mode === 'navigate') {
      return caches.match('/index.html');
    }

    throw error;
  }
}

/**
 * Message handler - allow manual cache control
 */
self.addEventListener('message', (event) => {
  const { action, url } = event.data;

  switch (action) {
    case 'skipWaiting':
      self.skipWaiting();
      break;

    case 'cache':
      // Manually cache a URL
      caches.open(CACHE_NAME).then((cache) => {
        fetch(url).then((response) => {
          cache.put(url, response);
        });
      });
      break;

    case 'clearCache':
      // Clear all caches
      caches.keys().then((keys) => {
        keys.forEach((key) => caches.delete(key));
      });
      break;
  }
});
