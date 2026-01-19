# Notes on how to run
1.) there are 2 componentents you have to get working 
  1- Prisma Client
  2- Api 
2.) Order of things to do
  1- setup docker (daemon and cli)
  2- run docker build in the package.json (npm run docker:build)
  3- migrate a version of the prisma schema to the database container (npm run docker:migrate)
  4- look at docker logs running on the backend container (npm run docker:logs)
