import React, { useEffect, useState, useCallback, useMemo } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import { api } from '../utils/api';
import { useAuth } from '../context/AuthContext';
import GardenPlantCard, {
  type GardenPlantCardSpecies,
} from '../components/GardenPlantCard';
import PlantCard from '../components/PlantCard';
import PlantDetailsModal from '../components/PlantDetailsModal';
import CreateGardenModal from '../components/CreateGardenModal';
import { FaSeedling, FaMapMarkerAlt, FaClock, FaGlobe, FaSearch, FaChartBar, FaExclamationTriangle, FaLeaf, FaTint, FaBookmark, FaPlus } from 'react-icons/fa';
import { mapPlantToVisualCategory } from '../utils/plantVisualCategory';

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
  creationTimestamp: string;
  species: GardenPlantCardSpecies;
  soil: { type: string };
}

interface GardenDetail extends GardenSummary {
  plants: GardenDetailPlant[];
}

interface SavedPlant {
  id: number;
  perenualId: number;
  commonName: string;
  scientificName: string;
  imgSrcUrls: { regular: string | null };
  family?: string;
  modelCategory?: string;
}

type GardenSubTab = 'plants' | 'saved';

const MyGardens: React.FC = () => {
  const navigate = useNavigate();
  const { user } = useAuth();
  const [gardens, setGardens] = useState<GardenSummary[]>([]);
  const [selectedId, setSelectedId] = useState<number | null>(null);
  const [detail, setDetail] = useState<GardenDetail | null>(null);
  const [listLoading, setListLoading] = useState(true);
  const [detailLoading, setDetailLoading] = useState(false);
  const [searchTerm, setSearchTerm] = useState('');
  const [showAnalytics, setShowAnalytics] = useState(false);
  const [subTab, setSubTab] = useState<GardenSubTab>('plants');
  const [gardenSavedPlants, setGardenSavedPlants] = useState<SavedPlant[]>([]);
  const [savedLoading, setSavedLoading] = useState(false);
  const [selectedSavedPlantId, setSelectedSavedPlantId] = useState<number | null>(null);
  const [showCreateModal, setShowCreateModal] = useState(false);
  const [globalSavedIds, setGlobalSavedIds] = useState<Set<number>>(new Set());
  const [savedPlantGardenStates, setSavedPlantGardenStates] = useState<Record<number, boolean>>({});

  const analytics = useMemo(() => {
    if (!detail) return null;
    const plants = detail.plants;
    const uniqueSpecies = new Set(plants.map(p => p.species.commonName.toLowerCase())).size;
    const categories = plants.reduce((acc: any, p) => {
      const cat = mapPlantToVisualCategory({
        type: p.species.type,
        flowers: p.species.flowers,
        cuisine: p.species.cuisine,
        edibleFruit: p.species.edibleFruit,
        edibleLeaf: p.species.edibleLeaf,
      });
      acc[cat] = (acc[cat] || 0) + 1;
      return acc;
    }, {});

    const threeDaysAgo = Date.now() - 3 * 24 * 3600 * 1000;
    const needsWater = plants.filter(p => {
      const last = p.lastWatered ? new Date(p.lastWatered).getTime() : 0;
      return last < threeDaysAgo || p.healthStatus === 'NeedsWater' || p.healthStatus === 'Wilting';
    });

    const speciesCounts = plants.reduce((acc: any, p) => {
      const name = p.species.commonName;
      acc[name] = (acc[name] || 0) + 1;
      return acc;
    }, {});

    return {
      total: plants.length,
      unique: uniqueSpecies,
      categories,
      thirsty: needsWater,
      speciesCounts
    };
  }, [detail]);

  const filteredPlants = useMemo(() => {
    if (!detail) return [];
    return detail.plants.filter(p => 
      p.species.commonName.toLowerCase().includes(searchTerm.toLowerCase()) ||
      p.species.scientificName.toLowerCase().includes(searchTerm.toLowerCase())
    );
  }, [detail, searchTerm]);

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
    setSubTab('plants'); // Reset sub-tab when switching gardens
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

  // Fetch saved plants for the current garden when "saved" sub-tab is active
  useEffect(() => {
    if (!user?.id || selectedId == null || subTab !== 'saved') {
      setGardenSavedPlants([]);
      return;
    }
    let cancelled = false;
    setSavedLoading(true);
    api
      .get(`/species/saved?userId=${user.id}&gardenId=${selectedId}`)
      .then((res: { data: SavedPlant[] }) => {
        if (!cancelled) setGardenSavedPlants(res.data);
      })
      .catch((e) => {
        console.error('Failed to load garden saved plants', e);
        if (!cancelled) setGardenSavedPlants([]);
      })
      .finally(() => {
        if (!cancelled) setSavedLoading(false);
      });
    return () => {
      cancelled = true;
    };
  }, [user?.id, selectedId, subTab]);

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

  // Fetch global saved IDs for the modal's global checkbox
  useEffect(() => {
    if (user) {
      api.get(`/species/saved?userId=${user.id}`)
        .then((res: { data: { perenualId: number }[] }) => {
          setGlobalSavedIds(new Set(res.data.map(s => s.perenualId)));
        })
        .catch(err => console.error('Failed to fetch global saved IDs', err));
    }
  }, [user]);

  // Fetch per-garden save states when a saved plant is selected
  useEffect(() => {
    if (!user || !selectedSavedPlantId || gardens.length === 0) {
      setSavedPlantGardenStates({});
      return;
    }
    const fetchStates = async () => {
      const states: Record<number, boolean> = {};
      await Promise.all(
        gardens.map(async (garden) => {
          try {
            const res = await api.get(`/species/saved?userId=${user.id}&gardenId=${garden.id}`);
            const ids: number[] = res.data.map((s: { perenualId: number }) => s.perenualId);
            states[garden.id] = ids.includes(selectedSavedPlantId);
          } catch {
            states[garden.id] = false;
          }
        })
      );
      setSavedPlantGardenStates(states);
    };
    fetchStates();
  }, [user, selectedSavedPlantId, gardens]);

  const handleSaveToDestinations = async (plantId: number, saveGlobal: boolean, gardenIds: number[]) => {
    if (!user) return;
    try {
      const wasGlobal = globalSavedIds.has(plantId);
      if (saveGlobal && !wasGlobal) {
        await api.post(`/species/save/${plantId}?userId=${user.id}`, {});
        setGlobalSavedIds(prev => { const next = new Set(prev); next.add(plantId); return next; });
      } else if (!saveGlobal && wasGlobal) {
        await api.del(`/species/save/${plantId}?userId=${user.id}`);
        setGlobalSavedIds(prev => { const next = new Set(prev); next.delete(plantId); return next; });
      }
      for (const garden of gardens) {
        const wasSaved = savedPlantGardenStates[garden.id] || false;
        const shouldSave = gardenIds.includes(garden.id);
        if (shouldSave && !wasSaved) {
          await api.post(`/species/save/${plantId}?userId=${user.id}&gardenId=${garden.id}`, {});
        } else if (!shouldSave && wasSaved) {
          await api.del(`/species/save/${plantId}?userId=${user.id}&gardenId=${garden.id}`);
        }
      }
      const newStates: Record<number, boolean> = {};
      gardens.forEach(g => { newStates[g.id] = gardenIds.includes(g.id); });
      setSavedPlantGardenStates(newStates);

      // Refresh saved plants for current garden tab
      if (selectedId != null) {
        api.get(`/species/saved?userId=${user.id}&gardenId=${selectedId}`)
          .then((res: { data: SavedPlant[] }) => setGardenSavedPlants(res.data))
          .catch(() => {});
      }
    } catch (err) {
      console.error('Failed to save to destinations', err);
    }
  };

  const handleUnsaveFromGarden = async (perenualId: number) => {
    if (!user || selectedId == null) return;
    try {
      await api.del(`/species/save/${perenualId}?userId=${user.id}&gardenId=${selectedId}`);
      setGardenSavedPlants(prev => prev.filter(p => p.perenualId !== perenualId));
      setSelectedSavedPlantId(null);
    } catch (err) {
      console.error("Failed to unsave from garden", err);
    }
  };

  const mapSavedToCardProps = (plant: SavedPlant) => ({
    id: plant.perenualId,
    common_name: plant.commonName,
    scientific_name: plant.scientificName,
    image_url: plant.imgSrcUrls?.regular || '',
    family_common_name: plant.family,
    modelCategory: plant.modelCategory
  });

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
            <button
              className="browse-back-btn"
              onClick={() => setShowCreateModal(true)}
              style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', borderColor: 'rgba(74, 222, 128, 0.5)', color: '#86efac' }}
            >
              <FaPlus /> Create Garden
            </button>
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
              Create a garden to start organizing and saving plants. You can also create gardens in <strong>Unreal Engine</strong> and sync them to your account.
            </p>
            <div style={{ display: 'flex', gap: '0.75rem', justifyContent: 'center', marginTop: '1.5rem', flexWrap: 'wrap' }}>
              <button 
                className="browse-back-btn" 
                onClick={() => setShowCreateModal(true)}
                style={{ borderColor: 'rgba(74, 222, 128, 0.5)', color: '#86efac', padding: '0.75rem 1.5rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}
              >
                <FaPlus /> Create Garden
              </button>
              <button 
                className="browse-back-btn" 
                onClick={() => navigate('/demo-garden')}
                style={{ padding: '0.75rem 1.5rem' }}
              >
                View Demo Garden
              </button>
            </div>
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
                    position: 'relative',
                    overflow: 'hidden'
                  }}
                >
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start' }}>
                    <div>
                      <h2 style={{ margin: '0 0 0.5rem', color: 'rgba(255,255,255,0.95)' }}>
                        {detail.name}
                      </h2>
                      {detail.description ? (
                        <p style={{ color: 'rgba(255,255,255,0.7)', margin: '0 0 1rem', lineHeight: 1.5, maxWidth: '800px' }}>
                          {detail.description}
                        </p>
                      ) : null}
                    </div>
                    {detail.plants.length > 0 && (
                      <button 
                        className={`browse-back-btn ${showAnalytics ? 'active' : ''}`} 
                        onClick={() => setShowAnalytics(!showAnalytics)}
                        style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', color: showAnalytics ? '#86efac' : undefined, borderColor: showAnalytics ? '#86efac' : undefined, backgroundColor: 'rgba(15, 23, 42, 0.4)' }}
                      >
                        <FaChartBar /> {showAnalytics ? 'Hide Analytics' : 'Garden Analytics'}
                      </button>
                    )}
                  </div>
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
                    {analytics && analytics.thirsty.length > 0 && (
                      <div style={{ display: 'flex', alignItems: 'center', gap: '0.35rem', color: '#fca5a5' }}>
                        <FaTint /> {analytics.thirsty.length} plants need attention
                      </div>
                    )}
                    {analytics && analytics.thirsty.length === 0 && detail.plants.length > 0 && (
                      <div style={{ display: 'flex', alignItems: 'center', gap: '0.35rem', color: '#86efac' }}>
                        <FaTint /> All plants recently watered
                      </div>
                    )}
                    <div style={{ opacity: 0.85 }}>Created {formatDt(detail.creationTimestamp)}</div>
                    <div style={{ fontWeight: 600, marginTop: '0.25rem' }}>
                      {detail.plants.length} plant{detail.plants.length === 1 ? '' : 's'} in this garden
                    </div>
                  </div>
                </section>

                {showAnalytics && analytics && (
                  <section
                    style={{
                      background: 'rgba(15, 23, 42, 0.6)',
                      border: '1px solid rgba(74, 222, 128, 0.3)',
                      borderRadius: '16px',
                      padding: '1.5rem',
                      display: 'grid',
                      gridTemplateColumns: 'repeat(auto-fit, minmax(280px, 1fr))',
                      gap: '1.5rem',
                      animation: 'slideUp 0.3s ease-out',
                      backdropFilter: 'blur(12px)',
                      marginBottom: '2rem'
                    }}
                  >
                    <div className="modal-state-box" style={{ background: 'rgba(255,255,255,0.03)', padding: '1rem', borderRadius: '12px' }}>
                      <h4 style={{ color: '#86efac', display: 'flex', alignItems: 'center', gap: '0.5rem', marginBottom: '1rem' }}>
                        <FaLeaf /> Species Diversity
                      </h4>
                      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                        <div>
                          <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: 'white' }}>{analytics.unique}</div>
                          <div style={{ fontSize: '0.8rem', color: 'rgba(255,255,255,0.6)' }}>Unique Species</div>
                        </div>
                        <div style={{ textAlign: 'right' }}>
                          <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: 'white' }}>{analytics.total}</div>
                          <div style={{ fontSize: '0.8rem', color: 'rgba(255,255,255,0.6)' }}>Total Plants</div>
                        </div>
                      </div>
                      <div style={{ marginTop: '1rem' }}>
                        {Object.entries(analytics.speciesCounts).map(([name, count]: [any, any]) => (
                          <div key={name} style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.85rem', padding: '0.2rem 0' }}>
                            <span style={{ color: 'rgba(255,255,255,0.8)' }}>{name}</span>
                            <span style={{ color: '#86efac', fontWeight: 'bold' }}>x{count}</span>
                          </div>
                        ))}
                      </div>
                    </div>

                    <div className="modal-state-box" style={{ background: 'rgba(255,255,255,0.03)', padding: '1rem', borderRadius: '12px' }}>
                      <h4 style={{ color: '#38bdf8', display: 'flex', alignItems: 'center', gap: '0.5rem', marginBottom: '1rem' }}>
                        <FaChartBar /> Category Breakdown
                      </h4>
                      <div style={{ display: 'flex', flexDirection: 'column', gap: '0.75rem' }}>
                        {Object.entries(analytics.categories).map(([cat, count]: [any, any]) => (
                          <div key={cat} style={{ width: '100%' }}>
                            <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.8rem', marginBottom: '0.25rem' }}>
                              <span style={{ textTransform: 'capitalize', color: 'white' }}>{cat}s</span>
                              <span>{count}</span>
                            </div>
                            <div style={{ height: '6px', background: 'rgba(255,255,255,0.1)', borderRadius: '3px', overflow: 'hidden' }}>
                              <div 
                                style={{ 
                                  height: '100%', 
                                  width: `${(count / analytics.total) * 100}%`, 
                                  background: cat === 'tree' ? '#10b981' : cat === 'flower' ? '#f472b6' : '#f59e0b' 
                                }} 
                              />
                            </div>
                          </div>
                        ))}
                      </div>
                    </div>

                    <div className="modal-state-box" style={{ background: 'rgba(255,255,255,0.03)', padding: '1rem', borderRadius: '12px' }}>
                      <h4 style={{ color: '#fca5a5', display: 'flex', alignItems: 'center', gap: '0.5rem', marginBottom: '1rem' }}>
                        <FaExclamationTriangle /> Needs Attention
                      </h4>
                      {analytics.thirsty.length > 0 ? (
                        <div style={{ display: 'flex', flexDirection: 'column', gap: '0.5rem' }}>
                          {analytics.thirsty.map((p: any) => (
                            <div key={p.id} style={{ padding: '0.5rem', background: 'rgba(239, 68, 68, 0.1)', borderLeft: '3px solid #ef4444', borderRadius: '4px' }}>
                              <div style={{ fontSize: '0.9rem', color: 'white', fontWeight: 'bold' }}>{p.species.commonName}</div>
                              <div style={{ fontSize: '0.75rem', color: '#fca5a5' }}>
                                Not watered in {Math.floor((Date.now() - new Date(p.lastWatered!).getTime()) / (1000 * 3600 * 24))} days
                              </div>
                            </div>
                          ))}
                        </div>
                      ) : (
                        <div style={{ textAlign: 'center', padding: '1rem', color: 'rgba(255,255,255,0.5)' }}>
                          <FaTint style={{ fontSize: '1.5rem', marginBottom: '0.5rem', color: '#86efac' }} />
                          <p>Your garden is perfectly hydrated!</p>
                        </div>
                      )}
                    </div>
                  </section>
                )}

                {/* Sub-tab bar: Plants | Saved Plants */}
                <div className="sub-tab-bar">
                  <button
                    className={`sub-tab ${subTab === 'plants' ? 'active' : ''}`}
                    onClick={() => setSubTab('plants')}
                  >
                    <FaSeedling /> Plants
                    <span style={{ opacity: 0.6, fontSize: '0.85rem' }}>({detail.plants.length})</span>
                  </button>
                  <button
                    className={`sub-tab ${subTab === 'saved' ? 'active' : ''}`}
                    onClick={() => setSubTab('saved')}
                  >
                    <FaBookmark /> Saved Plants
                    {subTab === 'saved' && gardenSavedPlants.length > 0 && (
                      <span style={{ opacity: 0.6, fontSize: '0.85rem' }}>({gardenSavedPlants.length})</span>
                    )}
                  </button>
                </div>

                {/* Plants sub-tab content */}
                {subTab === 'plants' && (
                  <>
                    {detail.plants.length === 0 ? (
                      <p style={{ color: 'rgba(255,255,255,0.5)' }}>
                        No plants in this garden yet. Add them from Unreal.
                      </p>
                    ) : (
                      <>
                        <div className="browse-search-bar" style={{ maxWidth: '100%', marginBottom: '1rem' }}>
                          <FaSearch className="browse-search-icon" />
                          <input
                            type="text"
                            className="browse-search-input"
                            placeholder="Search plants in your garden..."
                            value={searchTerm}
                            onChange={(e) => setSearchTerm(e.target.value)}
                          />
                        </div>

                        <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1rem' }}>
                          <h3 style={{ color: 'rgba(255,255,255,0.9)', fontSize: '1.2rem', margin: 0 }}>
                            Plants
                          </h3>
                          <span style={{ fontSize: '0.9rem', color: 'rgba(255,255,255,0.5)' }}>
                            Showing {filteredPlants.length} of {detail.plants.length}
                          </span>
                        </div>

                        {filteredPlants.length === 0 ? (
                          <div style={{ textAlign: 'center', padding: '4rem', color: 'rgba(255,255,255,0.4)' }}>
                            <FaSearch style={{ fontSize: '2rem', marginBottom: '1rem', opacity: 0.5 }} />
                            <p>No plants match your search "{searchTerm}"</p>
                          </div>
                        ) : (
                          <div className="browse-grid">
                            {filteredPlants.map((p) => (
                              <GardenPlantCard key={p.id} plant={{ ...p, plantedDate: p.creationTimestamp } as any} />
                            ))}
                          </div>
                        )}
                      </>
                    )}
                  </>
                )}

                {/* Saved Plants sub-tab content */}
                {subTab === 'saved' && (
                  <>
                    {savedLoading ? (
                      <div style={{ textAlign: 'center', color: 'rgba(255,255,255,0.6)', padding: '2rem' }}>
                        Loading saved plants for {detail.name}…
                      </div>
                    ) : gardenSavedPlants.length === 0 ? (
                      <div style={{ textAlign: 'center', color: 'rgba(255,255,255,0.5)', padding: '3rem' }}>
                        <FaBookmark style={{ fontSize: '2.5rem', marginBottom: '1rem', opacity: 0.4 }} />
                        <p>No plants saved to <strong>{detail.name}</strong> yet.</p>
                        <p style={{ fontSize: '0.85rem', marginTop: '0.5rem', opacity: 0.6 }}>
                          Browse species and save them to this garden so they're ready when you open Unreal.
                        </p>
                        <button 
                          className="browse-chip active"
                          style={{ marginTop: '1rem', display: 'inline-flex', alignItems: 'center', gap: '0.5rem' }}
                          onClick={() => navigate('/browse')}
                        >
                          <FaSearch /> Browse Species
                        </button>
                      </div>
                    ) : (
                      <div className="browse-grid">
                        {gardenSavedPlants.map((plant) => (
                          <PlantCard
                            key={plant.id}
                            plant={mapSavedToCardProps(plant)}
                            onClick={() => setSelectedSavedPlantId(plant.perenualId)}
                          />
                        ))}
                      </div>
                    )}
                  </>
                )}
              </>
            )}
          </>
        )}
      </div>

      {/* Details Modal for garden saved plants — with manage saves */}
      <PlantDetailsModal
        isOpen={!!selectedSavedPlantId}
        plantId={selectedSavedPlantId!}
        onClose={() => setSelectedSavedPlantId(null)}
        isSaved={selectedSavedPlantId ? globalSavedIds.has(selectedSavedPlantId) : false}
        onToggleSave={() => selectedSavedPlantId && handleUnsaveFromGarden(selectedSavedPlantId)}
        gardens={gardens}
        gardenSaveStates={savedPlantGardenStates}
        onSaveToDestinations={(saveGlobal, gardenIds) =>
          selectedSavedPlantId && handleSaveToDestinations(selectedSavedPlantId, saveGlobal, gardenIds)
        }
      />

      {/* Create Garden Modal */}
      {user && (
        <CreateGardenModal
          isOpen={showCreateModal}
          onClose={() => setShowCreateModal(false)}
          userId={user.id}
          onCreated={loadList}
        />
      )}
    </div>
  );
};

export default MyGardens;
