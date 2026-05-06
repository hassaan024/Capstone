-- DropIndex
DROP INDEX "Species_commonName_key";

-- DropIndex
DROP INDEX "Species_scientificName_key";

-- AlterTable
ALTER TABLE "_UserSavedSpecies" ADD CONSTRAINT "_UserSavedSpecies_AB_pkey" PRIMARY KEY ("A", "B");

-- DropIndex
DROP INDEX "_UserSavedSpecies_AB_unique";
