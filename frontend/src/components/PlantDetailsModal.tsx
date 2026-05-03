import React, { useEffect, useState } from 'react';
import { api } from '../utils/api';
import { useAuth } from '../context/AuthContext';
import PlantShoppingPanel from './PlantShoppingPanel';
import SaveDestinationModal from './SaveDestinationModal';

interface Garden {
  id: number;
  name: string;
}

interface PlantDetailsModalProps {
  plantId: number;
  isOpen: boolean;
  onClose: () => void;
  isSaved?: boolean;
  onToggleSave?: () => void;
  // Garden-aware save props
  gardens?: Garden[];
  gardenSaveStates?: Record<number, boolean>;
  onSaveToDestinations?: (saveGlobal: boolean, gardenIds: number[]) => void;
  // Pre-computed bloom days from the DB (shown if Perenual doesn't return one)
  bloomDays?: number;
}

interface PlantDetails {
  common_name: string;
  scientific_name: string[];
  default_image?: {
    regular_url: string;
  };
  sunlight?: string[];
  watering?: string;
  hardiness?: {
    min: string;
    max: string;
  };
  care_level?: string;
  maintenance?: string;
  type?: string;
  cycle?: string;
  growth_rate?: string;
  indoor?: boolean;
  drought_tolerant?: boolean;
  medicinal?: boolean;
  edible_fruit?: boolean;
  edible_leaf?: boolean;
  invasive?: boolean;
  modelCategory?: string;
  bloomDays?: number;
}

import { FaSun, FaTint, FaThermometerHalf, FaFlask, FaInfoCircle, FaCheck, FaBookmark, FaRegBookmark, FaTimes } from 'react-icons/fa';

