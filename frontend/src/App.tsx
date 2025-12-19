import React from "react";
import { Routes, Route, Navigate } from "react-router-dom";
import LandingPage from "./routes/LandingPage";
import Login from "./routes/Login";

export default function App(): JSX.Element {
  return (
    <Routes>
      <Route path="/" element={<LandingPage />} />
      <Route path="/login" element={<Login />} />

      <Route
        path="/dashboard"
        element={
          <div className="container" style={{ padding: "60px 0" }}>
            <div className="glass" style={{ padding: 28 }}>
              <div className="kicker">LeafyLedger</div>
              <div className="h1" style={{ marginTop: 10 }}>
                Dashboard (Coming Soon)
              </div>
              <p className="p">
                This will become your weather, pricing, and garden analytics hub.
              </p>
            </div>
          </div>
        }
      />

      <Route path="*" element={<Navigate to="/" replace />} />
    </Routes>
  );
}
