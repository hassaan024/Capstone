// src/App.tsx
import React from "react";
import { Routes, Route } from "react-router-dom";
import LandingPage from "./pages/LoginPage";
import LoginPage from "./pages/LoginPage";

const App: React.FC = () => {
  return (
    <Routes>
      <Route path="/" element={<LandingPage />} />
      <Route path="/login" element={<LoginPage />} />
      {/* Future: <Route path="/dashboard" element={<Dashboard />} /> */}
    </Routes>
  );
};

export default App;
