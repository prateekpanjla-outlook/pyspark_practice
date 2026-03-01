/**
 * useServiceWorker Hook
 * Registers and manages the service worker for offline support
 */

import { useEffect, useState } from 'react';

interface ServiceWorkerState {
  isSupported: boolean;
  isRegistered: boolean;
  isActivated: boolean;
  isOffline: boolean;
  updateAvailable: boolean;
}

export function useServiceWorker() {
  const [state, setState] = useState<ServiceWorkerState>({
    isSupported: 'serviceWorker' in navigator,
    isRegistered: false,
    isActivated: false,
    isOffline: !navigator.onLine,
    updateAvailable: false,
  });

  useEffect(() => {
    if (!state.isSupported) return;

    // Register service worker
    navigator.serviceWorker
      .register('/sw.js', {
        type: 'classic',
      })
      .then((registration) => {
        console.log('[SW] Service worker registered:', registration);

        setState((prev) => ({ ...prev, isRegistered: true }));

        // Check for updates
        registration.addEventListener('updatefound', () => {
          const newWorker = registration.installing;
          if (newWorker) {
            newWorker.addEventListener('statechange', () => {
              if (newWorker.state === 'installed' && navigator.serviceWorker.controller) {
                console.log('[SW] New version available');
                setState((prev) => ({ ...prev, updateAvailable: true }));
              }
            });
          }
        });

        // Claim clients for immediate activation
        if (registration.active) {
          setState((prev) => ({ ...prev, isActivated: true }));
        } else {
          registration.addEventListener('controllerchange', () => {
            setState((prev) => ({ ...prev, isActivated: true }));
          });
        }
      })
      .catch((error) => {
        console.error('[SW] Registration failed:', error);
      });

    // Listen for online/offline events
    const handleOnline = () => setState((prev) => ({ ...prev, isOffline: false }));
    const handleOffline = () => setState((prev) => ({ ...prev, isOffline: true }));

    window.addEventListener('online', handleOnline);
    window.addEventListener('offline', handleOffline);

    return () => {
      window.removeEventListener('online', handleOnline);
      window.removeEventListener('offline', handleOffline);
    };
  }, [state.isSupported]);

  /**
   * Skip waiting and activate new service worker
   */
  const activateUpdate = () => {
    if (!state.isSupported || !state.updateAvailable) return;

    navigator.serviceWorker.getRegistration().then((registration) => {
      registration?.waiting?.postMessage({ type: 'SKIP_WAITING' });
      // Reload to activate new version
      window.location.reload();
    });
  };

  /**
   * Manually check for updates
   */
  const checkForUpdates = async () => {
    if (!state.isSupported) return;

    const registration = await navigator.serviceWorker.getRegistration();
    await registration?.update();
  };

  return {
    ...state,
    activateUpdate,
    checkForUpdates,
  };
}
