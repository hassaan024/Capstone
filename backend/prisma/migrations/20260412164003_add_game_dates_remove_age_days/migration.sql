/*
  Warnings:

  - You are about to drop the column `ageDays` on the `PlantInstance` table. All the data in the column will be lost.

*/
-- AlterTable
ALTER TABLE "PlantInstance" DROP COLUMN "ageDays",
ADD COLUMN     "currentGameDate" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
ADD COLUMN     "plantedDate" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP;
