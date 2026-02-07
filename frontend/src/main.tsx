// src/index.tsx
import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import { BrowserRouter } from 'react-router-dom'
import { GoogleOAuthProvider } from '@react-oauth/google'
import { GOOGLE_OAUTH_CLIENT_ID } from './utils/constants'
import 'bootstrap/dist/css/bootstrap.min.css'
import "./styles/global.css";
import "./styles/LandingPage.css";
import "./styles/Login.css";
import "./styles/Dashboard.css";
import "./styles/Settings.css";
import App from './App'

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <GoogleOAuthProvider clientId={GOOGLE_OAUTH_CLIENT_ID}>
      <BrowserRouter>
        <App />
      </BrowserRouter>
    </GoogleOAuthProvider>
  </StrictMode>
);

