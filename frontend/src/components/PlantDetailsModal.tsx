import React, { useEffect, useState } from 'react';
import { api } from '../utils/api';

interface PlantDetailsModalProps {
  plantId: number;
  isOpen: boolean;
  onClose: () => void;
  isSaved?: boolean;
  onToggleSave?: () => void;
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

import { FaSun, FaTint, FaThermometerHalf, FaFlask, FaInfoCircle, FaCheck, FaBookmark, FaRegBookmark, FaTimes } from 'react-icons/fa';

const PlantDetailsModal: React.FC<PlantDetailsModalProps> = ({ plantId, isOpen, onClose, isSaved, onToggleSave }) => {
  const [details, setDetails] = useState<PlantDetails | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const [showInfo, setShowInfo] = useState(false);

  useEffect(() => {
    if (isOpen && plantId) {
      setLoading(true);
      setError('');
      setShowInfo(false); // Reset info on open
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
        <button className="modal-close-btn" onClick={onClose}><FaTimes /></button>
        
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
                    <div className="modal-stat-label"><FaSun /> Light (0-10)</div>
                    <div className="modal-stat-value">{growth?.light ?? 'N/A'}</div>
                  </div>
                  <div className="modal-stat-box">
                    <div className="modal-stat-label"><FaTint /> Humidity</div>
                    <div className="modal-stat-value">{growth?.atmospheric_humidity ?? 'N/A'}/10</div>
                  </div>
                  <div className="modal-stat-box">
                    <div className="modal-stat-label"><FaThermometerHalf /> Temp Range</div>
                    <div className="modal-stat-value">
                      {growth?.minimum_temperature?.deg_f ?? '?'}°F - {growth?.maximum_temperature?.deg_f ?? '?'}°F
                    </div>
                  </div>
                  <div className="modal-stat-box">
                    <div className="modal-stat-label"><FaFlask /> Soil pH</div>
                    <div className="modal-stat-value">
                      {growth?.ph_minimum ?? '?'} - {growth?.ph_maximum ?? '?'}
                    </div>
                  </div>
                </div>

                <div className="modal-actions-wrapper" style={{ marginTop: '2rem' }}>
                    <div className="modal-actions" style={{ display: 'flex', justifyContent: 'flex-end', gap: '0.5rem', alignItems: 'center' }}>
                    
                    <button
                        onClick={() => setShowInfo(!showInfo)}
                        style={{
                            background: 'transparent',
                            border: '1px solid rgba(255,255,255,0.2)',
                            borderRadius: '50%',
                            width: '36px',
                            height: '36px',
                            display: 'flex',
                            alignItems: 'center',
                            justifyContent: 'center',
                            cursor: 'pointer',
                            color: 'white',
                            fontSize: '1.2rem'
                        }}
                        title="What is this?"
                    >
                        <FaInfoCircle />
                    </button>

                    <button 
                        onClick={onToggleSave}
                        style={{
                        padding: '0.75rem 1.5rem',
                        borderRadius: '8px',
                        border: 'none',
                        background: isSaved ? '#3b82f6' : '#10b981',
                        color: 'white',
                        fontWeight: 'bold',
                        cursor: 'pointer',
                        display: 'flex',
                        alignItems: 'center',
                        gap: '0.5rem',
                        fontSize: '1rem',
                        boxShadow: '0 4px 6px -1px rgba(0, 0, 0, 0.1)'
                        }}
                    >
                        <span>{isSaved ? <><FaBookmark /> Saved</> : <><FaRegBookmark /> Save for Later</>}</span>
                    </button>
                    </div>

                    {showInfo && (
                        <div style={{
                            marginTop: '1rem',
                            padding: '1rem',
                            background: 'rgba(59, 130, 246, 0.1)',
                            border: '1px solid rgba(59, 130, 246, 0.3)',
                            borderRadius: '8px',
                            color: '#bfdbfe',
                            fontSize: '0.9rem',
                            position: 'relative',
                            animation: 'fadeIn 0.2s ease-out'
                        }}>
                            <button 
                                onClick={() => setShowInfo(false)}
                                style={{
                                    position: 'absolute',
                                    top: '0.5rem',
                                    right: '0.5rem',
                                    background: 'transparent',
                                    border: 'none',
                                    color: '#bfdbfe',
                                    cursor: 'pointer',
                                    fontSize: '1rem'
                                }}
                            >
                                <FaTimes />
                            </button>
                            <strong>Save for Later:</strong> Collecting plants here allows you to easily access them when you open the garden planner in Unreal Engine. Build your dream palette!
                        </div>
                    )}
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
