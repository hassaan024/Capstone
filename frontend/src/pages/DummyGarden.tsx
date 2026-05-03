import React, { useState, useMemo } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import GardenPlantCard from '../components/GardenPlantCard';
import { FaSeedling, FaMapMarkerAlt, FaClock, FaGlobe, FaSearch, FaChartBar, FaExclamationTriangle, FaLeaf, FaTint, FaProjectDiagram } from 'react-icons/fa';
import PlantStageTrackerCard from '../components/PlantStageTrackerCard';
import { mapPlantToVisualCategory } from '../utils/plantVisualCategory';

const MOCK_GARDEN = {
  id: 0,
  name: "Emerald Sanctuary",
  description: "A lush, experimental garden showcasing different biomes and plant species. This is a preview of how your synchronized garden will appear in the app.",
  latitude: 34.0522,
  longitude: -118.2437,
  timezone: "America/Los_Angeles",
  bloomDate: "2026-08-30T00:00:00Z",
  creationTimestamp: new Date().toISOString(),
  lastUpdated: new Date().toISOString(),
  _count: { plants: 3 },
  plants: [
    {
      id: 1,
      heightCm: 25,
      ageDays: 15,
      healthStatus: "NeedsWater",
      lastWatered: new Date(Date.now() - 3600000 * 24 * 4).toISOString(),
      plantedDate: "2026-07-31T00:00:00.000Z", // bloomDate (Aug 30) - 30 days
      species: {
        commonName: "tomato",
        scientificName: "Lycopersicon esculentum 'Big Beef'",
        type: "Fruit",
        flowers: true,
        cuisine: true,
        edibleFruit: true,
        edibleLeaf: true,
        medicinal: false,
        droughtTolerant: false,
        indoor: false,
        invasive: false,
        imgSrcUrls: { regular: "https://s3.us-central-1.wasabisys.com/perenual/species_image/5022_lycopersicon_esculentum_big_beef/regular/52614055517_dca623a496_b.jpg" },
        bloomDays: 30
      },
      soil: { type: "LOAM" }
    },
    {
      id: 3,
      heightCm: 45,
      ageDays: 60,
      healthStatus: "Excellent",
      lastWatered: new Date(Date.now() - 3600000 * 24).toISOString(),
      plantedDate: "2026-07-11T00:00:00.000Z", // bloomDate (Aug 30) - 50 days
      species: {
        commonName: "Akane Apple",
        scientificName: "Malus 'Akane'",
        type: "tree",
        flowers: false,
        cuisine: true,
        edibleFruit: false,
        edibleLeaf: false,
        medicinal: false,
        droughtTolerant: false,
        indoor: false,
        invasive: false,
        imgSrcUrls: { regular: "https://s3.us-central-1.wasabisys.com/perenual/species_image/351_malus_akane/regular/800px-Akane-Pomme-20141026.jpg" },
        bloomDays: 50
      },
      soil: { type: "LOAM" }
    },
    {
      id: 5,
      heightCm: 150,
      ageDays: 30,
      healthStatus: "Healthy",
      lastWatered: new Date(Date.now() - 3600000 * 6).toISOString(),
      plantedDate: "2026-08-10T00:00:00.000Z", // bloomDate (Aug 30) - 20 days
      species: {
        commonName: "lily of the Nile",
        scientificName: "Agapanthus (group)",
        type: "Bulb",
        flowers: true,
        cuisine: false,
        edibleFruit: false,
        edibleLeaf: false,
        medicinal: false,
        droughtTolerant: true,
        indoor: false,
        invasive: true,
        imgSrcUrls: { regular: "https://s3.us-central-1.wasabisys.com/perenual/species_image/575_agapanthus_group/regular/2742717111_04f3b1bee3_b.jpg" },
        bloomDays: 20
      },
      soil: { type: "SANDY" }
    }
  ]
};

