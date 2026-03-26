# AI Chatbot

The **AI Chatbot** is a floating widget accessible throughout the application that provides contextual plant care advice and answers user questions using the Gemini API.

## Overview
- **Component**: `ChatWidget`
- **Location**: Rendered at the root level (`App.tsx`), making it globally accessible across all pages.
- **Purpose**: Serve as an intelligent assistant to help users with plant care, garden management, and app navigation.

## Features
- **Floating UI**: Sits in the corner of the screen and can be expanded or minimized dynamically.
- **Expand/Minimize Toggle**: Users can expand the chat panel to cover a larger portion of the screen (approx. half width) for a comfortable reading experience, or minimize it to stay out of the way.
- **AI Integration (Gemini API)**: Connects to a backend service powered by the Gemini API to stream intelligent, context-aware responses.
- **Markdown Support**: Renders complex responses, lists, and formatting properly using Markdown.
- **Smart Suggestions**: Provides quick-reply suggestions to guide users on what they can ask the bot.
- **Error Handling**: Graceful fallback and error messages (e.g., handling free-tier API limits).

## Technical Details
- The widget manages its own open/closed/expanded state to ensure uninterrupted conversation while users navigate between different pages (e.g., from Dashboard to Browse).
- Chat history is maintained locally during the session to provide conversational context to the AI.
