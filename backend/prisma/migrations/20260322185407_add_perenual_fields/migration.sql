/*
  Warnings:

  - You are about to drop the column `bloomRate` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `hardinessMapUrl` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `imgSrcUrl` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `maxHumidity` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `minHumidity` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `poisonousToHumans` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `poisonousToPets` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `propagation` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `sunlight` on the `Species` table. All the data in the column will be lost.
  - You are about to drop the column `witherRate` on the `Species` table. All the data in the column will be lost.

*/
-- AlterTable
ALTER TABLE "Species" DROP COLUMN "bloomRate",
DROP COLUMN "hardinessMapUrl",
DROP COLUMN "imgSrcUrl",
DROP COLUMN "maxHumidity",
DROP COLUMN "minHumidity",
DROP COLUMN "poisonousToHumans",
DROP COLUMN "poisonousToPets",
DROP COLUMN "propagation",
DROP COLUMN "sunlight",
DROP COLUMN "witherRate";
