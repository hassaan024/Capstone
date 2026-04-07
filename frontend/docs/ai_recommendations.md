# AI Recommendations & Toggles

The application provides intelligent, automated AI recommendations to guide users and provide insights about their plants and pages.

## Overview
- **System**: Context-aware suggestions triggered by user actions (navigating pages, viewing plants).
- **Control**: User-facing switches in account settings for comprehensive control.
- **Purpose**: Balance automated helpfulness with user privacy and convenience.

## Types of Recommendations
- **Plant-Specific Recommendations**: Suggested via small text bubbles over the chatbot when a user views a plant (e.g., fun facts, care tips).
- **Page Information Guides**: Helpful prompts appearing automatically upon landing on the Dashboard, Browse, or Saved Species pages.

## User Control (Toggles)
Located in the **Account Settings** page, users can independently toggle these features:
- **Plant Recommendations**: Opt-in/out of automated popups for specific plants.
- **Page Info Recommendations**: Opt-in/out of helpful guide prompts for all pages.

## Technical Execution
- **Triggers**: Dispatch `CustomEvent` calls (`suggestChat`) from React `useEffect` hooks.
- **Preference Persistence**: User choices are stored in the PostgreSQL database (`User` model) and respected globally across all devices.
- **Context Integration**: The application's `AuthContext` provides the real-time user settings to all frontend components.
