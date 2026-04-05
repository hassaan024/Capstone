import React from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import GardenPlantCard from '../components/GardenPlantCard';
import { FaSeedling, FaMapMarkerAlt, FaClock, FaGlobe } from 'react-icons/fa';

const MOCK_GARDEN = {
  id: 0,
  name: "Emerald Sanctuary (Demo Garden)",
  description: "A lush, experimental garden showcasing different biomes and plant species. This is a preview of how your synchronized Unreal Engine garden will appear.",
  latitude: 34.0522,
  longitude: -118.2437,
  timezone: "America/Los_Angeles",
  creationTimestamp: new Date().toISOString(),
  lastUpdated: new Date().toISOString(),
  _count: { plants: 5 },
  plants: [
    {
      id: 101,
      heightCm: 120,
      ageDays: 45,
      healthStatus: "Excellent",
      lastWatered: new Date(Date.now() - 3600000 * 4).toISOString(),
      notes: "The Japanese Maple is thriving in the partial shade area. Its leaves are showing vibrant autumn colors.",
      species: {
        commonName: "Japanese Maple",
        scientificName: "Acer palmatum",
        type: "Tree",
        flowers: false,
        cuisine: false,
        edibleFruit: false,
        edibleLeaf: false,
        imgSrcUrls: { regular: "https://images.unsplash.com/photo-1518531933037-91b2f5f229cc?auto=format&fit=crop&q=80&w=800" }
      },
      soil: { type: "Loamy" }
    },
    {
      id: 102,
      heightCm: 35,
      ageDays: 20,
      healthStatus: "Good",
      lastWatered: new Date(Date.now() - 3600000 * 12).toISOString(),
      notes: "The Lavender is attracting many pollinators. It smells wonderful when in bloom.",
      species: {
        commonName: "Lavender",
        scientificName: "Lavandula angustifolia",
        type: "Herb",
        flowers: true,
        cuisine: true,
        edibleFruit: false,
        edibleLeaf: true,
        imgSrcUrls: { regular: "https://images.unsplash.com/photo-1520658406734-d022b39dd75d?auto=format&fit=crop&q=80&w=800" }
      },
      soil: { type: "Sandy" }
    },
    {
      id: 103,
      heightCm: 55,
      ageDays: 30,
      healthStatus: "Healthy",
      lastWatered: new Date(Date.now() - 3600000 * 2).toISOString(),
      notes: "Tomato plants are growing rapidly. Starting to see small green fruits forming.",
      species: {
        commonName: "Organic Tomato",
        scientificName: "Solanum lycopersicum",
        type: "Vegetable",
        flowers: true,
        cuisine: true,
        edibleFruit: true,
        edibleLeaf: false,
        imgSrcUrls: { regular: "https://images.unsplash.com/photo-1592841200221-a6898f307bac?auto=format&fit=crop&q=80&w=800" }
      },
      soil: { type: "Compost Mix" }
    },
    {
      id: 104,
      heightCm: 25,
      ageDays: 15,
      healthStatus: "Rapid Growth",
      lastWatered: new Date(Date.now() - 3600000 * 24).toISOString(),
      notes: "Basil is bushy and ready for light pruning. Very aromatic.",
      species: {
        commonName: "Sweet Basil",
        scientificName: "Ocimum basilicum",
        type: "Herb",
        flowers: false,
        cuisine: true,
        edibleFruit: false,
        edibleLeaf: true,
        imgSrcUrls: { regular: "https://images.unsplash.com/photo-1618375531912-77ac3142a142?auto=format&fit=crop&q=80&w=800" }
      },
      soil: { type: "Potting Mix" }
    },
    {
      id: 105,
      heightCm: 180,
      ageDays: 60,
      healthStatus: "Majestic",
      lastWatered: new Date(Date.now() - 3600000 * 48).toISOString(),
      notes: "The Sunflower is tall and following the sun perfectly. Petals are just about to open.",
      species: {
        commonName: "Common Sunflower",
        scientificName: "Helianthus annuus",
        type: "Flower",
        flowers: true,
        cuisine: true,
        edibleFruit: true,
        edibleLeaf: false,
        imgSrcUrls: { regular: "https://images.unsplash.com/photo-1470509037663-253afd7f0f51?auto=format&fit=crop&q=80&w=800" }
      },
      soil: { type: "Loamy" }
    }
  ]
};

const DummyGarden: React.FC = () => {
  const navigate = useNavigate();

  const formatDt = (iso: string) => {
    try {
      return new Date(iso).toLocaleString(undefined, { dateStyle: 'medium', timeStyle: 'short' });
    } catch {
      return iso;
    }
  };

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
            My Gardens (Preview)
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
            marginBottom: '1.75rem',
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
            </div>
            <div className="browse-chip active">DEMO MODE</div>
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
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
              <FaClock /> Updated {formatDt(MOCK_GARDEN.lastUpdated)}
            </div>
            <div style={{ opacity: 0.85 }}>Created {formatDt(MOCK_GARDEN.creationTimestamp)}</div>
            <div style={{ fontWeight: 600, marginTop: '0.25rem', color: '#86efac' }}>
              {MOCK_GARDEN.plants.length} plant{MOCK_GARDEN.plants.length === 1 ? '' : 's'} in this demo garden
            </div>
          </div>
        </section>

        <h3
          style={{
            color: 'rgba(255,255,255,0.9)',
            fontSize: '1.1rem',
            marginBottom: '1rem',
          }}
        >
          Plants
        </h3>
        <div className="browse-grid">
          {MOCK_GARDEN.plants.map((p) => (
            <GardenPlantCard key={p.id} plant={p as any} />
          ))}
        </div>
      </div>
    </div>
  );
};

export default DummyGarden;
