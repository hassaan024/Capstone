import { ModelCategory } from 'enums/table_enums';

export function computeModelCategory(species: {
  type?: string | null;
  cycle?: string | null;
  edibleFruit?: boolean | null;
  edibleLeaf?: boolean | null;
  cuisine?: boolean | null;
  commonName?: string | null;
  scientificName?: string | null;
}): ModelCategory {
  const typeStr = (species.type || species.cycle || '').toLowerCase();

  if (typeStr.includes('flower')) return ModelCategory.flower;
  if (typeStr.includes('tree')) return ModelCategory.tree;

  if (
    typeStr.includes('vegetable') ||
    species.edibleFruit ||
    species.edibleLeaf ||
    species.cuisine ||
    typeStr.includes('herb') ||
    typeStr.includes('fruit') ||
    species.commonName?.toLowerCase().includes('tomato')
  ) {
    return ModelCategory.vegetable;
  }

  if (species.scientificName?.includes('Malus')) return ModelCategory.tree;

  return ModelCategory.flower;
}
