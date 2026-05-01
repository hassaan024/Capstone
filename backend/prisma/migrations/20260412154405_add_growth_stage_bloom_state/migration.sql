-- CreateEnum
CREATE TYPE "GrowthStage" AS ENUM ('Seedling', 'Vegetative', 'Flowering', 'Fruiting', 'Dormant');

-- AlterTable
ALTER TABLE "PlantInstance" ADD COLUMN     "bloomState" BOOLEAN NOT NULL DEFAULT false,
ADD COLUMN     "growthStage" "GrowthStage";
