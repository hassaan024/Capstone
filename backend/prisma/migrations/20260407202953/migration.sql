-- AlterTable
ALTER TABLE "User" ADD COLUMN     "pageInfoRecommendations" BOOLEAN NOT NULL DEFAULT true,
ADD COLUMN     "plantRecommendations" BOOLEAN NOT NULL DEFAULT true;
