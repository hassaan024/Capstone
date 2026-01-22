import React from 'react';

interface PlantCardProps {
  plant: {
    id: number;
    common_name: string;
    scientific_name: string;
    image_url?: string;
    family_common_name?: string;
  };
  onClick: () => void;
}

const PlantCard: React.FC<PlantCardProps> = ({ plant, onClick }) => {
  return (
    <div className="plant-card" onClick={onClick}>
      <div className="plant-card-image-wrapper">
        {plant.image_url ? (
          <img 
            src={plant.image_url} 
            alt={plant.common_name} 
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
        <h3 className="plant-card-title">{plant.common_name || 'Unknown Plant'}</h3>
        <p className="plant-card-scientific">{plant.scientific_name}</p>
        <span className="plant-card-family">
          {plant.family_common_name || 'General'}
        </span>
      </div>
    </div>
  );
};

export default PlantCard;
