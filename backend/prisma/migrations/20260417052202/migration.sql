-- DropForeignKey
ALTER TABLE "Garden" DROP CONSTRAINT "Garden_ownerId_fkey";

-- DropForeignKey
ALTER TABLE "PlantHistory" DROP CONSTRAINT "PlantHistory_plantId_fkey";

-- DropForeignKey
ALTER TABLE "PlantInstance" DROP CONSTRAINT "PlantInstance_gardenId_fkey";

-- AddForeignKey
ALTER TABLE "Garden" ADD CONSTRAINT "Garden_ownerId_fkey" FOREIGN KEY ("ownerId") REFERENCES "User"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "PlantInstance" ADD CONSTRAINT "PlantInstance_gardenId_fkey" FOREIGN KEY ("gardenId") REFERENCES "Garden"("id") ON DELETE CASCADE ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "PlantHistory" ADD CONSTRAINT "PlantHistory_plantId_fkey" FOREIGN KEY ("plantId") REFERENCES "PlantInstance"("id") ON DELETE CASCADE ON UPDATE CASCADE;
