-- CreateEnum
CREATE TYPE "HealthStatus" AS ENUM ('Healthy', 'Wilting', 'Sick', 'Dead', 'NeedsWater');

-- AlterTable
ALTER TABLE "Garden" ALTER COLUMN "description" DROP NOT NULL;

-- AlterTable
ALTER TABLE "PlantInstance" ADD COLUMN     "ageDays" INTEGER,
ADD COLUMN     "healthStatus" "HealthStatus",
ADD COLUMN     "heightCm" DOUBLE PRECISION,
ADD COLUMN     "lastWatered" TIMESTAMP(3),
ADD COLUMN     "notes" TEXT;

-- AlterTable
ALTER TABLE "User" ALTER COLUMN "passwordHash" DROP NOT NULL;
