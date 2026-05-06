-- CreateTable
CREATE TABLE IF NOT EXISTS "_UserSavedSpecies" (
    "A" INTEGER NOT NULL,
    "B" INTEGER NOT NULL
);

-- CreateIndex
CREATE UNIQUE INDEX IF NOT EXISTS "_UserSavedSpecies_AB_unique" ON "_UserSavedSpecies"("A", "B");

-- CreateIndex
CREATE INDEX IF NOT EXISTS "_UserSavedSpecies_B_index" ON "_UserSavedSpecies"("B");

-- AddForeignKey
ALTER TABLE "_UserSavedSpecies" ADD CONSTRAINT "_UserSavedSpecies_A_fkey" FOREIGN KEY ("A") REFERENCES "Species"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "_UserSavedSpecies" ADD CONSTRAINT "_UserSavedSpecies_B_fkey" FOREIGN KEY ("B") REFERENCES "User"("id") ON DELETE CASCADE ON UPDATE CASCADE;
