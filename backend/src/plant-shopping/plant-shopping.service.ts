import { Injectable } from '@nestjs/common';

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

@Injectable()
export class PlantShoppingService {
  /**
   * Generate search URLs for major plant retailers.
   */
  getShoppingLinks(plantName: string): ShoppingLink[] {
    const encoded = encodeURIComponent(`${plantName} plant`);

    return [
      {
        retailer: 'Amazon',
        url: `https://www.amazon.com/s?k=${encoded}`,
        icon: '📦',
        color: '#FF9900',
        tagline: 'Fast shipping & reviews',
      },
      {
        retailer: 'Walmart',
        url: `https://www.walmart.com/search?q=${encoded}`,
        icon: '🛒',
        color: '#0071CE',
        tagline: 'Everyday low prices',
      },
    ];
  }

  /**
   * Generate simulated 12-month price history for a plant.
   * Uses a deterministic seed based on plantId so the same plant always
   * shows the same chart (no randomness between page loads).
   */
  getPriceHistory(plantId: number): { data: PriceDataPoint[]; estimatedRange: { low: number; high: number } } {
    const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];

    // Deterministic pseudo-random based on plantId
    const seed = (plantId * 2654435761) >>> 0; // Knuth multiplicative hash
    const seededRandom = (i: number): number => {
      const x = Math.sin(seed + i * 127.1) * 43758.5453;
      return x - Math.floor(x); // 0..1
    };

    // Base price between $5 and $45 depending on plant
    const basePrice = 5 + seededRandom(0) * 40;

    const data: PriceDataPoint[] = months.map((month, i) => {
      // Seasonal variation: plants cost more in spring (Mar-May)
      const seasonalBoost = (i >= 2 && i <= 4) ? 1.15 : (i >= 9 ? 0.9 : 1.0);
      const noise = (seededRandom(i + 1) - 0.5) * 8; // +/- $4 noise
      const avgPrice = Math.round((basePrice * seasonalBoost + noise) * 100) / 100;
      const spread = 2 + seededRandom(i + 100) * 6;
      const lowPrice = Math.round((avgPrice - spread) * 100) / 100;
      const highPrice = Math.round((avgPrice + spread) * 100) / 100;

      return {
        month,
        avgPrice: Math.max(1, avgPrice),
        lowPrice: Math.max(1, lowPrice),
        highPrice: Math.max(1, highPrice),
      };
    });

    const allLows = data.map(d => d.lowPrice);
    const allHighs = data.map(d => d.highPrice);

    return {
      data,
      estimatedRange: {
        low: Math.min(...allLows),
        high: Math.max(...allHighs),
      },
    };
  }
}
