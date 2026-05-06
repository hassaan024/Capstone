-- CreateEnum
CREATE TYPE "SoilType" AS ENUM ('LOAM', 'SANDY', 'CLAY', 'SILT', 'PEAT', 'CHALK');

-- CreateTable
CREATE TABLE "User" (
    "id" SERIAL NOT NULL,
    "email" TEXT NOT NULL,
    "displayName" TEXT NOT NULL,
    "passwordHash" TEXT NOT NULL,
    "creationTimestamp" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "lastUpdated" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "User_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "Garden" (
    "id" SERIAL NOT NULL,
    "ownerId" INTEGER NOT NULL,
    "name" TEXT NOT NULL,
    "description" TEXT NOT NULL,
    "latitude" DOUBLE PRECISION NOT NULL,
    "longitude" DOUBLE PRECISION NOT NULL,
    "timezone" TEXT,
    "creationTimestamp" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "lastUpdated" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "Garden_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "PlantInstance" (
    "id" SERIAL NOT NULL,
    "gardenId" INTEGER NOT NULL,
    "speciesId" INTEGER NOT NULL,
    "soilId" INTEGER NOT NULL,
    "creationTimestamp" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "lastUpdated" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "PlantInstance_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "Species" (
    "id" SERIAL NOT NULL,
    "commonName" TEXT NOT NULL,
    "scientificName" TEXT NOT NULL,
    "growthRate" DOUBLE PRECISION NOT NULL,
    "bloomRate" DOUBLE PRECISION,
    "witherRate" DOUBLE PRECISION,
    "minTemp" DOUBLE PRECISION,
    "maxTemp" DOUBLE PRECISION,
    "minHumidity" DOUBLE PRECISION,
    "maxHumidity" DOUBLE PRECISION,
    "avgHoursSun" DOUBLE PRECISION,
    "imgSrcUrl" TEXT,
    "notes" TEXT,

    CONSTRAINT "Species_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "Soil" (
    "id" SERIAL NOT NULL,
    "nitrogen" DOUBLE PRECISION NOT NULL,
    "phosphorus" DOUBLE PRECISION NOT NULL,
    "potassium" DOUBLE PRECISION NOT NULL,
    "pH" DOUBLE PRECISION NOT NULL,
    "organicPercentage" DOUBLE PRECISION,
    "type" "SoilType" NOT NULL,

    CONSTRAINT "Soil_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE UNIQUE INDEX "User_email_key" ON "User"("email");

-- CreateIndex
CREATE UNIQUE INDEX "Species_commonName_key" ON "Species"("commonName");

-- CreateIndex
CREATE UNIQUE INDEX "Species_scientificName_key" ON "Species"("scientificName");

-- AddForeignKey
ALTER TABLE "Garden" ADD CONSTRAINT "Garden_ownerId_fkey" FOREIGN KEY ("ownerId") REFERENCES "User"("id") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "PlantInstance" ADD CONSTRAINT "PlantInstance_gardenId_fkey" FOREIGN KEY ("gardenId") REFERENCES "Garden"("id") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "PlantInstance" ADD CONSTRAINT "PlantInstance_speciesId_fkey" FOREIGN KEY ("speciesId") REFERENCES "Species"("id") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "PlantInstance" ADD CONSTRAINT "PlantInstance_soilId_fkey" FOREIGN KEY ("soilId") REFERENCES "Soil"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
