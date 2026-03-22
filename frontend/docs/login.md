# Login Page

The **Login Page** (`/login`) is the main entry point for users to authenticate into LeafyLedger. It ensures secure access to user-specific data such as gardens, saved plants, and personalized weather information.

## Overview
- **Path**: `/login`
- **Purpose**: Authenticate users and restrict access to the dashboard and other protected routes.
- **State Management**: Uses the `AuthContext` (`useAuth` hook) to manage the user's authentication state globally across the application.

## Features
- **Secure Authentication**: Authenticates users before granting access to the application.
- **Redirection**: 
  - Unauthenticated users trying to access protected routes (e.g., Dashboard, Settings) are automatically redirected to the Login page.
  - Upon successful login, users are redirected to the Dashboard.
- **Session Persistence**: Maintains user session using the context provider so users don't have to log in repeatedly.

## Usage
Once logged in, the `user` object is populated in the `AuthContext`, providing access to user details like `id`, `displayName`, and coordinates (`latitude`, `longitude`) needed across the app.
