import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import { api } from '../utils/api';
import PlantCard from '../components/PlantCard';
import PlantDetailsModal from '../components/PlantDetailsModal';

const BrowseSpecies: React.FC = () => {
  const navigate = useNavigate();
  const [query, setQuery] = useState('');
  const [results, setResults] = useState<{ id: number; common_name: string; scientific_name: string; image_url?: string }[]>([]);
  const [loading, setLoading] = useState(false);
  const [selectedPlantId, setSelectedPlantId] = useState<number | null>(null);

  const categories = ['Vegetables', 'Fruits', 'Flowers', 'Herbs', 'Cacti'];

  const handleSearch = async (e: React.FormEvent | string) => {
    if (typeof e !== 'string') e.preventDefault();
    const q = typeof e === 'string' ? e : query;
    if (!q.trim()) return;

    setLoading(true);
    setResults([]); // Clear previous
    try {
      const { data } = await api.get(`/trefle/search?q=${encodeURIComponent(q)}`);
      // Trefle returns { data: [...] } structure
      if (Array.isArray(data.data)) {
        setResults(data.data);
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
            <span style={{ fontSize: '1.2rem' }}>🌿</span>
            Browse Plants
          </div>
          <button className="browse-back-btn" onClick={() => navigate('/dashboard')}>
            Back to Dashboard
          </button>
        </header>

        {/* Search & Filter */}
        <section className="browse-search-section">
          <form className="browse-search-bar" onSubmit={handleSearch}>
            <span className="browse-search-icon">🔍</span>
            <input 
              type="text" 
              className="browse-search-input" 
              placeholder="Search for a plant (e.g. Tomato, Rose)..."
              value={query}
              onChange={(e) => setQuery(e.target.value)}
            />
          </form>

          <div className="browse-categories">
            {categories.map(cat => (
              <button 
                key={cat} 
                className="browse-chip"
                onClick={() => {
                  setQuery(cat);
                  handleSearch(cat);
                }}
              >
                {cat}
              </button>
            ))}
          </div>
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
      />
    </div>
  );
};

export default BrowseSpecies;
