import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import { api } from '../utils/api';
import PlantCard from '../components/PlantCard';
import PlantDetailsModal from '../components/PlantDetailsModal';
import { useAuth } from '../context/AuthContext';

import { FaLeaf, FaSearch } from 'react-icons/fa';

const BrowseSpecies: React.FC = () => {
  const navigate = useNavigate();
  const [query, setQuery] = useState('');
  const [results, setResults] = useState<{ id: number; common_name: string; scientific_name: string; image_url?: string }[]>([]);
  const [loading, setLoading] = useState(false);
  const [selectedPlantId, setSelectedPlantId] = useState<number | null>(null);
  const [savedPlantIds, setSavedPlantIds] = useState<Set<number>>(new Set());
  const { user } = useAuth();
  // Suggest Chatbot interaction
  React.useEffect(() => {
    if (!localStorage.getItem('hasSeenBrowsePopup')) {
      const timer = setTimeout(() => {
        const event = new CustomEvent('suggestChat', { detail: 'What can I do on the Browse Species page?' });
        window.dispatchEvent(event);
        localStorage.setItem('hasSeenBrowsePopup', 'true');
      }, 2000);
      return () => clearTimeout(timer);
    }
  }, []);

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

  const handleToggleSave = async (plantId: number) => {
    console.log("handleToggleSave called for:", plantId, "User:", user);
    if (!user) {
        console.warn("No user logged in, cannot save.");
        // TODO: Show toast or login prompt
        return; 
    }
    
    const isSaved = savedPlantIds.has(plantId);
    console.log("Current save state:", isSaved);

    try {
        if (isSaved) {
            console.log("Unsaving...");
            await api.del(`/species/save/${plantId}?userId=${user.id}`);
            setSavedPlantIds(prev => {
                const next = new Set(prev);
                next.delete(plantId);
                return next;
            });
             console.log("Unsaved!");
        } else {
            console.log("Saving...");
            await api.post(`/species/save/${plantId}?userId=${user.id}`, {});
            setSavedPlantIds(prev => {
                const next = new Set(prev);
                next.add(plantId);
                return next;
            });
            console.log("Saved!");
        }
    } catch (err) {
        console.error("Failed to toggle save", err);
    }
  };


  const handleSearch = async (e: React.FormEvent | string) => {
    if (typeof e !== 'string') e.preventDefault();
    const q = typeof e === 'string' ? e : query;
    if (!q.trim()) return;

    setLoading(true);
    setResults([]); // Clear previous
    try {
      const { data } = await api.get(`/perenual/search?query=${encodeURIComponent(q)}`);
      // Perenual returns { data: [...] } structure
      if (Array.isArray(data.data)) {
        const mappedResults = data.data.map((p: any) => ({
          id: p.id,
          common_name: p.common_name,
          scientific_name: Array.isArray(p.scientific_name) ? p.scientific_name[0] : p.scientific_name,
          image_url: p.default_image?.regular_url || p.default_image?.original_url
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
        {loading ? (
          <div style={{ textAlign: 'center', color: 'rgba(255,255,255,0.7)', marginTop: '2rem' }}>
            Searching the botanical database...
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
            {!loading && results.length === 0 && query && (
               <div style={{ gridColumn: '1 / -1', textAlign: 'center', color: 'rgba(255,255,255,0.5)' }}>
                 No plants found. Try a different search term.
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
      />
    </div>
  );
};

export default BrowseSpecies;
