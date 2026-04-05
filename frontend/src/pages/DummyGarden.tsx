import React from 'react';
import { useNavigate } from 'react-router-dom';
import '../styles/BrowseSpecies.css';
import GardenPlantCard from '../components/GardenPlantCard';
import { FaSeedling, FaMapMarkerAlt, FaClock, FaGlobe } from 'react-icons/fa';

const MOCK_GARDEN = {
  id: 0,
  name: "Emerald Sanctuary",
  description: "A lush, experimental garden showcasing different biomes and plant species. This is a preview of how your synchronized Unreal Engine garden will appear.",
  latitude: 34.0522,
  longitude: -118.2437,
  timezone: "America/Los_Angeles",
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
      lastWatered: new Date(Date.now() - 3600000 * 48).toISOString(),
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
      lastWatered: new Date(Date.now() - 3600000 * 72).toISOString(),
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
            <div className="browse-chip active">DEMO GARDEN</div>
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
              {MOCK_GARDEN.plants.length} plant{MOCK_GARDEN.plants.length === 1 ? '' : 's'} in this garden
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
