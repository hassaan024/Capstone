import React from 'react';
import { mapPlantToVisualCategory } from '../utils/plantVisualCategory';

interface PlantStageTrackerCardProps {
  plant: {
    id: number;
    plantedDate?: string | null;
    species: any;
  };
  currentTimestamp: number;   // The simulated time
  bloomTimestamp: number;     // The target time it should be at Stage 4
}

const getStageEmojis = (category: string) => {
  switch (category) {
    case 'tree':
      return ['🌰', '🌱', '🌿', '🌳'];
    case 'flower':
      return ['🌰', '🌱', '🪴', '🌺'];
    case 'vegetable':
    case 'herb':
      return ['🫘', '🌱', '🪴', '🍅'];
    case 'succulent':
    case 'cactus':
      return ['🌵', '🌱', '🪴', '🌵'];
    default:
      return ['🫘', '🌱', '🌿', '🪴']; // Generic fallback
  }
};

const PlantStageTrackerCard: React.FC<PlantStageTrackerCardProps> = ({ plant, currentTimestamp, bloomTimestamp }) => {
  const { species, plantedDate } = plant;
  
  const category = mapPlantToVisualCategory({
    type: species.type,
    cycle: species.cycle,
    scientificName: species.scientificName,
    commonName: species.commonName,
    cuisine: species.cuisine,
    edibleFruit: species.edibleFruit,
    edibleLeaf: species.edibleLeaf,
  });

  const emojis = getStageEmojis(category);
  
  // Logic to calculate progress
  let progress = 0;
  let stageIndex = 0;
  
  if (plantedDate) {
    const plantedTime = new Date(plantedDate).getTime();
    const totalDuration = bloomTimestamp - plantedTime;
    
    if (totalDuration > 0) {
      const elapsed = currentTimestamp - plantedTime;
      if (elapsed < 0) {
        progress = elapsed / totalDuration; // Keeps it negative
      } else {
        progress = Math.max(0, Math.min(1, elapsed / totalDuration));
      }
    } else {
      // If planted on or after bloom date, immediately fully bloomed
      progress = 1;
    }
  }

  // Determine stage (0 to 3 index for array)
  if (progress < 0) {
    stageIndex = -1; // Not yet planted
  } else if (progress < 0.25) {
    stageIndex = 0;
  } else if (progress < 0.50) {
    stageIndex = 1;
  } else if (progress < 0.75) {
    stageIndex = 2;
  } else {
    stageIndex = 3;
  }

  return (
    <div style={{ position: 'relative', width: '100%', height: '100%' }}>
      {stageIndex < 0 && plantedDate && (
        <div style={{
          position: 'absolute',
          top: '50%',
          left: '50%',
          transform: 'translate(-50%, -50%)',
          background: 'rgba(59, 130, 246, 0.9)',
          backdropFilter: 'blur(4px)',
          border: '1px solid rgba(147, 197, 253, 0.5)',
          color: 'white',
          padding: '0.75rem 1.25rem',
          borderRadius: '12px',
          fontWeight: 'bold',
          fontSize: '1rem',
          textAlign: 'center',
          zIndex: 20,
          boxShadow: '0 4px 20px rgba(0,0,0,0.6)',
          pointerEvents: 'none',
          whiteSpace: 'nowrap'
        }}>
          <div style={{ fontSize: '0.75rem', textTransform: 'uppercase', letterSpacing: '0.05em', opacity: 0.9, marginBottom: '0.2rem' }}>Plant On</div>
          <div style={{ fontSize: '1.25rem', color: '#eff6ff' }}>
            {new Date(plantedDate).toLocaleDateString(undefined, {month: 'short', day: 'numeric', year: 'numeric'})}
          </div>
        </div>
      )}
      <div className={`plant-card ${stageIndex < 0 ? 'inactive' : ''}`} style={{ 
        transition: 'all 0.3s ease',
        opacity: stageIndex < 0 ? 0.3 : 1,
        transform: stageIndex < 0 ? 'scale(0.95)' : 'scale(1)',
        cursor: 'default',
        margin: 0,
        height: '100%'
      }}>
        <div style={{
        position: 'relative',
        width: '100%',
        height: '160px',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        background: 'linear-gradient(135deg, rgba(30,41,59,0.5), rgba(15,23,42,0.8))',
        borderBottom: '1px solid rgba(255,255,255,0.05)',
        overflow: 'hidden'
      }}>
        <div className={`plant-category-pill ${category}`} style={{ position: 'absolute', top: '10px', left: '10px', zIndex: 10 }}>
          {category.charAt(0).toUpperCase() + category.slice(1)}
        </div>
        
        <div style={{
           position: 'absolute',
           top: '10px',
           right: '10px',
           background: 'rgba(255,255,255,0.1)',
           padding: '2px 8px',
           borderRadius: '12px',
           fontSize: '0.7rem',
           fontWeight: 'bold',
           color: 'rgba(255,255,255,0.8)'
        }}>
          {stageIndex < 0 ? 'Upcoming' : `Stage ${stageIndex + 1}/4`}
        </div>

        <div style={{ 
          fontSize: '4.5rem', 
          filter: 'drop-shadow(0 4px 6px rgba(0,0,0,0.5))',
          transform: `scale(${stageIndex < 0 ? 0.8 : 1 + (progress * 0.2)})`,
          transition: 'transform 0.5s cubic-bezier(0.34, 1.56, 0.64, 1)'
        }}>
          {stageIndex < 0 ? '⏳' : emojis[stageIndex]}
        </div>
        
        {/* Progress Bar inside visual box */}
        <div style={{
          position: 'absolute',
          bottom: 0,
          left: 0,
          width: '100%',
          height: '4px',
          background: 'rgba(255,255,255,0.1)'
        }}>
          <div style={{
            height: '100%',
            width: `${stageIndex < 0 ? 0 : progress * 100}%`,
            background: progress >= 1 ? '#4ade80' : '#60a5fa',
            transition: 'width 0.4s ease'
          }} />
        </div>
      </div>
      
      <div className="plant-card-content">
        <h3 className="plant-card-title">{species.commonName || 'Plant'}</h3>
        <p className="plant-card-scientific" style={{ marginBottom: '0.5rem' }}>{species.scientificName}</p>
        
        <div style={{ display: 'grid', gap: '0.25rem', fontSize: '0.8rem', color: 'rgba(255,255,255,0.6)' }}>
          {plantedDate && (
            <div style={{ display: 'flex', justifyContent: 'space-between' }}>
              <span>Plant Date:</span>
              <span style={{ color: 'rgba(255,255,255,0.8)' }}>
                {new Date(plantedDate).toLocaleDateString(undefined, {month: 'short', day: 'numeric', year: 'numeric'})}
              </span>
            </div>
          )}
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span>State:</span>
            <span style={{ color: stageIndex < 0 ? 'rgba(255,255,255,0.5)' : progress >= 1 ? '#86efac' : '#bae6fd', fontWeight: 600 }}>
              {stageIndex < 0 ? 'Not Planted' 
                : stageIndex === 0 ? 'Seed / Sprouting'
                : stageIndex === 1 ? 'Early Growth'
                : stageIndex === 2 ? 'Maturing'
                : 'Fully Bloomed'}
            </span>
          </div>
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span>Growth:</span>
            <span style={{ color: 'rgba(255,255,255,0.8)' }}>
              {stageIndex < 0 ? '0%' : `${Math.round(progress * 100)}%`}
            </span>
          </div>
        </div>
      </div>
    </div>
    </div>
  );
};

export default PlantStageTrackerCard;
