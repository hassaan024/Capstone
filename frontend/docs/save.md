# Saved Species

The **Saved Species** page (`/saved-species`) acts as a personalized library where users can keep track of plants they are interested in or currently caring for.

## Overview
- **Path**: `/saved-species`
- **Purpose**: Display all the plant species the user has bookmarked or saved from the Browse page.

## Features
- **Personalized List**: Shows only the plants the authenticated user has explicitly saved.
- **Quick Access**: Provides a fast way to reference care instructions and details for favorite plants without having to search for them again.
- **Management**: Users can remove species from their saved list if they are no longer interested.

## Integration
- Links heavily with the **Browse** feature. Clicking a bookmark icon on a species card in `/browse` will add it to the backend database under the user's profile, which then populates this page.
