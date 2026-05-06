-- CreateTable
CREATE TABLE "PlantHistory" (
    "id" SERIAL NOT NULL,
    "plantId" INTEGER NOT NULL,
    "date" TIMESTAMP(3) NOT NULL,
    "heightCm" DOUBLE PRECISION,
    "healthStatus" "HealthStatus",
    "predictedValue" DOUBLE PRECISION,
    "notes" TEXT,

    CONSTRAINT "PlantHistory_pkey" PRIMARY KEY ("id")
);

-- CreateIndex
CREATE INDEX "PlantHistory_plantId_date_idx" ON "PlantHistory"("plantId", "date");

-- AddForeignKey
ALTER TABLE "PlantHistory" ADD CONSTRAINT "PlantHistory_plantId_fkey" FOREIGN KEY ("plantId") REFERENCES "PlantInstance"("id") ON DELETE RESTRICT ON UPDATE CASCADE;
