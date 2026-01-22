/*
  Warnings:

  - You are about to drop the column `perenualId` on the `Species` table. All the data in the column will be lost.
  - A unique constraint covering the columns `[trefleId]` on the table `Species` will be added. If there are existing duplicate values, this will fail.

*/
-- DropIndex
DROP INDEX "Species_perenualId_key";

-- AlterTable
ALTER TABLE "Species" DROP COLUMN "perenualId",
ADD COLUMN     "trefleId" INTEGER;

-- CreateIndex
CREATE UNIQUE INDEX "Species_trefleId_key" ON "Species"("trefleId");
