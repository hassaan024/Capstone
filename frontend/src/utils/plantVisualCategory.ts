/**
 * Mirrors backend species → Flower | Vegetable | Tree rules.
 *
 * Rules (kept in sync with computeModelCategory / enrichWithModelCategory in backend):
 *  - Vegetable if edible flags or vegetable/herb/fruit in type string
 *  - Tree ONLY via scientific name containing "Malus" (apple trees)
 *  - Flower is the default (mirrors browse list API which has no `type` field)
 */
export type VisualCategory = 'flower' | 'vegetable' | 'tree';

export type PlantVisualInput = {
  type?: string | null;
  cycle?: string | null;
  scientificName?: string | null;
  flowers?: boolean | null;
  cuisine?: boolean | null;
  edibleFruit?: boolean | null;
  edibleLeaf?: boolean | null;
  commonName?: string | null;
};

export function mapPlantToVisualCategory(species: PlantVisualInput): VisualCategory {
  const t = ((species.type ?? '') + ' ' + (species.cycle ?? '')).toLowerCase().trim();

  // If Perenual explicitly tagged it as a flower, trust that over edible flags
  if (t.includes('flower')) return 'flower';

  // Tree: Perenual explicitly tagged it as a tree (takes priority over edible signals)
  if (t.includes('tree')) return 'tree';

  // Vegetable: edible signals OR vegetable/herb/fruit in type string
  const cuisine = Boolean(species.cuisine);
  const edibleFruit = Boolean(species.edibleFruit);
  const edibleLeaf = Boolean(species.edibleLeaf);
  const vegByFlags = cuisine || edibleFruit || edibleLeaf;
  const vegByType = ['vegetable', 'herb', 'fruit'].some((k) => t.includes(k));
  const isTomato = (species.commonName ?? '').toLowerCase().includes('tomato');
  if (vegByFlags || vegByType || isTomato) return 'vegetable';

  // Tree fallback: only via scientific name (Malus = apple trees)
  if ((species.scientificName ?? '').includes('Malus')) return 'tree';

  // Default: flower
  return 'flower';
}
