// src/App.tsx
import React from "react";
import { Routes, Route } from "react-router-dom";
import { AuthProvider } from "./context/AuthContext";
import LandingPage from "./pages/LandingPage";
import LoginPage from "./pages/LoginPage";
import { Dashboard } from "./pages/Dashboard";
import BrowseSpecies from "./pages/BrowseSpecies";
import SavedSpecies from "./pages/SavedSpecies";
import MyGardens from "./pages/MyGardens";
import DummyGarden from "./pages/DummyGarden";
import Settings from "./pages/Settings";
import ChatWidget from "./components/ChatWidget";

const App: React.FC = () => {
  return (
    <AuthProvider>
      <Routes>
        <Route path="/" element={<LandingPage />} />
        <Route path="/login" element={<LoginPage />} />
        <Route path="/dashboard" element={<Dashboard />} />
        <Route path="/browse" element={<BrowseSpecies />} />
        <Route path="/saved-species" element={<SavedSpecies />} />
        <Route path="/gardens" element={<MyGardens />} />
        <Route path="/demo-garden" element={<DummyGarden />} />
        <Route path="/settings" element={<Settings />} />
      </Routes>
      <ChatWidget />
    </AuthProvider>
  );
};

export default App;
