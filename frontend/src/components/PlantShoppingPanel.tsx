import React, { useEffect, useState } from 'react';
import { api } from '../utils/api';
import { AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import { FaShoppingBag, FaChartLine, FaExternalLinkAlt } from 'react-icons/fa';
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
  const [priceData, setPriceData] = useState<PriceHistoryResponse | null>(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    if (!plantName || !plantId) return;

    setLoading(true);

    const fetchLinks = api.get(`/plant-shopping/links/${encodeURIComponent(plantName)}`)
      .then((res: { data: ShoppingLink[] }) => setLinks(res.data))
      .catch((err: Error) => console.error('Failed to fetch shopping links', err));

    const fetchPrices = api.get(`/plant-shopping/price-history/${plantId}`)
      .then((res: { data: PriceHistoryResponse }) => setPriceData(res.data))
      .catch((err: Error) => console.error('Failed to fetch price history', err));

    Promise.all([fetchLinks, fetchPrices]).finally(() => setLoading(false));
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

  const CustomTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
      return (
        <div style={{
          background: 'rgba(15, 23, 42, 0.95)',
          border: '1px solid rgba(148, 163, 184, 0.3)',
          borderRadius: '10px',
          padding: '0.75rem 1rem',
          boxShadow: '0 8px 16px rgba(0,0,0,0.4)',
        }}>
          <p style={{ color: '#e2e8f0', fontWeight: 600, marginBottom: '0.35rem', fontSize: '0.85rem' }}>{label}</p>
          <p style={{ color: '#86efac', margin: '0.15rem 0', fontSize: '0.8rem' }}>
            Avg: ${payload[0]?.value?.toFixed(2)}
          </p>
          <p style={{ color: '#94a3b8', margin: '0.15rem 0', fontSize: '0.75rem' }}>
            Range: ${payload[1]?.value?.toFixed(2)} – ${payload[2]?.value?.toFixed(2)}
          </p>
        </div>
      );
    }
    return null;
  };

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

      {/* Price History Chart */}
      {priceData && priceData.data.length > 0 && (
        <div className="price-chart-section">
          <div className="price-chart-header">
            <span className="price-chart-title">
              <FaChartLine /> Price Trends (12 months)
            </span>
            <span className="price-chart-badge">Estimated</span>
          </div>

          <div className="price-chart-container">
            <ResponsiveContainer width="100%" height={220}>
              <AreaChart data={priceData.data} margin={{ top: 10, right: 20, left: 10, bottom: 0 }}>
                <defs>
                  <linearGradient id="avgGradient" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="5%" stopColor="#10b981" stopOpacity={0.3} />
                    <stop offset="95%" stopColor="#10b981" stopOpacity={0.02} />
                  </linearGradient>
                  <linearGradient id="rangeGradient" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="5%" stopColor="#8b5cf6" stopOpacity={0.12} />
                    <stop offset="95%" stopColor="#8b5cf6" stopOpacity={0.02} />
                  </linearGradient>
                </defs>
                <CartesianGrid strokeDasharray="3 3" stroke="rgba(148,163,184,0.08)" />
                <XAxis
                  dataKey="month"
                  tick={{ fill: 'rgba(148,163,184,0.6)', fontSize: 12 }}
                  axisLine={{ stroke: 'rgba(148,163,184,0.1)' }}
                  tickLine={false}
                />
                <YAxis
                  tick={{ fill: 'rgba(148,163,184,0.6)', fontSize: 12 }}
                  axisLine={{ stroke: 'rgba(148,163,184,0.1)' }}
                  tickLine={false}
                  tickFormatter={(v: number) => `$${v}`}
                />
                <Tooltip content={<CustomTooltip />} />
                <Area
                  type="monotone"
                  dataKey="avgPrice"
                  stroke="#10b981"
                  strokeWidth={2.5}
                  fill="url(#avgGradient)"
                  dot={false}
                  activeDot={{ r: 5, fill: '#10b981', stroke: '#0f172a', strokeWidth: 2 }}
                />
                <Area
                  type="monotone"
                  dataKey="lowPrice"
                  stroke="transparent"
                  fill="url(#rangeGradient)"
                  dot={false}
                />
                <Area
                  type="monotone"
                  dataKey="highPrice"
                  stroke="rgba(139,92,246,0.3)"
                  strokeWidth={1}
                  strokeDasharray="4 4"
                  fill="transparent"
                  dot={false}
                />
              </AreaChart>
            </ResponsiveContainer>
          </div>

          {/* Price Range Summary */}
          <div className="price-range-summary">
            <div className="price-range-item">
              <div className="price-range-label">Low</div>
              <div className="price-range-value">${priceData.estimatedRange.low.toFixed(2)}</div>
            </div>
            <div className="price-range-divider"></div>
            <div className="price-range-item">
              <div className="price-range-label">High</div>
              <div className="price-range-value">${priceData.estimatedRange.high.toFixed(2)}</div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default PlantShoppingPanel;
