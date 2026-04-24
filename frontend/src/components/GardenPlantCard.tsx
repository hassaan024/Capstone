import React from 'react';
import { mapPlantToVisualCategory } from '../utils/plantVisualCategory';

export interface GardenPlantCardSpecies {
  commonName: string;
  scientificName: string;
  type?: string | null;
  flowers?: boolean | null;
  cuisine?: boolean | null;
  edibleFruit?: boolean | null;
  edibleLeaf?: boolean | null;
  medicinal?: boolean | null;
  droughtTolerant?: boolean | null;
  indoor?: boolean | null;
  invasive?: boolean | null;
  imgSrcUrls?: { regular?: string | null } | Record<string, unknown> | null;
}

export interface GardenPlantCardProps {
  plant: {
    id: number;
    heightCm?: number | null;
    ageDays?: number | null;
    healthStatus?: string | null;
    lastWatered?: string | null;
    plantedDate?: string | null;
    notes?: string | null;
    species: GardenPlantCardSpecies;
    soil: { type: string };
  };
}

function imageFromSpecies(s: GardenPlantCardSpecies): string {
  const urls = s.imgSrcUrls;
  if (urls && typeof urls === 'object' && 'regular' in urls && urls.regular) {
    return String(urls.regular);
  }
  return '';
}

function formatWhen(iso: string | null | undefined): string {
  if (!iso) return '—';
  try {
    return new Date(iso).toLocaleDateString(undefined, {
      month: 'short',
      day: 'numeric',
      year: 'numeric',
    });
  } catch {
    return iso;
  }
}

const chipStyle: React.CSSProperties = {
  fontSize: '0.7rem',
  padding: '0.15rem 0.45rem',
  borderRadius: '4px',
  fontWeight: 600,
  background: 'rgba(148, 163, 184, 0.2)',
  color: '#cbd5e1',
};

const utilityBadgeStyle = (bg: string, color: string): React.CSSProperties => ({
  padding: '0.15rem 0.4rem',
  background: bg,
  color: color,
  borderRadius: '4px',
  fontSize: '0.65rem',
  fontWeight: 'bold',
  textTransform: 'uppercase',
  letterSpacing: '0.02em',
});

const GardenPlantCard: React.FC<GardenPlantCardProps> = ({ plant }) => {
  const { species } = plant;
  const category = mapPlantToVisualCategory({
    type: species.type,
    scientificName: species.scientificName,
    commonName: species.commonName,
    cuisine: species.cuisine,
    edibleFruit: species.edibleFruit,
    edibleLeaf: species.edibleLeaf,
  });
  const imageUrl = imageFromSpecies(species);

  return (
    <div className="plant-card" style={{ cursor: 'default' }}>
      <div className="plant-card-image-wrapper">
        <div className={`plant-category-pill ${category}`}>
          {category.charAt(0).toUpperCase() + category.slice(1)}
        </div>
        {imageUrl ? (
          <img
            src={imageUrl}
            alt={species.commonName}
            className="plant-card-img"
            loading="lazy"
          />
        ) : (
          <div
            style={{
              width: '100%',
              height: '100%',
              background: 'linear-gradient(45deg, #1e293b, #0f172a)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: '#64748b',
            }}
          >
            <span>No Image</span>
          </div>
        )}
      </div>
      <div className="plant-card-content">
        <h3 className="plant-card-title">{species.commonName || 'Plant'}</h3>
        <p className="plant-card-scientific">{species.scientificName}</p>
        
        {/* Utility Pills */}
        <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem', margin: '0.25rem 0 0.5rem' }}>
          {(species.edibleFruit || species.edibleLeaf || species.cuisine) && (
            <span style={utilityBadgeStyle('rgba(217, 70, 239, 0.2)', '#f0abfc')}>Edible</span>
          )}
          {species.medicinal && (
            <span style={utilityBadgeStyle('rgba(16, 185, 129, 0.2)', '#6ee7b7')}>Medicinal</span>
          )}
          {species.indoor && (
            <span style={utilityBadgeStyle('rgba(59, 130, 246, 0.2)', '#93c5fd')}>Indoor</span>
          )}
          {species.droughtTolerant && (
            <span style={utilityBadgeStyle('rgba(245, 158, 11, 0.2)', '#fcd34d')}>Drought Tolerant</span>
          )}
          {species.invasive && (
            <span style={utilityBadgeStyle('rgba(239, 68, 68, 0.2)', '#fca5a5')}>Invasive</span>
          )}
        </div>

        <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '0.35rem' }}>
          {plant.healthStatus && (
            <span style={chipStyle}>Health: {plant.healthStatus}</span>
          )}
          <span style={chipStyle}>Soil: {plant.soil?.type ?? '—'}</span>
          {plant.heightCm != null && (
            <span style={chipStyle}>{plant.heightCm.toFixed(0)} cm tall</span>
          )}
          {plant.ageDays != null && <span style={chipStyle}>{plant.ageDays} days old</span>}
        </div>
        
        <div style={{ marginTop: '0.75rem', borderTop: '1px solid rgba(255,255,255,0.05)', paddingTop: '0.5rem' }}>
          <p style={{ fontSize: '0.75rem', color: 'rgba(255,255,255,0.5)', display: 'flex', justifyContent: 'space-between' }}>
            <span>Planted:</span>
            <span style={{ color: 'rgba(255,255,255,0.8)' }}>{formatWhen(plant.plantedDate)}</span>
          </p>
          <p style={{ fontSize: '0.75rem', color: 'rgba(255,255,255,0.5)', display: 'flex', justifyContent: 'space-between', marginTop: '0.2rem' }}>
            <span>Last watered:</span>
            <span style={{ color: 'rgba(255,255,255,0.8)' }}>{formatWhen(plant.lastWatered)}</span>
          </p>
        </div>
      </div>
    </div>
  );
};

export default GardenPlantCard;
