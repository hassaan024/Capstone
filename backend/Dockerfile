# 1. Base Image
FROM node:22-alpine

# Install build dependencies for native modules like bcrypt
RUN apk add --no-cache python3 make g++

# create a app directory in the container
WORKDIR /app

# copy over the package.json
# note this copyies into the working directory (/app)
COPY package*.json ./ 

# install node_modules according to the package files
# This will compile bcrypt for Alpine Linux
RUN npm ci

# copy over source code
COPY . . 

# build the prisma client
RUN npm run generate

# run on port 4000
EXPOSE 4000

CMD ["npm", "run", "start:dev"]