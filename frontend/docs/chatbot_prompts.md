# Chatbot Prompts & Quick Replies

The **Chatbot Prompts & Quick Replies** system enhances user interaction by providing ready-to-ask questions based on the context of the current page or specific plant being viewed.

## Overview
- **Core Component**: `ChatWidget.tsx`
- **Purpose**: Assist users in formulating questions for the AI and provide immediate access to relevant information.

## Types of Prompts
- **Context-Aware Suggestions**: Small text bubbles popping up over the closed chatbot widget to pique user interest.
- **Quick-Reply Buttons**: Interactive buttons within the chat window when a specific plant context is active.
- **"Tell me more" Guides**: Page-level introductory prompts suggesting what users can do in different sections.

## Interactive Prompts
- **Dashboard Prompt**: *"What can I do on the Dashboard?"*
- **Browse Prompt**: *"What can I do on the Browse Species page?"*
- **Saved Species Prompt**: *"What can I do on the Saved Species page?"*
- **Plant Details Prompt**: *"Tell me some fun facts about [Plant Name]!"*

## Quick-Reply Suggestions (In-Chat)
When a user asks about a plant, the chatbot automatically generates clickable follow-up questions:
- *Can this plant grow well in my local weather?*
- *Is this plant invasive?*
- *Does this plant have any medicinal uses?*

## User Accessibility
AI prompts and suggestions can be disabled in the **Account Settings** page for a more streamlined, manual-only experience.
