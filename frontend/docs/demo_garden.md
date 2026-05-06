# Demo Garden Simulation

The **Demo Garden** (formally **DummyGarden**) is a visual tool that provides users with a simulated environment to showcase the potential of their plant collections.

## Overview
- **Component**: `DummyGarden`
- **Location**: Accessible via the "Explore Demo Garden" link on the Dashboard.
- **Purpose**: Showcase the 3D-like growth simulation and layout possibilities for plants before users create their own physical gardens.

## Features
- **3D Visualization Simulation**: Provides a grid-based representation of plants.
- **Mock Growth Progress**: Demonstrates what plants look like at different growth stages (Seeded, Leafy, Bloom, Peak).
- **Interactive Layout**: Allows users to see how different species (Trees, Flowers, Vegetables) look when placed together.
- **Model Categories**: Uses the dynamic `modelCategory` system (`tree`, `flower`, `vegetable`) to assign appropriate visual models.

## Technical Details
- **Mock Data**: Uses a predefined set of plants to ensure a full, vibrant garden experience for first-time users.
- **Styling**: Leverages premium CSS animations and gradients to create a visually striking interface.
- **Routing**: `/demo-garden` endpoint on the frontend.
