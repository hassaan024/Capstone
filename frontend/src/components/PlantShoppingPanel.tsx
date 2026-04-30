import React, { useEffect, useState } from 'react';
import { api } from '../utils/api';
import { FaShoppingBag, FaExternalLinkAlt } from 'react-icons/fa';
import '../styles/PlantShopping.css';

interface PlantShoppingPanelProps {
  plantName: string;
  plantId: number;
}

interface ShoppingLink {
  retailer: string;
  url: string;
  icon: string;
  color: string;
  tagline: string;
}

interface PriceDataPoint {
  month: string;
  avgPrice: number;
  lowPrice: number;
  highPrice: number;
}

interface PriceHistoryResponse {
  data: PriceDataPoint[];
  estimatedRange: { low: number; high: number };
}

const PlantShoppingPanel: React.FC<PlantShoppingPanelProps> = ({ plantName, plantId }) => {
  const [links, setLinks] = useState<ShoppingLink[]>([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    if (!plantName || !plantId) return;

    setLoading(true);

    const fetchLinks = api.get(`/plant-shopping/links/${encodeURIComponent(plantName)}`)
      .then((res: { data: ShoppingLink[] }) => setLinks(res.data))
      .catch((err: Error) => console.error('Failed to fetch shopping links', err));

    Promise.all([fetchLinks]).finally(() => setLoading(false));
  }, [plantName, plantId]);

  if (loading) {
    return (
      <div className="shopping-section">
        <div className="shopping-loading">
          <div className="shopping-loading-spinner"></div>
          <p>Loading shopping data...</p>
        </div>
      </div>
    );
  }


  return (
    <div className="shopping-section">
      {/* Header */}
      <div className="shopping-header">
        <span className="shopping-header-icon"><FaShoppingBag /></span>
        <h3>Where to Buy</h3>
      </div>

      {/* Retailer Cards */}
      {links.length > 0 && (
        <div className="retailer-grid">
          {links.map((link) => (
            <a
              key={link.retailer}
              href={link.url}
              target="_blank"
              rel="noopener noreferrer"
              className="retailer-card"
              style={{ '--accent': link.color } as React.CSSProperties}
            >
              <span style={{ borderColor: `${link.color}33` }} className="retailer-icon">{link.icon}</span>
              <div className="retailer-info">
                <p className="retailer-name">{link.retailer}</p>
                <p className="retailer-tagline">{link.tagline}</p>
              </div>
              <FaExternalLinkAlt className="retailer-arrow" />
            </a>
          ))}
        </div>
      )}
    </div>
  );
};

export default PlantShoppingPanel;
