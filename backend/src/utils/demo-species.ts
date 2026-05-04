/**
 * Hardcoded species data for six common North Louisiana plants.
 *
 * WHY THIS EXISTS
 * ───────────────
 * The Perenual API is missing key fields the ML model needs, particularly
 * days-to-bloom and accurate temperature/water tolerances. These records fill
 * that gap for demo purposes and show what the system can do with complete data.
 *
 * HOW IT IS USED
 * ──────────────
 * When a prediction or timeline runs for a plant whose species name matches one
 * of these entries, the service substitutes this data into the ML payload
 * instead of whatever the API returned. The `daysToFirstBloom` field also
 * overrides the growth-formula estimate so planting-date calculations are based
 * on real horticultural data rather than a formula approximation.
 *
 * DATA SOURCES
 * ────────────
 * Values were cross-referenced from:
 *  - LSU AgCenter (lsuagcenter.com) — Louisiana-specific planting calendars
 *  - USDA PLANTS Database (plants.usda.gov) — growth habits, bloom periods
 *  - OpenFarm API (openfarm.cc/api/v1/crops) — days to maturity for vegetables
 *  - Trefle API (trefle.io) — already in the stack; bloom months, sowing months
 *  - RHS Plant Finder (rhs.org.uk) — ornamental shrubs and perennials
 *
 * ADDING MORE PLANTS
 * ──────────────────
 * Add an entry to DEMO_SPECIES below. The seed endpoint
 * (POST /backend/prediction/demo/seed) will upsert it into the database.
 */

export interface DemoSpeciesData {
  // ── Identity ───────────────────────────────────────────────────────────────
  commonName: string;
  scientificName: string;
  type: string;            // 'Flower' | 'Vegetable' | 'Tree' | 'Shrub'
  cycle: string;           // 'Annual' | 'Perennial'
  careLevel: string;

  // ── ML model inputs (heights in feet, temps in °F) ─────────────────────────
  growthRate: number;      // 1 = Low, 2 = Medium, 3 = High
  minHeight: number;       // feet
  maxHeight: number;       // feet
  avgHoursSun: number;
  minTemp: number;         // °F
  maxTemp: number;         // °F
  wateringMinDays: number;
  wateringMaxDays: number;
  droughtTolerant: boolean;
  tropical: boolean;

  // ── Taxonomy ───────────────────────────────────────────────────────────────
  family: string;
  genus: string;
  speciesEpithet: string;
  origin: string[];

  // ── Reproduction ───────────────────────────────────────────────────────────
  flowers: boolean;
  floweringSeason: string | null;
  fruits: boolean;
  edibleFruit: boolean;

  // ── Manually researched growth timing ─────────────────────────────────────
  /**
   * Days from planting (seed or transplant) to first visible bloom or harvestable
   * size. Sourced from LSU AgCenter / OpenFarm / RHS — overrides the formula
   * in estimateDaysToMature() so timelines are based on real data.
   */
  daysToFirstBloom: number;

  /**
   * Plain-English advice for planting in North Louisiana (Zone 8a).
   * Shown to users in the "sow info" pop-up.
   */
  sowingInfo: string;

  notes: string;
}

