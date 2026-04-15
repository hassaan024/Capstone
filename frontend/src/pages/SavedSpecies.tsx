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

  const handleUnsave = async (perenualId: number) => {
    if (!user) return;
    try {
      const url = activeTab === 'global'
        ? `/species/save/${perenualId}?userId=${user.id}`
        : `/species/save/${perenualId}?userId=${user.id}&gardenId=${activeTab}`;
      await api.del(url);
      setSavedPlants(prev => prev.filter(p => p.perenualId !== perenualId));
      setSelectedPlantId(null); // Close modal if open
    } catch (err) {
        console.error("Failed to unsave plant", err);
    }
  };

  // Map SavedPlant (backend) to PlantCard props
  const mapToCardProps = (plant: SavedPlant) => ({
    id: plant.perenualId,
    common_name: plant.commonName,
    scientific_name: plant.scientificName,
    image_url: plant.imgSrcUrls?.regular || '',
    family_common_name: plant.family,
    modelCategory: plant.modelCategory
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

      {/* Details Modal */}
      <PlantDetailsModal 
        isOpen={!!selectedPlantId} 
        plantId={selectedPlantId!} 
        onClose={() => setSelectedPlantId(null)}
        isSaved={true} // Always true on this page
        onToggleSave={() => selectedPlantId && handleUnsave(selectedPlantId)}
      />
    </div>
  );
};

export default SavedSpecies;
