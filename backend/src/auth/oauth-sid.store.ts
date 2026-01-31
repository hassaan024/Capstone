type SidEntry = { returnPort: number; createdAt: number };

const SID_TTL_MS = 2 * 60 * 1000; // 2 minutes
const sidStore = new Map<string, SidEntry>();

export function putSid(sid: string, returnPort: number) {
  sidStore.set(sid, { returnPort, createdAt: Date.now() });
}

export function getSid(sid: string): SidEntry | null {
  const entry = sidStore.get(sid);
  if (!entry) return null;

  if (Date.now() - entry.createdAt > SID_TTL_MS) {
    sidStore.delete(sid);
    return null;
  }
  return entry;
}

export function deleteSid(sid: string) {
  sidStore.delete(sid);
}