export const DEMO_SPECIES: DemoSpeciesData[] = [
  // ── 1. Flower ─────────────────────────────────────────────────────────────
  {
    commonName: 'Profusion Zinnia',
    scientificName: 'Zinnia elegans',
    type: 'Flower',
    cycle: 'Annual',
    careLevel: 'Easy',
    growthRate: 3,
    minHeight: 1.0,
    maxHeight: 2.0,
    avgHoursSun: 7.5,
    minTemp: 55,
    maxTemp: 100,
    wateringMinDays: 5,
    wateringMaxDays: 7,
    droughtTolerant: true,
    tropical: false,
    family: 'Asteraceae',
    genus: 'Zinnia',
    speciesEpithet: 'elegans',
    origin: ['Mexico'],
    flowers: true,
    floweringSeason: 'Summer',
    fruits: false,
    edibleFruit: false,
    daysToFirstBloom: 60,   // source: LSU AgCenter; 55–65 days seed to first bloom
    sowingInfo:
      'Direct sow after last frost (mid-February in south, early March in north Louisiana). ' +
      'Thin to 6–9 inches apart. No indoor start needed — zinnias dislike transplanting.',
    notes:
      'Profusion Zinnia is one of the most heat- and humidity-tolerant annuals ' +
      'for Louisiana gardens. Blooms continuously from late spring until frost ' +
      'with minimal deadheading. Highly resistant to powdery mildew.',
  },

  // ── 2. Vegetable ──────────────────────────────────────────────────────────
  {
    commonName: 'Creole Tomato',
    scientificName: 'Solanum lycopersicum',
    type: 'Vegetable',
    cycle: 'Annual',
    careLevel: 'Medium',
    growthRate: 2,
    minHeight: 2.0,
    maxHeight: 6.0,
    avgHoursSun: 8.0,
    minTemp: 55,
    maxTemp: 95,
    wateringMinDays: 2,
    wateringMaxDays: 3,
    droughtTolerant: false,
    tropical: false,
    family: 'Solanaceae',
    genus: 'Solanum',
    speciesEpithet: 'lycopersicum',
    origin: ['South America'],
    flowers: true,
    floweringSeason: 'Spring',
    fruits: true,
    edibleFruit: true,
    daysToFirstBloom: 80,   // source: LSU AgCenter; 75–90 days transplant to first ripe fruit
    sowingInfo:
      'Start seeds indoors 6–8 weeks before last frost (early January in north Louisiana). ' +
      'Transplant outdoors after last frost when nighttime temps stay above 50 °F. ' +
      'Stake or cage at planting time.',
    notes:
      'The Creole Tomato is the iconic Louisiana home-garden tomato, prized for ' +
      'its thin skin and rich flavor. Thrives in the hot, humid Louisiana climate. ' +
      'Consistent watering is critical — irregular moisture causes blossom-end rot.',
  },

  // ── 3. Tree ───────────────────────────────────────────────────────────────
  {
    commonName: 'Crape Myrtle',
    scientificName: 'Lagerstroemia indica',
    type: 'Tree',
    cycle: 'Perennial',
    careLevel: 'Easy',
    growthRate: 2,
    minHeight: 6.0,
    maxHeight: 20.0,
    avgHoursSun: 6.5,
    minTemp: 0,
    maxTemp: 105,
    wateringMinDays: 7,
    wateringMaxDays: 14,
    droughtTolerant: true,
    tropical: false,
    family: 'Lythraceae',
    genus: 'Lagerstroemia',
    speciesEpithet: 'indica',
    origin: ['China', 'Southeast Asia'],
    flowers: true,
    floweringSeason: 'Summer',
    fruits: false,
    edibleFruit: false,
    daysToFirstBloom: 730,  // source: RHS; standard-size varieties bloom years 2–3 from a 1-gal nursery plant
    sowingInfo:
      'Plant container-grown trees in spring or fall. Avoid planting in summer heat. ' +
      'Full sun is essential for best flowering. Do NOT top ("crape murder") — ' +
      'natural vase shape produces more blooms and better structure.',
    notes:
      'Crape Myrtle is the quintessential Louisiana landscape tree, blooming ' +
      'pink/red/white/purple from June through September. Extremely tolerant ' +
      'of heat, humidity, and drought once established.',
  },

  // ── 4. Shrub ──────────────────────────────────────────────────────────────
  {
    commonName: 'Southern Azalea',
    scientificName: 'Rhododendron canescens',
    type: 'Shrub',
    cycle: 'Perennial',
    careLevel: 'Easy',
    growthRate: 1,          // slow-growing shrub, 1–2 ft/year
    minHeight: 4.0,
    maxHeight: 10.0,
    avgHoursSun: 4.5,       // partial shade preferred; direct afternoon sun scorches leaves
    minTemp: 5,             // °F — hardy throughout Louisiana
    maxTemp: 95,            // °F — heat tolerant with moisture
    wateringMinDays: 5,
    wateringMaxDays: 7,
    droughtTolerant: false,
    tropical: false,
    family: 'Ericaceae',
    genus: 'Rhododendron',
    speciesEpithet: 'canescens',
    origin: ['North America'],
    flowers: true,
    floweringSeason: 'Spring',
    fruits: false,
    edibleFruit: false,
    daysToFirstBloom: 365,  // source: LSU AgCenter; nursery plant planted in fall blooms the following spring (~1 year)
    sowingInfo:
      'Plant container-grown shrubs in fall (October–November) or early spring. ' +
      'Acidic, well-drained soil is critical (pH 4.5–6.0). Mulch heavily to keep ' +
      'roots cool and moist. Avoid planting near concrete foundations (raises soil pH).',
    notes:
      'The native Piedmont Azalea (R. canescens) is one of the most spectacular ' +
      'flowering shrubs in Louisiana, covering itself in fragrant pink blooms in ' +
      'early spring before the leaves emerge. Native populations are found throughout ' +
      'north and central Louisiana along streams and woodland edges.',
  },

  // ── 5. Flower (Perennial) ─────────────────────────────────────────────────
  {
    commonName: 'Louisiana Iris',
    scientificName: 'Iris louisiana',
    type: 'Flower',
    cycle: 'Perennial',
    careLevel: 'Easy',
    growthRate: 2,
    minHeight: 2.0,
    maxHeight: 4.0,
    avgHoursSun: 6.0,
    minTemp: 0,             // °F — remarkably cold-hardy for a Louisiana plant
    maxTemp: 100,
    wateringMinDays: 2,
    wateringMaxDays: 4,     // loves wet conditions; native to bayous and swamps
    droughtTolerant: false,
    tropical: false,
    family: 'Iridaceae',
    genus: 'Iris',
    speciesEpithet: 'louisiana',
    origin: ['United States'],
    flowers: true,
    floweringSeason: 'Spring',
    fruits: false,
    edibleFruit: false,
    daysToFirstBloom: 210,  // source: LSU AgCenter; rhizomes planted Aug–Oct bloom the following spring (~7 months)
    sowingInfo:
      'Plant rhizomes August through October in shallow water or consistently moist soil. ' +
      'Set rhizomes just below the soil surface. Louisiana Iris thrive in bog gardens, ' +
      'pond margins, and low-lying areas that collect water.',
    notes:
      'Louisiana Iris is the official state wildflower of Louisiana, naturally ' +
      'occurring in the marshes and swamps of the Gulf Coast. Hybridized varieties ' +
      'come in virtually every color. One of the few plants that actually prefers ' +
      'the waterlogged conditions common in Louisiana gardens.',
  },

  // ── 6. Vegetable ──────────────────────────────────────────────────────────
  {
    commonName: 'Okra',
    scientificName: 'Abelmoschus esculentus',
    type: 'Vegetable',
    cycle: 'Annual',
    careLevel: 'Easy',
    growthRate: 3,          // fast — one of the quickest vegetables in Louisiana heat
    minHeight: 3.0,
    maxHeight: 6.0,
    avgHoursSun: 8.0,
    minTemp: 65,            // °F — tropical crop, chilled below 55 °F
    maxTemp: 105,           // °F — thrives in extreme Louisiana summer heat
    wateringMinDays: 3,
    wateringMaxDays: 5,
    droughtTolerant: true,  // moderately drought-tolerant once established
    tropical: true,
    family: 'Malvaceae',
    genus: 'Abelmoschus',
    speciesEpithet: 'esculentus',
    origin: ['Africa'],
    flowers: true,
    floweringSeason: 'Summer',
    fruits: true,
    edibleFruit: true,
    daysToFirstBloom: 55,   // source: OpenFarm API + LSU AgCenter; 50–60 days seed to first pod
    sowingInfo:
      'Direct sow after last frost when soil temperature reaches 65 °F (late March–April ' +
      'in north Louisiana). Soak seeds overnight for faster germination. Plant 1 inch deep, ' +
      '12–18 inches apart. Harvest pods when 3–4 inches long — they toughen quickly.',
    notes:
      'Okra is a Louisiana staple crop and one of the best vegetables for the brutal ' +
      'summer heat. The hibiscus-like flowers are ornamental as well as productive. ' +
      'A single plant can produce dozens of pods per season. Essential for gumbo.',
  },
];

/**
 * Returns the demo species data for a plant if its species commonName or
 * scientificName matches one of the demo entries, otherwise returns null.
 */
export function getDemoSpecies(
  commonName: string,
  scientificName: string,
): DemoSpeciesData | null {
  return (
    DEMO_SPECIES.find(
      (d) =>
        d.commonName.toLowerCase() === commonName.toLowerCase() ||
        d.scientificName.toLowerCase() === scientificName.toLowerCase(),
    ) ?? null
  );
}
