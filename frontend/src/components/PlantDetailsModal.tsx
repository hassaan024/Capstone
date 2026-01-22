import React, { useEffect, useState } from 'react';
import { api } from '../utils/api';

interface PlantDetailsModalProps {
  plantId: number;
  isOpen: boolean;
  onClose: () => void;
}

interface PlantDetails {
  common_name: string;
  scientific_name: string;
  image_url: string;
  main_species: {
    growth: {
      ph_maximum: number;
      ph_minimum: number;
      light: number;
      atmospheric_humidity: number;
      soil_nutriments: number;
      minimum_temperature: { deg_f: number; deg_c: number };
      maximum_temperature: { deg_f: number; deg_c: number };
    };
  };
}

const PlantDetailsModal: React.FC<PlantDetailsModalProps> = ({ plantId, isOpen, onClose }) => {
  const [details, setDetails] = useState<PlantDetails | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');

  useEffect(() => {
    if (isOpen && plantId) {
      setLoading(true);
      setError('');
      api.get(`/trefle/species/${plantId}`)
        .then((res: { data: { data: PlantDetails } }) => {
          setDetails(res.data.data);
          setLoading(false);
        })
        .catch((err: Error) => {
          console.error(err);
          setError('Failed to load plant details.');
          setLoading(false);
        });
    }
  }, [isOpen, plantId]);

  if (!isOpen) return null;

  const growth = details?.main_species?.growth;

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-content" onClick={e => e.stopPropagation()}>
        <button className="modal-close-btn" onClick={onClose}>&times;</button>
        
        {loading ? (
          <div style={{ padding: '3rem', textAlign: 'center', color: 'white' }}>Loading plant data...</div>
        ) : error ? (
          <div style={{ padding: '3rem', textAlign: 'center', color: '#f87171' }}>{error}</div>
        ) : details ? (
          <>
            <div className="modal-body">
              {details.image_url && (
                <img 
                  src={details.image_url} 
                  alt={details.common_name} 
                  className="modal-header-image" 
                />
              )}
              <div className="modal-details">
                <h2 className="modal-title">{details.common_name}</h2>
                <p className="modal-subtitle">{details.scientific_name}</p>

                <div className="modal-grid">
                  <div className="modal-stat-box">
                    <div className="modal-stat-label">☀️ Light (0-10)</div>
                    <div className="modal-stat-value">{growth?.light ?? 'N/A'}</div>
                  </div>
                  <div className="modal-stat-box">
                    <div className="modal-stat-label">💧 Humidity</div>
                    <div className="modal-stat-value">{growth?.atmospheric_humidity ?? 'N/A'}/10</div>
                  </div>
                  <div className="modal-stat-box">
                    <div className="modal-stat-label">🌡️ Temp Range</div>
                    <div className="modal-stat-value">
                      {growth?.minimum_temperature?.deg_f ?? '?'}°F - {growth?.maximum_temperature?.deg_f ?? '?'}°F
                    </div>
                  </div>
                  <div className="modal-stat-box">
                    <div className="modal-stat-label">🧪 Soil pH</div>
                    <div className="modal-stat-value">
                      {growth?.ph_minimum ?? '?'} - {growth?.ph_maximum ?? '?'}
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </>
        ) : null}
      </div>
    </div>
  );
};

export default PlantDetailsModal;
