-- AlterTable
ALTER TABLE "_GardenSavedSpecies" ADD CONSTRAINT "_GardenSavedSpecies_AB_pkey" PRIMARY KEY ("A", "B");

-- DropIndex
DROP INDEX "_GardenSavedSpecies_AB_unique";
