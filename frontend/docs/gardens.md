# Garden Management

The **Garden Management** system allows users to create, view, and organize their virtual gardens, tracking plants and their growth progress.

## Overview
- **Core Feature**: Users can create multiple gardens (e.g., "Front Yard", "Kitchen Balcony").
- **Components**: `MyGardens`, `GardenDetails`.
- **Purpose**: Centralized organization of saved plants into specific physical or logical locations.

## Features
- **Garden Creation**: Users can define a name, description, and geographical location (latitude/longitude) for each garden.
- **Location-Based Data**: Gardens use their specific coordinates to fetch hyper-local weather and climate data.
- **Plant Association**: Users can add specific species from their "Saved Species" collection into their individual gardens.
- **Visual Overview**: A grid or list view showing all owned gardens with summary stats (number of plants, etc.).

## Technical Details
- **Backend Model**: `Garden` (linked to `User` id).
- **Relational Data**: Plants within a garden are managed through the `GardenPlant` junction table, linking `Garden` and `Species`.
- **Location Sync**: Garden coordinates are used to query the Open-Meteo API via the `WeatherService` for accurate growth simulations.
