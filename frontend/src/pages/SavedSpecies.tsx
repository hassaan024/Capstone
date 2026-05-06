import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import { api } from '../utils/api';
import PlantCard from '../components/PlantCard';
import PlantDetailsModal from '../components/PlantDetailsModal';
import { useAuth } from '../context/AuthContext';
import { FaLeaf, FaBookmark, FaSearch, FaGlobeAmericas, FaSeedling } from 'react-icons/fa';

interface SavedPlant {
  id: number;
  perenualId: number;
  commonName: string;
  scientificName: string;
  imgSrcUrls: { regular: string | null };
  family?: string;
  modelCategory?: string;
  bloomDays?: number;
}

interface GardenSummary {
  id: number;
  name: string;
  _count: { plants: number };
}

type ActiveTab = 'global' | number; // 'global' or gardenId

const SavedSpecies: React.FC = () => {
  const navigate = useNavigate();
  const { user } = useAuth();
  const [savedPlants, setSavedPlants] = useState<SavedPlant[]>([]);
  const [loading, setLoading] = useState(true);
  const [selectedPlantId, setSelectedPlantId] = useState<number | null>(null);
  const [gardens, setGardens] = useState<GardenSummary[]>([]);
  const [activeTab, setActiveTab] = useState<ActiveTab>('global');
  const [globalSavedIds, setGlobalSavedIds] = useState<Set<number>>(new Set());
  const [gardenSaveStates, setGardenSaveStates] = useState<Record<number, boolean>>({});

  // Suggest Chatbot interaction
  useEffect(() => {
    if (user?.pageInfoRecommendations !== false) {
      const timer = setTimeout(() => {
        const event = new CustomEvent('suggestChat', { detail: 'What can I do on the Saved Species page?' });
        window.dispatchEvent(event);
      }, 2000);
      return () => clearTimeout(timer);
    }
  }, [user?.pageInfoRecommendations]);

  // Fetch gardens
  useEffect(() => {
    if (user) {
      api.get(`/garden/by-user/${user.id}`)
        .then((res: { data: GardenSummary[] }) => setGardens(res.data))
        .catch(err => console.error("Failed to fetch gardens", err));
    }
  }, [user]);

  // Fetch saved plants for active tab
  useEffect(() => {
    if (user) {
      fetchSavedPlants();
    }
  }, [user, activeTab]);

  // Fetch global saved IDs (always needed for the modal's global checkbox)
  useEffect(() => {
    if (user) {
      api.get(`/species/saved?userId=${user.id}`)
        .then((res: { data: { perenualId: number }[] }) => {
          setGlobalSavedIds(new Set(res.data.map(s => s.perenualId)));
        })
        .catch(err => console.error("Failed to fetch global saved IDs", err));
    }
  }, [user]);

  const fetchSavedPlants = async () => {
    setLoading(true);
    try {
      const url = activeTab === 'global'
        ? `/species/saved?userId=${user?.id}`
        : `/species/saved?userId=${user?.id}&gardenId=${activeTab}`;
      const res = await api.get(url);
      setSavedPlants(res.data);
    } catch (err) {
      console.error("Failed to fetch saved species", err);
    } finally {
      setLoading(false);
    }
  };

  // When a plant is selected, fetch per-garden save states for the modal
  useEffect(() => {
    if (!user || !selectedPlantId || gardens.length === 0) {
      setGardenSaveStates({});
      return;
    }

    const fetchGardenStates = async () => {
      const states: Record<number, boolean> = {};
      await Promise.all(
        gardens.map(async (garden) => {
          try {
            const res = await api.get(`/species/saved?userId=${user.id}&gardenId=${garden.id}`);
            const ids: number[] = res.data.map((s: { perenualId: number }) => s.perenualId);
            states[garden.id] = ids.includes(selectedPlantId);
          } catch {
            states[garden.id] = false;
          }
        })
      );
      setGardenSaveStates(states);
    };

    fetchGardenStates();
  }, [user, selectedPlantId, gardens]);

  const handleSaveToDestinations = async (plantId: number, saveGlobal: boolean, gardenIds: number[]) => {
    if (!user) return;

    try {
      // Handle global save/unsave
      const wasGlobal = globalSavedIds.has(plantId);
      if (saveGlobal && !wasGlobal) {
        await api.post(`/species/save/${plantId}?userId=${user.id}`, {});
        setGlobalSavedIds(prev => { const next = new Set(prev); next.add(plantId); return next; });
      } else if (!saveGlobal && wasGlobal) {
        await api.del(`/species/save/${plantId}?userId=${user.id}`);
        setGlobalSavedIds(prev => { const next = new Set(prev); next.delete(plantId); return next; });
      }

      // Handle per-garden save/unsave
      for (const garden of gardens) {
        const wasSaved = gardenSaveStates[garden.id] || false;
        const shouldSave = gardenIds.includes(garden.id);

        if (shouldSave && !wasSaved) {
          await api.post(`/species/save/${plantId}?userId=${user.id}&gardenId=${garden.id}`, {});
        } else if (!shouldSave && wasSaved) {
          await api.del(`/species/save/${plantId}?userId=${user.id}&gardenId=${garden.id}`);
        }
      }

      // Update garden save states locally
      const newStates: Record<number, boolean> = {};
      gardens.forEach(g => {
        newStates[g.id] = gardenIds.includes(g.id);
      });
      setGardenSaveStates(newStates);

      // Refresh the current tab's list to reflect changes
      fetchSavedPlants();
    } catch (err) {
      console.error("Failed to save to destinations", err);
    }
  };

  // Map SavedPlant (backend) to PlantCard props
  const mapToCardProps = (plant: SavedPlant) => ({
    id: plant.perenualId,
    common_name: plant.commonName,
    scientific_name: plant.scientificName,
    image_url: plant.imgSrcUrls?.regular || '',
    family_common_name: plant.family,
    modelCategory: plant.modelCategory,
    bloomDays: plant.bloomDays,
  });

  const activeGardenName = activeTab === 'global'
    ? 'All Saved'
    : gardens.find(g => g.id === activeTab)?.name || 'Garden';

  return (
    <div className="browse-root">
      <div className="browse-background-gradient"></div>
      
      <div className="browse-container">
        {/* Header */}
        <header className="browse-header">
          <div className="browse-title">
            <span style={{ fontSize: '1.2rem', display: 'flex', alignItems: 'center', marginRight: '0.5rem' }}><FaBookmark /></span>
            Saved Plants
          </div>
          <button className="browse-back-btn" onClick={() => navigate('/dashboard')}>
            Back to Dashboard
          </button>
        </header>

        {/* Tab Bar */}
        <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.5rem', alignItems: 'center' }}>
          <button
            className={`browse-chip ${activeTab === 'global' ? 'active' : ''}`}
            onClick={() => setActiveTab('global')}
            style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
          >
            <FaGlobeAmericas /> All Saved
          </button>

          {gardens.map(garden => (
            <button
              key={garden.id}
              className={`browse-chip ${activeTab === garden.id ? 'active' : ''}`}
              onClick={() => setActiveTab(garden.id)}
              style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
            >
              <FaSeedling /> {garden.name}
            </button>
          ))}

          <div style={{ marginLeft: 'auto' }}>
            <button 
              className="browse-chip active"
              style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
              onClick={() => navigate('/browse')}
            >
              <FaSearch /> Save more plants
            </button>
          </div>
        </div>

        {/* Grid */}
        {loading ? (
          <div style={{ textAlign: 'center', color: 'rgba(255,255,255,0.7)', marginTop: '2rem' }}>
            Loading your {activeTab === 'global' ? 'saved plants' : `plants for ${activeGardenName}`}...
          </div>
        ) : (
          <div className="browse-grid">
            {savedPlants.length > 0 ? (
                savedPlants.map(plant => (
                 <PlantCard 
                    key={plant.id} 
                    plant={mapToCardProps(plant)} 
                    onClick={() => setSelectedPlantId(plant.perenualId)}
                />
                ))
            ) : (
                <div style={{ gridColumn: '1 / -1', textAlign: 'center', color: 'rgba(255,255,255,0.5)', padding: '3rem' }}>
                  <FaLeaf style={{ fontSize: '3rem', marginBottom: '1rem', opacity: 0.5 }} />
                  {activeTab === 'global' ? (
                    <>
                      <p>You haven't saved any plants yet.</p>
                      <button 
                        className="ll-btn ll-btn-primary" 
                        style={{ marginTop: '1rem' }}
                        onClick={() => navigate('/browse')}
                      >
                        Browse Species
                      </button>
                    </>
                  ) : (
                    <>
                      <p>No plants saved to <strong>{activeGardenName}</strong> yet.</p>
                      <p style={{ fontSize: '0.9rem', marginTop: '0.5rem', opacity: 0.7 }}>
                        Browse species and save them to this garden to see them here.
                      </p>
                      <button 
                        className="ll-btn ll-btn-primary" 
                        style={{ marginTop: '1rem' }}
                        onClick={() => navigate('/browse')}
                      >
                        Browse Species
                      </button>
                    </>
                  )}
                </div>
            )}
          </div>
        )}
      </div>

      {/* Details Modal — with full garden-aware save management */}
      <PlantDetailsModal
        isOpen={!!selectedPlantId}
        plantId={selectedPlantId!}
        onClose={() => setSelectedPlantId(null)}
        bloomDays={savedPlants.find(p => p.perenualId === selectedPlantId)?.bloomDays}
        isSaved={selectedPlantId ? globalSavedIds.has(selectedPlantId) : false}
        onToggleSave={() => {
          if (!selectedPlantId || !user) return;
          const isGlobal = globalSavedIds.has(selectedPlantId);
          if (isGlobal) {
            api.del(`/species/save/${selectedPlantId}?userId=${user.id}`)
              .then(() => {
                setGlobalSavedIds(prev => { const next = new Set(prev); next.delete(selectedPlantId); return next; });
                fetchSavedPlants();
                setSelectedPlantId(null);
              })
              .catch(err => console.error("Failed to unsave", err));
          } else {
            api.post(`/species/save/${selectedPlantId}?userId=${user.id}`, {})
              .then(() => {
                setGlobalSavedIds(prev => { const next = new Set(prev); next.add(selectedPlantId); return next; });
                fetchSavedPlants();
              })
              .catch(err => console.error("Failed to save", err));
          }
        }}
        gardens={gardens}
        gardenSaveStates={gardenSaveStates}
        onSaveToDestinations={(saveGlobal, gardenIds) =>
          selectedPlantId && handleSaveToDestinations(selectedPlantId, saveGlobal, gardenIds)
        }
      />
    </div>
  );
};

export default SavedSpecies;
