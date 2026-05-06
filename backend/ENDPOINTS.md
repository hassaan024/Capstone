# Backend API Endpoints

Base URL: `http://localhost:4000/backend`

Run the backend with Docker before testing:
```bash
npm run docker:build      # first time only
npm run docker:migrate    # apply DB migrations
npm run docker:up         # start and tail logs
```

---

## Species

### GET /backend/species/saved
Returns all species saved by a user, including `bloomDays` and `modelCategory`.

**Query params:**
- `userId` (required) — user ID
- `gardenId` (optional) — if provided, returns only species planted in that garden

**Examples:**
```
GET /backend/species/saved?userId=1
GET /backend/species/saved?userId=1&gardenId=2
```

**Response:**
```json
[
  {
    "id": 1,
    "commonName": "Profusion Zinnia",
    "scientificName": "Zinnia elegans",
    "growthRate": 3,
    "bloomDays": 60,
    "modelCategory": "flower",
    ...
  }
]
```

---

### POST /backend/species/save/:trefleId
Saves a species to a user's collection and persists `bloomDays` to the database.
Optionally creates a `PlantInstance` in a specific garden (with LOAM soil defaults).

**Params:**
- `:trefleId` — Perenual/Trefle species ID

**Query params:**
- `userId` (required)
- `gardenId` (optional) — if provided, creates a plant instance in that garden

**Examples:**
```
POST /backend/species/save/1234?userId=1
POST /backend/species/save/1234?userId=1&gardenId=2
```

**Response (user collection save):**
```json
{
  "message": "Species saved successfully",
  "species": { "id": 1, "bloomDays": 60, ... }
}
```

**Response (garden save):**
```json
{
  "message": "Species added to garden",
  "plantInstance": { "id": 5, "gardenId": 2, "speciesId": 1, ... },
  "bloomDays": 60
}
```

---

### DELETE /backend/species/save/:trefleId
Removes a species from a user's collection, or removes all plant instances of that species from a garden.

**Params:**
- `:trefleId` — Perenual/Trefle species ID

**Query params:**
- `userId` (required)
- `gardenId` (optional)

**Examples:**
```
DELETE /backend/species/save/1234?userId=1
DELETE /backend/species/save/1234?userId=1&gardenId=2
```

---

## Prediction

### POST /backend/prediction/demo/seed
Seeds 6 Louisiana demo species into the database with accurate hardcoded data.
Safe to call multiple times. Returns the DB ID for each species.

**No body required.**

**Example:**
```
POST /backend/prediction/demo/seed
```

**Response:**
```json
[
  { "action": "created", "id": 1, "commonName": "Profusion Zinnia", "type": "Flower" },
  { "action": "created", "id": 2, "commonName": "Creole Tomato", "type": "Vegetable" },
  { "action": "created", "id": 3, "commonName": "Crape Myrtle", "type": "Tree" }
]
```

---

### GET /backend/prediction/bloom-estimate/:speciesId
Returns an estimated bloom date for a species given a planting date.
Formula-based, no ML call — fast response.

**Params:**
- `:speciesId` — internal DB species ID

**Query params:**
- `plantedDate` (optional) — ISO 8601 date string, defaults to today

**Example:**
```
GET /backend/prediction/bloom-estimate/1?plantedDate=2026-05-01
```

**Response:**
```json
{
  "speciesId": 1,
  "speciesName": "Profusion Zinnia",
  "plantedDate": "2026-05-01T00:00:00.000Z",
  "estimatedBloomDate": "2026-06-30T00:00:00.000Z",
  "daysToMature": 60,
  "maxHeightCm": 60.96,
  "feasible": true,
  "feasibilityNote": null
}
```

---

### POST /backend/prediction/:plantInstanceId
Runs a single ML prediction for a plant instance N days into the future.

**Params:**
- `:plantInstanceId` — plant instance ID

**Query params:**
- `daysAhead` (required) — integer, number of days into the future

**Example:**
```
POST /backend/prediction/5?daysAhead=30
```

**Response includes:** predicted height, growth stage, stress factors, confidence score, and notes from the ML model.

---

### POST /backend/prediction/garden/:gardenId/timeline
Generates a full stepped growth timeline for every plant in a garden.
Stores results in PlantHistory. Used by the Unreal Engine slider.

**Params:**
- `:gardenId` — garden ID

**Body:**
```json
{ "bloomDate": "2027-06-01" }
```

**Example:**
```
POST /backend/prediction/garden/1/timeline
Body: { "bloomDate": "2027-06-01" }
```

**Response:** Array of timeline entries per plant, each with 7-day snapshots from planting to bloom date.

---

## Suggested Postman Demo Order

1. `POST /backend/prediction/demo/seed` — seed demo species, note the returned IDs
2. `POST /backend/species/save/{speciesId}?userId=1` — save a species, confirm `bloomDays` is returned
3. `GET /backend/species/saved?userId=1` — confirm `bloomDays` and `modelCategory` in response
4. `GET /backend/prediction/bloom-estimate/{speciesId}?plantedDate=2026-05-01` — show fast bloom estimate
5. `POST /backend/prediction/{plantInstanceId}?daysAhead=30` — single ML prediction
6. `POST /backend/prediction/garden/{gardenId}/timeline` with `{ "bloomDate": "2027-06-01" }` — full timeline for Unreal slider
