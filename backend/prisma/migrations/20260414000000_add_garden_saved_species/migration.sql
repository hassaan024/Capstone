-- CreateTable
CREATE TABLE "_GardenSavedSpecies" (
    "A" INTEGER NOT NULL,
    "B" INTEGER NOT NULL,
    CONSTRAINT "_GardenSavedSpecies_A_fkey" FOREIGN KEY ("A") REFERENCES "Garden"("id") ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT "_GardenSavedSpecies_B_fkey" FOREIGN KEY ("B") REFERENCES "Species"("id") ON DELETE CASCADE ON UPDATE CASCADE
);

-- CreateIndex
CREATE UNIQUE INDEX "_GardenSavedSpecies_AB_unique" ON "_GardenSavedSpecies"("A", "B");

-- CreateIndex
CREATE INDEX "_GardenSavedSpecies_B_index" ON "_GardenSavedSpecies"("B");
