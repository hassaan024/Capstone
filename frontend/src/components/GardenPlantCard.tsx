import React from 'react';
import { mapPlantToVisualCategory } from '../utils/plantVisualCategory';

export interface GardenPlantCardSpecies {
  commonName: string;
  scientificName: string;
  family?: string | null;
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
  modelCategory?: string | null;
  bloomDays?: number | null;
  perenualId?: number | null;
}

export interface GardenPlantCardProps {
  plant: {
    id: number;
    heightCm?: number | null;
    ageDays?: number | null;
    healthStatus?: string | null;
    lastWatered?: string | null;
    plantedDate?: string | null;
    bloomDate?: string | null;      // actual bloom date saved by Unreal
    notes?: string | null;
    species: GardenPlantCardSpecies;
    soil?: { type: string };
  };
  onClick?: () => void;
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

const GardenPlantCard: React.FC<GardenPlantCardProps> = ({ plant, onClick }) => {
  const { species } = plant;
  // If backend provided the computed modelCategory, use it. Otherwise compute fallback.
  const category = (species as any).modelCategory || mapPlantToVisualCategory({
    type: species.type,
    scientificName: species.scientificName,
    commonName: species.commonName,
    cuisine: species.cuisine,
    edibleFruit: species.edibleFruit,
    edibleLeaf: species.edibleLeaf,
  });
  const imageUrl = imageFromSpecies(species);

  // Use the actual bloom date saved by Unreal on the plant instance.
  // Fall back to computing from plantedDate + bloomDays only when the
  // instance bloom date is absent (e.g. legacy records).
  let bloomDateStr: string | null = plant.bloomDate ?? null;
  if (!bloomDateStr && plant.plantedDate && species.bloomDays) {
    const pDate = new Date(plant.plantedDate);
    pDate.setDate(pDate.getDate() + species.bloomDays);
    bloomDateStr = pDate.toISOString();
  }

  return (
    <div className={`plant-card ${onClick ? 'clickable' : ''}`} onClick={onClick} style={{ cursor: onClick ? 'pointer' : 'default' }}>
      <div className="plant-card-image-wrapper">
        {category && (
          <div className={`plant-category-pill ${category.toLowerCase()}`}>
            {category.charAt(0).toUpperCase() + category.slice(1)}
          </div>
        )}
        {species.bloomDays && (
          <div className="plant-category-pill" style={{ background: 'rgba(56, 189, 248, 0.9)', right: '0.5rem', left: 'auto' }}>
            ~{species.bloomDays} Days
          </div>
        )}
        {imageUrl ? (
          <img 
            src={imageUrl} 
            alt={species.commonName} 
            className="plant-card-img"
            loading="lazy"
          />
        ) : (
          <div style={{
            width: '100%', 
            height: '100%', 
            background: 'linear-gradient(45deg, #1e293b, #0f172a)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            color: '#64748b'
          }}>
            <span>No Image</span>
          </div>
        )}
      </div>
      <div className="plant-card-content">
        <h3 className="plant-card-title">{species.commonName || 'Unknown Plant'}</h3>
        <p className="plant-card-scientific">{species.scientificName}</p>
        <span className="plant-card-family">
          {species.family || 'General'}
        </span>
        
        <div style={{ marginTop: '0.75rem', borderTop: '1px solid rgba(255,255,255,0.05)', paddingTop: '0.5rem' }}>
          <p style={{ fontSize: '0.75rem', color: 'rgba(255,255,255,0.5)', display: 'flex', justifyContent: 'space-between' }}>
            <span>Plant Date:</span>
            <span style={{ color: 'rgba(255,255,255,0.8)' }}>{formatWhen(plant.plantedDate)}</span>
          </p>
          <p style={{ fontSize: '0.75rem', color: 'rgba(255,255,255,0.5)', display: 'flex', justifyContent: 'space-between', marginTop: '0.2rem' }}>
            <span>Bloom Date:</span>
            <span style={{ color: 'rgba(255,255,255,0.8)' }}>{formatWhen(bloomDateStr)}</span>
          </p>
        </div>
      </div>
    </div>
  );
};

export default GardenPlantCard;
