/*
  Warnings:

  - You are about to drop the column `wateringBenchmark` on the `Species` table. All the data in the column will be lost.

*/
-- AlterTable
ALTER TABLE "Species" DROP COLUMN "wateringBenchmark",
ADD COLUMN     "wateringMaxDays" INTEGER,
ADD COLUMN     "wateringMinDays" INTEGER;
