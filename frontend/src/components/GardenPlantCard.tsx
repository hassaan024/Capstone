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
  imgSrcUrls?: { regular?: string | null } | Record<string, unknown> | null;
}

export interface GardenPlantCardProps {
  plant: {
    id: number;
    heightCm?: number | null;
    ageDays?: number | null;
    healthStatus?: string | null;
    lastWatered?: string | null;
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
    return new Date(iso).toLocaleString(undefined, {
      dateStyle: 'medium',
      timeStyle: 'short',
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

const GardenPlantCard: React.FC<GardenPlantCardProps> = ({ plant }) => {
  const { species } = plant;
  const category = mapPlantToVisualCategory({
    type: species.type,
    flowers: species.flowers,
    cuisine: species.cuisine,
    edibleFruit: species.edibleFruit,
    edibleLeaf: species.edibleLeaf,
  });
  const imageUrl = imageFromSpecies(species);
  const notesPreview =
    plant.notes && plant.notes.length > 80 ? `${plant.notes.slice(0, 80)}…` : plant.notes;

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
        <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem', marginTop: '0.25rem' }}>
          {plant.healthStatus && (
            <span style={chipStyle}>Health: {plant.healthStatus}</span>
          )}
          <span style={chipStyle}>Soil: {plant.soil?.type ?? '—'}</span>
          {plant.heightCm != null && (
            <span style={chipStyle}>{plant.heightCm.toFixed(0)} cm</span>
          )}
          {plant.ageDays != null && <span style={chipStyle}>{plant.ageDays} days</span>}
        </div>
        <p style={{ fontSize: '0.75rem', color: 'rgba(255,255,255,0.5)', marginTop: '0.5rem' }}>
          Last watered: {formatWhen(plant.lastWatered ?? undefined)}
        </p>
        {notesPreview && (
          <p
            style={{
              fontSize: '0.8rem',
              color: 'rgba(255,255,255,0.65)',
              marginTop: '0.35rem',
              lineHeight: 1.35,
            }}
          >
            {notesPreview}
          </p>
        )}
      </div>
    </div>
  );
};

export default GardenPlantCard;
