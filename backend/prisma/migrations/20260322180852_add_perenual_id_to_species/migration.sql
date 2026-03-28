/*
  Warnings:

  - A unique constraint covering the columns `[perenualId]` on the table `Species` will be added. If there are existing duplicate values, this will fail.

*/
-- AlterTable
ALTER TABLE "Species" ADD COLUMN     "perenualId" INTEGER;

-- CreateIndex
CREATE UNIQUE INDEX "Species_perenualId_key" ON "Species"("perenualId");

-- CreateIndex
CREATE INDEX "Species_commonName_idx" ON "Species"("commonName");