const DummyGarden: React.FC = () => {
  const navigate = useNavigate();
  const [searchTerm, setSearchTerm] = useState('');
  const [showAnalytics, setShowAnalytics] = useState(false);
  const [subTab, setSubTab] = useState<'grid' | 'track'>('grid');
  
  // Timeline dates for Track tab
  const timelineDates = useMemo(() => {
    let earliest = MOCK_GARDEN.bloomDate ? new Date(MOCK_GARDEN.bloomDate).getTime() : Date.now();
    for (const p of MOCK_GARDEN.plants) {
      if (p.plantedDate) {
        const d = new Date(p.plantedDate).getTime();
        if (d < earliest) earliest = d;
      }
    }
    const end = MOCK_GARDEN.bloomDate ? new Date(MOCK_GARDEN.bloomDate).getTime() : Date.now();
    return { start: earliest, end };
  }, []);

  const [sliderValue, setSliderValue] = useState(100); // 0 to 100
  const currentTimestamp = useMemo(() => {
    if (timelineDates.start === timelineDates.end) return timelineDates.end;
    const diff = timelineDates.end - timelineDates.start;
    return timelineDates.start + (diff * (sliderValue / 100));
  }, [sliderValue, timelineDates]);

  const formatDt = (iso: string) => {
    try {
      return new Date(iso).toLocaleString(undefined, { dateStyle: 'medium', timeStyle: 'short' });
    } catch {
      return iso;
    }
  };

  const analytics = useMemo(() => {
    const plants = MOCK_GARDEN.plants;
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
  }, []);

  const filteredPlants = useMemo(() => {
    return MOCK_GARDEN.plants.filter(p => 
      p.species.commonName.toLowerCase().includes(searchTerm.toLowerCase()) ||
      p.species.scientificName.toLowerCase().includes(searchTerm.toLowerCase())
    );
  }, [searchTerm]);

  const mapsHref = `https://www.google.com/maps?q=${MOCK_GARDEN.latitude},${MOCK_GARDEN.longitude}`;

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
            Demo Garden
          </div>
          <div style={{ display: 'flex', gap: '0.75rem' }}>
            <button className="browse-back-btn" onClick={() => navigate('/gardens')}>

              Back to My Gardens
            </button>
            <button className="browse-back-btn" onClick={() => navigate('/dashboard')}>
              Back to Dashboard
            </button>
          </div>
        </header>

        <section
          style={{
            background: 'rgba(30, 41, 59, 0.55)',
            border: '1px solid rgba(148, 163, 184, 0.2)',
            borderRadius: '12px',
            padding: '1.25rem 1.5rem',
            position: 'relative',
            overflow: 'hidden'
          }}
        >
          <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start' }}>
            <div>
              <h2 style={{ margin: '0 0 0.5rem', color: 'rgba(255,255,255,0.95)' }}>
                {MOCK_GARDEN.name}
              </h2>
              <p style={{ color: 'rgba(255,255,255,0.7)', margin: '0 0 1rem', lineHeight: 1.5, maxWidth: '800px' }}>
                {MOCK_GARDEN.description}
              </p>
              {MOCK_GARDEN.bloomDate && (
                <div style={{
                  display: 'inline-flex',
                  alignItems: 'center',
                  gap: '0.5rem',
                  background: 'rgba(252, 211, 77, 0.15)',
                  color: '#fcd34d',
                  padding: '0.4rem 0.8rem',
                  borderRadius: '6px',
                  fontSize: '0.85rem',
                  fontWeight: 600,
                  marginBottom: '1rem',
                  border: '1px solid rgba(252, 211, 77, 0.25)'
                }}>
                  <FaClock />
                  Target Bloom: {new Date(MOCK_GARDEN.bloomDate).toLocaleDateString(undefined, { timeZone: 'UTC', month: 'long', day: 'numeric', year: 'numeric' })}
                </div>
              )}
            </div>
            <button 
              className={`browse-back-btn ${showAnalytics ? 'active' : ''}`} 
              onClick={() => setShowAnalytics(!showAnalytics)}
              style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', color: showAnalytics ? '#86efac' : undefined, borderColor: showAnalytics ? '#86efac' : undefined, backgroundColor: 'rgba(15, 23, 42, 0.4)' }}
            >
              <FaChartBar /> {showAnalytics ? 'Hide Analytics' : 'Garden Analytics'}
            </button>
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
                {MOCK_GARDEN.latitude.toFixed(4)}, {MOCK_GARDEN.longitude.toFixed(4)}
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
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
              <FaGlobe /> {MOCK_GARDEN.timezone}
            </div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.35rem', color: analytics.thirsty.length > 0 ? '#fca5a5' : '#86efac' }}>
              <FaTint /> {analytics.thirsty.length > 0 ? `${analytics.thirsty.length} plants need attention` : 'All plants recently watered'}
            </div>
          </div>
        </section>

        {showAnalytics && (
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
              backdropFilter: 'blur(12px)'
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
                  {analytics.thirsty.map(p => (
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

        {/* Sub-tab bar: Grid | Track */}
        <div className="sub-tab-bar" style={{ marginTop: '1rem', marginBottom: '1rem' }}>
          <button
            className={`sub-tab ${subTab === 'grid' ? 'active' : ''}`}
            onClick={() => setSubTab('grid')}
          >
            <FaSeedling /> Plant Grid
          </button>
          <button
            className={`sub-tab ${subTab === 'track' ? 'active' : ''}`}
            onClick={() => setSubTab('track')}
          >
            <FaProjectDiagram /> Track Growth Stages
          </button>
        </div>

        {subTab === 'grid' && (
          <>
            <div className="browse-search-bar" style={{ maxWidth: '100%' }}>
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
                Plants in Grid
              </h3>
              <span style={{ fontSize: '0.9rem', color: 'rgba(255,255,255,0.5)' }}>
                Showing {filteredPlants.length} of {MOCK_GARDEN.plants.length}
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
                  <GardenPlantCard key={p.id} plant={p as any} />
                ))}
              </div>
            )}
          </>
        )}

        {subTab === 'track' && (
          <>
            <div style={{
              background: 'rgba(15, 23, 42, 0.8)',
              border: '1px solid rgba(148, 163, 184, 0.2)',
              borderRadius: '12px',
              padding: '1.5rem',
              marginBottom: '1.5rem'
            }}>
              <h3 style={{ margin: '0 0 1rem', color: '#86efac', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                <FaClock /> Timeline Tracker
              </h3>
              <p style={{ color: 'rgba(255,255,255,0.6)', fontSize: '0.9rem', marginBottom: '1.5rem' }}>
                Drag the timeline to see what stages your plants will be in at different dates. Models automatically transition between stages as they grow.
              </p>
              
              <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.85rem', color: 'rgba(255,255,255,0.8)', marginBottom: '0.5rem' }}>
                <span>{new Date(timelineDates.start).toLocaleDateString(undefined, {month: 'short', day: 'numeric'})} (First Plant)</span>
                <span style={{ color: '#fbbf24', fontWeight: 'bold' }}>
                  Viewing: {new Date(currentTimestamp).toLocaleDateString(undefined, {month: 'short', day: 'numeric', year: 'numeric'})}
                </span>
                <span>{new Date(timelineDates.end).toLocaleDateString(undefined, {month: 'short', day: 'numeric'})} (Bloom Target)</span>
              </div>
              
              <input 
                type="range" 
                min="0" 
                max="100" 
                value={sliderValue} 
                onChange={(e) => setSliderValue(Number(e.target.value))}
                style={{
                  width: '100%',
                  appearance: 'none',
                  height: '8px',
                  background: 'rgba(255,255,255,0.1)',
                  borderRadius: '4px',
                  outline: 'none',
                  cursor: 'pointer'
                }}
              />
            </div>
            
            <div className="browse-grid">
              {MOCK_GARDEN.plants.map(p => (
                <PlantStageTrackerCard
                  key={p.id}
                  plant={p}
                  currentTimestamp={currentTimestamp}
                  bloomTimestamp={timelineDates.end}
                />
              ))}
            </div>
          </>
        )}
      </div>
    </div>
  );
};

export default DummyGarden;
