import React, { useEffect, useState, useCallback } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import { api } from '../utils/api';
import { useAuth } from '../context/AuthContext';
import GardenPlantCard, {
  type GardenPlantCardSpecies,
} from '../components/GardenPlantCard';
import { FaSeedling, FaMapMarkerAlt, FaClock, FaGlobe } from 'react-icons/fa';

interface GardenSummary {
  id: number;
  name: string;
  description: string | null;
  latitude: number;
  longitude: number;
  timezone: string | null;
  creationTimestamp: string;
  lastUpdated: string;
  _count: { plants: number };
}

interface GardenDetailPlant {
  id: number;
  heightCm: number | null;
  ageDays: number | null;
  healthStatus: string | null;
  lastWatered: string | null;
  notes: string | null;
  species: GardenPlantCardSpecies;
  soil: { type: string };
}

interface GardenDetail extends GardenSummary {
  plants: GardenDetailPlant[];
}

const MyGardens: React.FC = () => {
  const navigate = useNavigate();
  const { user } = useAuth();
  const [gardens, setGardens] = useState<GardenSummary[]>([]);
  const [selectedId, setSelectedId] = useState<number | null>(null);
  const [detail, setDetail] = useState<GardenDetail | null>(null);
  const [listLoading, setListLoading] = useState(true);
  const [detailLoading, setDetailLoading] = useState(false);

  const loadList = useCallback(async () => {
    if (!user?.id) return;
    setListLoading(true);
    try {
      const res = await api.get(`/garden/by-user/${user.id}`);
      const list: GardenSummary[] = res.data;
      setGardens(list);
      if (list.length > 0) {
        setSelectedId((prev) => (prev != null && list.some((g) => g.id === prev) ? prev : list[0].id));
      } else {
        setSelectedId(null);
        setDetail(null);
      }
    } catch (e) {
      console.error('Failed to load gardens', e);
      setGardens([]);
    } finally {
      setListLoading(false);
    }
  }, [user?.id]);

  useEffect(() => {
    if (!user) {
      navigate('/login');
      return;
    }
    loadList();
  }, [user, navigate, loadList]);

  useEffect(() => {
    if (!user?.id || selectedId == null) {
      setDetail(null);
      return;
    }
    let cancelled = false;
    setDetailLoading(true);
    api
      .get(`/garden/${selectedId}?userId=${user.id}`)
      .then((res: { data: GardenDetail }) => {
        if (!cancelled) setDetail(res.data);
      })
      .catch((e) => {
        console.error('Failed to load garden detail', e);
        if (!cancelled) setDetail(null);
      })
      .finally(() => {
        if (!cancelled) setDetailLoading(false);
      });
    return () => {
      cancelled = true;
    };
  }, [user?.id, selectedId]);

  useEffect(() => {
    if (!localStorage.getItem('hasSeenGardensPopup')) {
      const t = setTimeout(() => {
        window.dispatchEvent(
          new CustomEvent('suggestChat', { detail: 'What shows up on My Gardens?' }),
        );
        localStorage.setItem('hasSeenGardensPopup', 'true');
      }, 2000);
      return () => clearTimeout(t);
    }
  }, []);

  const formatDt = (iso: string) => {
    try {
      return new Date(iso).toLocaleString(undefined, { dateStyle: 'medium', timeStyle: 'short' });
    } catch {
      return iso;
    }
  };

  const mapsHref =
    detail != null
      ? `https://www.google.com/maps?q=${detail.latitude},${detail.longitude}`
      : '#';

  return (
    <div className="browse-root">
      <div className="browse-background-gradient" />
      <div className="browse-container">
        <header className="browse-header">
          <div className="browse-title">
            <span
              style={{
                fontSize: '1.2rem',
                display: 'flex',
                alignItems: 'center',
                marginRight: '0.5rem',
              }}
            >
              <FaSeedling />
            </span>
            My Gardens
          </div>
          <div style={{ display: 'flex', gap: '0.75rem' }}>
            <button className="browse-back-btn" onClick={() => navigate('/dashboard')}>
              Back to Dashboard
            </button>
          </div>
        </header>

        {listLoading ? (
          <div style={{ textAlign: 'center', color: 'rgba(255,255,255,0.7)', marginTop: '2rem' }}>
            Loading your gardens…
          </div>
        ) : gardens.length === 0 ? (
          <div
            style={{
              gridColumn: '1 / -1',
              textAlign: 'center',
              color: 'rgba(255,255,255,0.6)',
              padding: '3rem 1.5rem',
              maxWidth: '32rem',
              margin: '0 auto',
            }}
          >
            <FaSeedling style={{ fontSize: '3rem', marginBottom: '1rem', opacity: 0.5 }} />
            <p style={{ fontSize: '1.05rem', marginBottom: '0.75rem' }}>No gardens yet</p>
            <p style={{ lineHeight: 1.6 }}>
              Gardens are created in <strong>Unreal Engine</strong>. When you build a garden and sync
              it to your account, it will show up here with every plant you have placed.
            </p>
            <button 
              className="browse-back-btn" 
              onClick={() => navigate('/demo-garden')}
              style={{ marginTop: '1.5rem', borderColor: 'rgba(74, 222, 128, 0.5)', color: '#86efac', padding: '0.75rem 1.5rem' }}
            >
              View Demo Garden Visualization
            </button>
          </div>
        ) : (
          <>
            <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.5rem', marginBottom: '1.25rem' }}>
              {gardens.map((g) => (
                <button
                  key={g.id}
                  type="button"
                  className={`browse-chip ${selectedId === g.id ? 'active' : ''}`}
                  onClick={() => setSelectedId(g.id)}
                >
                  {g.name}
                  <span style={{ opacity: 0.7, marginLeft: '0.35rem' }}>({g._count.plants})</span>
                </button>
              ))}
            </div>

            {detailLoading && (
              <div style={{ color: 'rgba(255,255,255,0.6)', marginBottom: '1rem' }}>
                Loading garden…
              </div>
            )}

            {detail && !detailLoading && (
              <>
                <section
                  style={{
                    background: 'rgba(30, 41, 59, 0.55)',
                    border: '1px solid rgba(148, 163, 184, 0.2)',
                    borderRadius: '12px',
                    padding: '1.25rem 1.5rem',
                    marginBottom: '1.75rem',
                  }}
                >
                  <h2 style={{ margin: '0 0 0.5rem', color: 'rgba(255,255,255,0.95)' }}>
                    {detail.name}
                  </h2>
                  {detail.description ? (
                    <p style={{ color: 'rgba(255,255,255,0.7)', margin: '0 0 1rem', lineHeight: 1.5 }}>
                      {detail.description}
                    </p>
                  ) : null}
                  <div
                    style={{
                      display: 'grid',
                      gap: '0.65rem',
                      fontSize: '0.9rem',
                      color: 'rgba(255,255,255,0.75)',
                    }}
                  >
                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', flexWrap: 'wrap' }}>
                      <FaMapMarkerAlt />
                      <span>
                        {detail.latitude.toFixed(4)}, {detail.longitude.toFixed(4)}
                      </span>
                      <a
                        href={mapsHref}
                        target="_blank"
                        rel="noopener noreferrer"
                        style={{ color: '#7dd3fc', marginLeft: '0.25rem' }}
                      >
                        Open in Maps
                      </a>
                    </div>
                    {detail.timezone && (
                      <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                        <FaGlobe /> {detail.timezone}
                      </div>
                    )}
                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                      <FaClock /> Updated {formatDt(detail.lastUpdated)}
                    </div>
                    <div style={{ opacity: 0.85 }}>Created {formatDt(detail.creationTimestamp)}</div>
                    <div style={{ fontWeight: 600, marginTop: '0.25rem' }}>
                      {detail.plants.length} plant{detail.plants.length === 1 ? '' : 's'} in this garden
                    </div>
                  </div>
                </section>

                <h3
                  style={{
                    color: 'rgba(255,255,255,0.9)',
                    fontSize: '1.1rem',
                    marginBottom: '1rem',
                  }}
                >
                  Plants
                </h3>
                {detail.plants.length === 0 ? (
                  <p style={{ color: 'rgba(255,255,255,0.5)' }}>
                    No plants in this garden yet. Add them from Unreal.
                  </p>
                ) : (
                  <div className="browse-grid">
                    {detail.plants.map((p) => (
                      <GardenPlantCard key={p.id} plant={p} />
                    ))}
                  </div>
                )}
              </>
            )}
          </>
        )}
      </div>
    </div>
  );
};

export default MyGardens;
