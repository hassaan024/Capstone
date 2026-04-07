/** Mirrors backend species → Flower | Vegetable | Tree rules. */
export type VisualCategory = 'flower' | 'vegetable' | 'tree';

export type PlantVisualInput = {
  type?: string | null;
  flowers?: boolean | null;
  cuisine?: boolean | null;
  edibleFruit?: boolean | null;
  edibleLeaf?: boolean | null;
};

export function mapPlantToVisualCategory(species: PlantVisualInput): VisualCategory {
  const t = (species.type ?? '').toLowerCase();
  if (t.includes('tree')) return 'tree';

  const cuisine = Boolean(species.cuisine);
  const edibleFruit = Boolean(species.edibleFruit);
  const edibleLeaf = Boolean(species.edibleLeaf);
  const flowers = Boolean(species.flowers);

  const vegByFlags = cuisine || edibleFruit || edibleLeaf;
  const vegByType = ['herb', 'vegetable', 'fruit', 'veggie'].some((k) => t.includes(k));
  if (vegByFlags || vegByType) return 'vegetable';

  if (flowers && !(cuisine || edibleFruit || edibleLeaf)) return 'flower';

  return 'tree';
}