const PlantDetailsModal: React.FC<PlantDetailsModalProps> = ({
  plantId, isOpen, onClose,
  isSaved, onToggleSave,
  gardens, gardenSaveStates, onSaveToDestinations,
  bloomDays: bloomDaysProp,
}) => {
  const { user } = useAuth();
  const [details, setDetails] = useState<PlantDetails | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const [showInfo, setShowInfo] = useState(false);
  const [showSaveModal, setShowSaveModal] = useState(false);

  useEffect(() => {
    if (isOpen && plantId) {
      setLoading(true);
      setError('');
      setShowInfo(false); // Reset info on open
      setShowSaveModal(false);
      api.get(`/perenual/details/${plantId}`)
        .then((res: { data: PlantDetails }) => {
          setDetails(res.data);
          setLoading(false);
          // Suggest a question to the chatbot, honoring user preferences
          if (user?.plantRecommendations !== false) {
            setTimeout(() => {
              const sciName = Array.isArray(res.data.scientific_name) ? res.data.scientific_name[0] : res.data.scientific_name;
              const event = new CustomEvent('suggestChat', { detail: `Tell me some fun facts about ${sciName} (${res.data.common_name})!` });
              window.dispatchEvent(event);
            }, 1000);
          }
        })
        .catch((err: Error) => {
          console.error(err);
          setError('Failed to load plant details.');
          setLoading(false);
        });
    }
  }, [isOpen, plantId]);

  if (!isOpen) return null;

  const hasGardens = gardens && gardens.length > 0;
  const useDestinationFlow = !!onSaveToDestinations;

  const handleSaveClick = () => {
    if (useDestinationFlow) {
      setShowSaveModal(true);
    } else if (onToggleSave) {
      onToggleSave();
    }
  };

  const handleDestinationConfirm = (saveGlobal: boolean, gardenIds: number[]) => {
    if (onSaveToDestinations) {
      onSaveToDestinations(saveGlobal, gardenIds);
    }
  };

  // Determine if plant is saved anywhere (global or any garden)
  const isSavedAnywhere = isSaved || (gardenSaveStates && Object.values(gardenSaveStates).some(Boolean));

  return (
    <>
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
                {details.default_image?.regular_url && (
                  <img 
                    src={details.default_image.regular_url} 
                    alt={details.common_name} 
                    className="modal-header-image" 
                  />
                )}
                <div className="modal-details">
                  <h2 className="modal-title">{details.common_name}</h2>
                  <p className="modal-subtitle">
                    {Array.isArray(details.scientific_name) ? details.scientific_name[0] : details.scientific_name}
                  </p>

                  <div className="modal-grid">
                    <div className="modal-stat-box">
                      <div className="modal-stat-label"><FaSun /> Sunlight</div>
                      <div className="modal-stat-value">{details.sunlight?.join(', ') || 'N/A'}</div>
                    </div>
                    <div className="modal-stat-box">
                      <div className="modal-stat-label"><FaTint /> Watering</div>
                      <div className="modal-stat-value">{details.watering || 'N/A'}</div>
                    </div>
                    <div className="modal-stat-box">
                      <div className="modal-stat-label"><FaThermometerHalf /> Hardiness Zones</div>
                      <div className="modal-stat-value">
                        {details.hardiness?.min === details.hardiness?.max 
                          ? (details.hardiness?.min ?? '?') 
                          : `${details.hardiness?.min ?? '?'} - ${details.hardiness?.max ?? '?'}`}
                      </div>
                    </div>
                    <div className="modal-stat-box">
                      <div className="modal-stat-label"><FaFlask /> Maintenance</div>
                      <div className="modal-stat-value">
                        {details.maintenance || details.care_level || 'N/A'}
                      </div>
                    </div>
                    
                    {/* New Perenual Details */}
                    {details.type && (
                      <div className="modal-stat-box">
                        <div className="modal-stat-label"><FaCheck /> Type</div>
                        <div className="modal-stat-value">{details.type}</div>
                      </div>
                    )}
                    {(details.bloomDays || bloomDaysProp) && (
                      <div className="modal-stat-box" style={{ background: 'rgba(56, 189, 248, 0.1)', borderColor: 'rgba(56, 189, 248, 0.3)' }}>
                        <div className="modal-stat-label" style={{ color: '#38bdf8' }}><FaInfoCircle /> Est. Time to Grow</div>
                        <div className="modal-stat-value" style={{ color: '#38bdf8' }}>~{details.bloomDays ?? bloomDaysProp} Days</div>
                      </div>
                    )}
                    {details.cycle && (
                      <div className="modal-stat-box">
                        <div className="modal-stat-label"><FaCheck /> Life Cycle</div>
                        <div className="modal-stat-value">{details.cycle}</div>
                      </div>
                    )}
                    {details.growth_rate && (
                      <div className="modal-stat-box">
                        <div className="modal-stat-label"><FaCheck /> Growth Rate</div>
                        <div className="modal-stat-value">{details.growth_rate}</div>
                      </div>
                    )}
                  </div>

                  {/* Tags / Badges */}
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.5rem', marginTop: '1rem' }}>
                    {details.indoor && <span style={{ padding: '0.25rem 0.5rem', background: 'rgba(59, 130, 246, 0.2)', color: '#93c5fd', borderRadius: '4px', fontSize: '0.8rem', fontWeight: 'bold' }}>Indoor Friendly</span>}
                    {details.drought_tolerant && <span style={{ padding: '0.25rem 0.5rem', background: 'rgba(245, 158, 11, 0.2)', color: '#fcd34d', borderRadius: '4px', fontSize: '0.8rem', fontWeight: 'bold' }}>Drought Tolerant</span>}
                    {details.medicinal && <span style={{ padding: '0.25rem 0.5rem', background: 'rgba(16, 185, 129, 0.2)', color: '#6ee7b7', borderRadius: '4px', fontSize: '0.8rem', fontWeight: 'bold' }}>Medicinal</span>}
                    {(details.edible_fruit || details.edible_leaf) && <span style={{ padding: '0.25rem 0.5rem', background: 'rgba(217, 70, 239, 0.2)', color: '#f0abfc', borderRadius: '4px', fontSize: '0.8rem', fontWeight: 'bold' }}>Edible</span>}
                    {details.invasive && <span style={{ padding: '0.25rem 0.5rem', background: 'rgba(239, 68, 68, 0.2)', color: '#fca5a5', borderRadius: '4px', fontSize: '0.8rem', fontWeight: 'bold' }}>Invasive</span>}
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
                          onClick={handleSaveClick}
                          style={{
                          padding: '0.75rem 1.5rem',
                          borderRadius: '8px',
                          border: 'none',
                          background: isSavedAnywhere ? '#3b82f6' : '#10b981',
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
                          <span>
                            {isSavedAnywhere
                              ? <><FaBookmark /> {useDestinationFlow ? 'Manage Saves' : 'Saved'}</>
                              : <><FaRegBookmark /> Save for Later</>
                            }
                          </span>
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
                              Save plants for later to easily access and add them when you open the garden planner in the app.
                              {hasGardens && ' You can save globally or to a specific garden.'}
                          </div>
                      )}
                  </div>

                  {/* Plant Shopping / Where to Buy */}
                  <PlantShoppingPanel
                    plantName={details.common_name || (Array.isArray(details.scientific_name) ? details.scientific_name[0] : details.scientific_name)}
                    plantId={plantId}
                  />
                </div>
              </div>
            </>
          ) : null}
        </div>
      </div>

      {/* Save Destination Modal */}
      {details && (
        <SaveDestinationModal
          isOpen={showSaveModal}
          onClose={() => setShowSaveModal(false)}
          plantName={details.common_name}
          gardens={gardens || []}
          isGloballySaved={!!isSaved}
          gardenSaveStates={gardenSaveStates || {}}
          onConfirm={handleDestinationConfirm}
        />
      )}
    </>
  );
};

export default PlantDetailsModal;
