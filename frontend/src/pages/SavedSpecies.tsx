import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import { api } from '../utils/api';
import PlantCard from '../components/PlantCard';
import PlantDetailsModal from '../components/PlantDetailsModal';
import { useAuth } from '../context/AuthContext';
import { FaLeaf, FaBookmark, FaSearch } from 'react-icons/fa';

interface SavedPlant {
  id: number;
  trefleId: number;
  commonName: string;
  scientificName: string;
  imgSrcUrl: string;
  familyCommonName?: string;
}

const SavedSpecies: React.FC = () => {
  const navigate = useNavigate();
  const { user } = useAuth();
  const [savedPlants, setSavedPlants] = useState<SavedPlant[]>([]);
  const [loading, setLoading] = useState(true);
  const [selectedPlantId, setSelectedPlantId] = useState<number | null>(null);

  useEffect(() => {
    if (user) {
      fetchSavedPlants();
    }
  }, [user]);

  const fetchSavedPlants = async () => {
    setLoading(true);
    try {
      const res = await api.get(`/species/saved?userId=${user?.id}`);
      setSavedPlants(res.data);
    } catch (err) {
      console.error("Failed to fetch saved species", err);
    } finally {
      setLoading(false);
    }
  };

  const handleUnsave = async (trefleId: number) => {
    if (!user) return;
    try {
        await api.del(`/species/save/${trefleId}?userId=${user.id}`);
        setSavedPlants(prev => prev.filter(p => p.trefleId !== trefleId));
        setSelectedPlantId(null); // Close modal if open
    } catch (err) {
        console.error("Failed to unsave plant", err);
    }
  };

  // Map SavedPlant (backend) to PlantCard props
  const mapToCardProps = (plant: SavedPlant) => ({
    id: plant.trefleId, // Use trefleId for card events to match Browse page logic if needed, or just for consistency
    common_name: plant.commonName,
    scientific_name: plant.scientificName,
    image_url: plant.imgSrcUrl,
    family_common_name: plant.familyCommonName
  });

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

        {/* Action Header */}
        <div style={{ display: 'flex', justifyContent: 'flex-end' }}>
             <button 
                className="browse-chip active"
                style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
                onClick={() => navigate('/browse')}
              >
                <FaSearch /> Save more plants
              </button>
        </div>

        {/* Grid */}
        {loading ? (
          <div style={{ textAlign: 'center', color: 'rgba(255,255,255,0.7)', marginTop: '2rem' }}>
            Loading your garden...
          </div>
        ) : (
          <div className="browse-grid">
            {savedPlants.length > 0 ? (
                savedPlants.map(plant => (
                <PlantCard 
                    key={plant.id} 
                    plant={mapToCardProps(plant)} 
                    onClick={() => setSelectedPlantId(plant.trefleId)} // Pass trefleId for API calls
                />
                ))
            ) : (
                <div style={{ gridColumn: '1 / -1', textAlign: 'center', color: 'rgba(255,255,255,0.5)', padding: '3rem' }}>
                  <FaLeaf style={{ fontSize: '3rem', marginBottom: '1rem', opacity: 0.5 }} />
                  <p>You haven't saved any plants yet.</p>
                  <button 
                    className="ll-btn ll-btn-primary" 
                    style={{ marginTop: '1rem' }}
                    onClick={() => navigate('/browse')}
                  >
                    Browse Species
                  </button>
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
