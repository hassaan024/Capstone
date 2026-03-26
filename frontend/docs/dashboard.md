# Dashboard

The **Dashboard** (`/dashboard`) serves as the central hub for the LeafyLedger application. It provides an overview of the user's gardens, quick actions to navigate the app, and personalized weather data.

## Overview
- **Path**: `/dashboard`
- **Purpose**: Provide a personalized home screen with quick access to key features and localized weather information.
- **Authentication**: Protected route. Requires the user to be logged in.

## Layout & Features
### 1. Welcome Section
- Displays a personalized welcome message using the user's `displayName`.

### 2. Quick Actions
Allows users to quickly navigate to important parts of the app:
- **Create New Garden**
- **View Saved Plants**: Navigates to `/saved-species`
- **Browse Species**: Navigates to `/browse`

### 3. Statistics Cards
Displays quick summary metrics for the user:
- Total **Gardens**
- Total **Plants**
- Total **Species**

### 4. Weather Widget (Weather API Included)
The Dashboard integrates a comprehensive `WeatherInfo` component that fetches real-time weather data based on the user's location.
- **Location Support**: 
  - Prompts users to set their location if not available.
  - Users can provide a **Zip/Postal Code & Country** or use the browser's **Geolocation API** to get exact coordinates.
  - Automatically fetches backend location data on mount.
- **Weather API Integration**: 
  - Retrieves weather metrics.
  - Features an expandable detailed section with a 24-hour forecast graph and extended metrics.
  - Location is saved to the backend to persist between sessions.

## Technical Details
- **Location Modal**: A modal that captures `zip` and `country` and converts it to latitude and longitude using a custom utility (`fetchLongLatFromZipAndCountry`).
- **Dependencies**: Uses `react-icons` for UI icons, `useAuth` for user context, and custom API calls to sync location with the backend.
