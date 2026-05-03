-- CreateEnum
CREATE TYPE "ModelCategory" AS ENUM ('flower', 'tree', 'vegetable');

-- AlterTable
ALTER TABLE "Species" ADD COLUMN "modelCategory" "ModelCategory";
