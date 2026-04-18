import React, { useState, useMemo } from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import GardenPlantCard from '../components/GardenPlantCard';
import { FaSeedling, FaMapMarkerAlt, FaClock, FaGlobe, FaSearch, FaChartBar, FaExclamationTriangle, FaLeaf, FaTint } from 'react-icons/fa';
import { mapPlantToVisualCategory } from '../utils/plantVisualCategory';

const MOCK_GARDEN = {
  id: 0,
  name: "Emerald Sanctuary",
  description: "A lush, experimental garden showcasing different biomes and plant species. This is a preview of how your synchronized Unreal Engine garden will appear.",
  latitude: 34.0522,
  longitude: -118.2437,
  timezone: "America/Los_Angeles",
  bloomDate: "2026-08-30T00:00:00Z",
  creationTimestamp: new Date().toISOString(),
  lastUpdated: new Date().toISOString(),
  _count: { plants: 5 },
  plants: [
    {
      id: 1,
      heightCm: 150,
      ageDays: 30,
      healthStatus: "Healthy",
      lastWatered: new Date(Date.now() - 3600000 * 6).toISOString(),
      plantedDate: new Date(Date.now() - 3600000 * 24 * 30).toISOString(),
      species: {
        commonName: 'common sunflower',
        scientificName: 'Helianthus annuus',
        type: 'Flower',
        flowers: true,
        cuisine: true,
        edibleFruit: true,
        edibleLeaf: true,
        medicinal: false,
        droughtTolerant: true,
        indoor: false,
        invasive: false,
        imgSrcUrls: { regular: 'https://s3.us-central-1.wasabisys.com/perenual/species_image/3384_helianthus_annuus/regular/52370427473_b8e914065a_b.jpg?X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0MPGHU7CIPXNPMVWMXUW%2F20260404%2Fus-central-1%2Fs3%2Faws4_request&X-Amz-Date=20260404T182622Z&X-Amz-SignedHeaders=host&X-Amz-Expires=86400&X-Amz-Signature=5047ce7bc2d518e6b5d75ef02307e5b025f20d131692d932e3e8d7df90fbf789' }
      },
      soil: { type: "LOAM" }
    },
    {
      id: 2,
      heightCm: 45,
      ageDays: 60,
      healthStatus: "Excellent",
      lastWatered: new Date(Date.now() - 3600000 * 24).toISOString(),
      plantedDate: new Date(Date.now() - 3600000 * 24 * 60).toISOString(),
      species: {
        commonName: 'Dolgo Apple',
        scientificName: "Malus 'Dolgo'",
        type: 'tree',
        flowers: false,
        cuisine: true,
        edibleFruit: true,
        edibleLeaf: false,
        medicinal: true,
        droughtTolerant: true,
        indoor: false,
        invasive: false,
        imgSrcUrls: { regular: 'https://s3.us-central-1.wasabisys.com/perenual/species_image/359_malus_dolgo/regular/apple-zieraepfel-wild-apple-tree-branch.jpg?X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0MPGHU7CIPXNPMVWMXUW%2F20260404%2Fus-central-1%2Fs3%2Faws4_request&X-Amz-Date=20260404T192749Z&X-Amz-SignedHeaders=host&X-Amz-Expires=86400&X-Amz-Signature=fa0882952f67e60b12a5ef410ea67be05dd1e952f4396e8142ebfe2fd29c5025' }
      },
      soil: { type: "LOAM" }
    },
    {
      id: 3,
      heightCm: 80,
      ageDays: 90,
      healthStatus: "Nurturing",
      lastWatered: new Date(Date.now() - 3600000 * 12).toISOString(),
      plantedDate: new Date(Date.now() - 3600000 * 24 * 90).toISOString(),
      species: {
        commonName: 'rose cactus',
        scientificName: 'Pereskia grandifolia',
        type: 'Broadleaf evergreen',
        flowers: true,
        cuisine: false,
        edibleFruit: false,
        edibleLeaf: false,
        medicinal: true,
        droughtTolerant: true,
        indoor: false,
        invasive: false,
        imgSrcUrls: { regular: 'https://s3.us-central-1.wasabisys.com/perenual/species_image/5809_pereskia_grandifolia/regular/27487227123_a9b4d90e13_b.jpg?X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0MPGHU7CIPXNPMVWMXUW%2F20260404%2Fus-central-1%2Fs3%2Faws4_request&X-Amz-Date=20260404T192810Z&X-Amz-SignedHeaders=host&X-Amz-Expires=86400&X-Amz-Signature=86b0f27d161aa3cf1a3942a5c7155856bc461807ad7242f67baffb42ef4deaa9' }
      },
      soil: { type: "SANDY" }
    },
    {
      id: 4,
      heightCm: 25,
      ageDays: 15,
      healthStatus: "NeedsWater",
      lastWatered: new Date(Date.now() - 3600000 * 24 * 4).toISOString(), // 4 days ago
      plantedDate: new Date(Date.now() - 3600000 * 24 * 15).toISOString(),
      species: {
        commonName: 'tomato',
        scientificName: "Lycopersicon esculentum 'Big Beef'",
        type: 'Fruit',
        flowers: true,
        cuisine: true,
        edibleFruit: true,
        edibleLeaf: true,
        medicinal: false,
        droughtTolerant: false,
        indoor: false,
        invasive: false,
        imgSrcUrls: { regular: 'https://s3.us-central-1.wasabisys.com/perenual/species_image/5022_lycopersicon_esculentum_big_beef/regular/52614055517_dca623a496_b.jpg?X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0MPGHU7CIPXNPMVWMXUW%2F20260404%2Fus-central-1%2Fs3%2Faws4_request&X-Amz-Date=20260404T193104Z&X-Amz-SignedHeaders=host&X-Amz-Expires=86400&X-Amz-Signature=29f63dd49aa3c61cdb053f549d33b5434d50c502a92c1beb41501067f771af90' }
      },
      soil: { type: "LOAM" }
    },
    {
      id: 5,
      heightCm: 300,
      ageDays: 365,
      healthStatus: "Healthy",
      lastWatered: new Date(Date.now() - 3600000 * 24 * 5).toISOString(), // 5 days ago
      plantedDate: new Date(Date.now() - 3600000 * 24 * 365).toISOString(),
      species: {
        commonName: 'Candied Apple Flowering Crab',
        scientificName: "Malus 'Candied Apple'",
        type: 'tree',
        flowers: false,
        cuisine: false,
        edibleFruit: false,
        edibleLeaf: false,
        medicinal: false,
        droughtTolerant: false,
        indoor: false,
        invasive: false,
        imgSrcUrls: { regular: 'https://s3.us-central-1.wasabisys.com/perenual/species_image/355_malus_candied_apple/regular/663px-Apples_on_tree_2021_G1.jpg?X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0MPGHU7CIPXNPMVWMXUW%2F20260404%2Fus-central-1%2Fs3%2Faws4_request&X-Amz-Date=20260404T193133Z&X-Amz-SignedHeaders=host&X-Amz-Expires=86400&X-Amz-Signature=422a4eed7a77cea3e19f676988a2686160c46fa32b0ea638f478a3a70d64460a' }
      },
      soil: { type: "LOAM" }
    }
  ]
};

const DummyGarden: React.FC = () => {
  const navigate = useNavigate();
  const [searchTerm, setSearchTerm] = useState('');
  const [showAnalytics, setShowAnalytics] = useState(false);

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
      </div>
    </div>
  );
};

export default DummyGarden;
