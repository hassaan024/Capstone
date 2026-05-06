import React, { useState, useEffect, useRef } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import { api } from '../utils/api';
import PlantCard from '../components/PlantCard';
import PlantDetailsModal from '../components/PlantDetailsModal';
import { useAuth } from '../context/AuthContext';
import { useToast } from '../context/ToastContext';

import { FaLeaf, FaSearch } from 'react-icons/fa';

interface GardenSummary {
  id: number;
  name: string;
}

const BrowseSpecies: React.FC = () => {
  const navigate = useNavigate();
  const [query, setQuery] = useState('');
  const [results, setResults] = useState<{ id: number; common_name: string; scientific_name: string; image_url?: string }[]>([]);
  const [loading, setLoading] = useState(false);
  const [isTyping, setIsTyping] = useState(false);
  const [selectedPlantId, setSelectedPlantId] = useState<number | null>(null);
  const [savedPlantIds, setSavedPlantIds] = useState<Set<number>>(new Set());
  const [gardens, setGardens] = useState<GardenSummary[]>([]);
  const [gardenSaveStates, setGardenSaveStates] = useState<Record<number, boolean>>({});
  const debounceRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const { user } = useAuth();
  const { toast } = useToast();

  // Debounced search-as-you-type
  useEffect(() => {
    if (debounceRef.current) clearTimeout(debounceRef.current);

    if (!query.trim()) {
      setResults([]);
      setIsTyping(false);
      return;
    }

    setIsTyping(true);
    debounceRef.current = setTimeout(() => {
      setIsTyping(false);
      handleSearch(query);
    }, 450);

    return () => {
      if (debounceRef.current) clearTimeout(debounceRef.current);
    };
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [query]);

  // Suggest Chatbot interaction
  React.useEffect(() => {
    if (user?.pageInfoRecommendations !== false) {
      const timer = setTimeout(() => {
        const event = new CustomEvent('suggestChat', { detail: 'What can I do on the Browse Species page?' });
        window.dispatchEvent(event);
      }, 2000);
      return () => clearTimeout(timer);
    }
  }, [user?.pageInfoRecommendations]);

  // Fetch saved species on mount
  React.useEffect(() => {
    if (user) {
      api.get(`/species/saved?userId=${user.id}`)
        .then((res: { data: { perenualId: number }[] }) => {
           const ids = new Set(res.data.map(s => s.perenualId));
           setSavedPlantIds(ids);
        })
        .catch(err => console.error("Failed to fetch saved species", err));
    }
  }, [user]);

  // Fetch user's gardens on mount
  React.useEffect(() => {
    if (user) {
      api.get(`/garden/by-user/${user.id}`)
        .then((res: { data: GardenSummary[] }) => {
          setGardens(res.data);
        })
        .catch(err => console.error("Failed to fetch gardens", err));
    }
  }, [user]);

  // When a plant is selected, fetch garden-level save states
  React.useEffect(() => {
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

  const handleToggleSave = async (plantId: number) => {
    if (!user) {
      toast('Please log in to save plants.', 'warning');
      return;
    }

    const isSaved = savedPlantIds.has(plantId);
    const plantName = results.find(r => r.id === plantId)?.common_name || 'Plant';

    try {
      if (isSaved) {
        await api.del(`/species/save/${plantId}?userId=${user.id}`);
        setSavedPlantIds(prev => { const next = new Set(prev); next.delete(plantId); return next; });
        toast(`Removed ${plantName} from your saved plants.`, 'info');
      } else {
        await api.post(`/species/save/${plantId}?userId=${user.id}`, {});
        setSavedPlantIds(prev => { const next = new Set(prev); next.add(plantId); return next; });
        toast(`${plantName} saved to your collection!`, 'success');
      }
    } catch (err) {
      console.error('Failed to toggle save', err);
      toast('Something went wrong. Please try again.', 'error');
    }
  };

  const handleSaveToDestinations = async (plantId: number, saveGlobal: boolean, gardenIds: number[]) => {
    if (!user) return;

    const plantName = results.find(r => r.id === plantId)?.common_name || 'Plant';

    try {
      // Handle global save/unsave
      const wasGlobal = savedPlantIds.has(plantId);
      if (saveGlobal && !wasGlobal) {
        await api.post(`/species/save/${plantId}?userId=${user.id}`, {});
        setSavedPlantIds(prev => { const next = new Set(prev); next.add(plantId); return next; });
      } else if (!saveGlobal && wasGlobal) {
        await api.del(`/species/save/${plantId}?userId=${user.id}`);
        setSavedPlantIds(prev => { const next = new Set(prev); next.delete(plantId); return next; });
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

      // Update garden save states
      const newStates: Record<number, boolean> = {};
      gardens.forEach(g => {
        newStates[g.id] = gardenIds.includes(g.id);
      });
      setGardenSaveStates(newStates);

      const gardenNames = gardenIds.map(id => gardens.find(g => g.id === id)?.name ?? 'a garden');
      const gardenLabel = gardenNames.length === 0
        ? ''
        : gardenNames.length === 1
          ? gardenNames[0]
          : gardenNames.length === 2
            ? `${gardenNames[0]} and ${gardenNames[1]}`
            : `${gardenNames[0]}, ${gardenNames[1]} and ${gardenNames.length - 2} more`;

      if (saveGlobal && gardenNames.length > 0) {
        toast(`${plantName} saved to your collection and ${gardenLabel}!`, 'success');
      } else if (saveGlobal) {
        toast(`${plantName} saved to your collection!`, 'success');
      } else if (gardenNames.length > 0) {
        toast(`${plantName} saved to ${gardenLabel}!`, 'success');
      } else {
        toast(`Removed ${plantName} from saved locations.`, 'info');
      }
    } catch (err) {
      console.error('Failed to save to destinations', err);
      toast('Something went wrong. Please try again.', 'error');
    }
  };


  const handleSearch = async (e: React.FormEvent | string) => {
    if (typeof e !== 'string') {
      e.preventDefault();
      // Explicit submit: cancel pending debounce and run immediately
      if (debounceRef.current) clearTimeout(debounceRef.current);
      setIsTyping(false);
    }
    const q = typeof e === 'string' ? e : query;
    if (!q.trim()) return;

    setLoading(true);
    setResults([]); // Clear previous
    try {
      const { data } = await api.get(`/perenual/search?query=${encodeURIComponent(q)}`);
      // Perenual returns { data: [...] } structure
      if (Array.isArray(data.data)) {
        // Filter out poor quality data or missing required fields
        const validPlants = data.data.filter((p: any) => 
          p.common_name && 
          p.scientific_name && 
          p.scientific_name.length > 0 && 
          p.default_image && 
          (p.default_image.regular_url || p.default_image.original_url)
        );

        const mappedResults = validPlants.map((p: any) => ({
          id: p.id,
          common_name: p.common_name,
          scientific_name: Array.isArray(p.scientific_name) ? p.scientific_name[0] : p.scientific_name,
          image_url: p.default_image?.regular_url || p.default_image?.original_url,
          modelCategory: p.modelCategory,
          bloomDays: p.bloomDays,
        }));
        setResults(mappedResults);
      } else {
        setResults([]);
      }
    } catch (err) {
      console.error('Search failed', err);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="browse-root">
      <div className="browse-background-gradient"></div>
      
      <div className="browse-container">
        {/* Header */}
        <header className="browse-header">
          <div className="browse-title">
            <span style={{ fontSize: '1.2rem', display: 'flex', alignItems: 'center', marginRight: '0.5rem' }}><FaLeaf /></span>
            Browse Plants
          </div>
          <button className="browse-back-btn" onClick={() => navigate('/dashboard')}>
            Back to Dashboard
          </button>
        </header>



        {/* Search & Filter */}
        <section className="browse-search-section">
          <form className="browse-search-bar" onSubmit={handleSearch}>
            <span className="browse-search-icon"><FaSearch /></span>
            <input 
              type="text" 
              className="browse-search-input" 
              placeholder="Search for a plant (e.g. Tomato, Rose)..."
              value={query}
              onChange={(e) => setQuery(e.target.value)}
            />
          </form>
        </section>

        {/* Results */}

        {/* Active query label */}
        {query.trim() && !loading && !isTyping && results.length > 0 && (
          <div style={{ marginBottom: '0.75rem', fontSize: '0.85rem', color: 'rgba(255,255,255,0.4)' }}>
            Results for <span style={{ color: 'rgba(134,239,172,0.8)', fontStyle: 'italic' }}>"{query.trim()}"</span>
          </div>
        )}

        {loading || isTyping ? (
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', gap: '0.65rem', color: 'rgba(255,255,255,0.55)', marginTop: '2.5rem' }}>
            <span style={{
              display: 'inline-block',
              width: '14px',
              height: '14px',
              border: '2px solid rgba(255,255,255,0.15)',
              borderTopColor: '#86efac',
              borderRadius: '50%',
              animation: 'browse-spin 0.7s linear infinite',
              flexShrink: 0,
            }} />
            {isTyping ? 'Waiting…' : 'Searching the botanical database…'}
          </div>
        ) : (
          <div className="browse-grid">
            {results.map(plant => (
              <PlantCard 
                key={plant.id} 
                plant={plant} 
                onClick={() => setSelectedPlantId(plant.id)}
              />
            ))}
            {results.length === 0 && query && (
               <div style={{ gridColumn: '1 / -1', textAlign: 'center', color: 'rgba(255,255,255,0.5)', padding: '2rem 0' }}>
                 No plants found for <em>"{query}"</em>. Try a different search term.
               </div>
            )}
            {results.length === 0 && !query && (
              <div style={{ gridColumn: '1 / -1', textAlign: 'center', padding: '3rem 0', color: 'rgba(255,255,255,0.3)' }}>
                <span style={{ fontSize: '2.5rem', display: 'block', marginBottom: '0.75rem' }}>🌿</span>
                Start typing a plant name to search the botanical database.
              </div>
            )}
          </div>
        )}
      </div>

      {/* Details Modal */}
      <PlantDetailsModal 
        isOpen={!!selectedPlantId} 
        plantId={selectedPlantId!} 
        onClose={() => setSelectedPlantId(null)}
        isSaved={selectedPlantId ? savedPlantIds.has(selectedPlantId) : false}
        onToggleSave={() => selectedPlantId && handleToggleSave(selectedPlantId)}
        gardens={gardens}
        gardenSaveStates={gardenSaveStates}
        onSaveToDestinations={
          (saveGlobal, gardenIds) => selectedPlantId && handleSaveToDestinations(selectedPlantId, saveGlobal, gardenIds)
        }
      />
    </div>
  );
};

export default BrowseSpecies;
