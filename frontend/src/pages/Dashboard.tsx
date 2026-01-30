// src/pages/Dashboard.tsx
import React, { useEffect, useState } from "react";
import { useNavigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";
import { FaCog, FaLeaf, FaSeedling, FaChartBar, FaPlus, FaSignOutAlt, FaSpa, FaSearch, FaBookmark, FaMapMarkerAlt, FaCompass } from 'react-icons/fa';
import { BACKEND_BASE_URL, countries } from "../utils/constants";
import { validatePostalCode } from "../utils/helper_functions";
import { fetchLongLatFromZipAndCountry } from "../utils/api";
import { sendLocationToBackend } from "../utils/backend_api";

export const Dashboard: React.FC = () => {
  const navigate = useNavigate();
  const { user, logout } = useAuth();
  const [zip, setZip] = useState("");
  const [country, setCountry] = useState("US");
  const [locationMenuOpen, setLocationMenuOpen] = useState(false);
  const [error, setError] = useState("");

  // Redirect to login if user is not authenticated
  useEffect(() => {
    if (!user) {
      navigate("/login");
    }
  }, [user, navigate]);

  const handleLogout = () => {
    logout();
    navigate("/login");
  };

  const handleSetLocation = async () => {
    if (!user?.id) {
      console.error("User ID is missing");
      return;
    }

    if (!validatePostalCode(zip, country)) {
      setError("Invalid postal code for selected country.");
      return;
    }
    console.log({ zip, country });
    setError("")

    const { lat, lng } = await fetchLongLatFromZipAndCountry(zip, country);
    console.log("Latitude:", lat, "Longitude:", lng)

    // send to the backend and store info for later use
    try {
      const data = await sendLocationToBackend(
        user.id,
        Number(lat),
        Number(lng),
      );
    } catch (err) {
      console.error("Failed to save location in Dashboard:", err);
    }
    setLocationMenuOpen(false);
  }

  const handleGetLocation = () => {

    if (!user?.id) {
      console.error("User ID is missing");
      return;
    }

    console.log(user)
    console.log("GETTING THE LOCATION...")

    navigator.geolocation.getCurrentPosition(
      async (position) => {
        const { latitude, longitude } = position.coords;
        console.log("Latitude:", latitude, "Longitude:", longitude);
        try {
          const data = await sendLocationToBackend(
            user.id,
            latitude,
            longitude
          );

          console.log("Data saved successfully:", data);
        } catch (err) {
          console.error("Failed to save location in Dashboard:", err);
        }
      },
      (error) => {
        console.error("Error getting location:", error.message)
      }
    )
  }

  if (!user) {
    return null;
  }

  return (
    <div className="dashboard-root">
      <div className="dashboard-background-gradient" />
      <div className="dashboard-blob dashboard-blob--green" />
      <div className="dashboard-blob dashboard-blob--blue" />
      <div className="dashboard-blob dashboard-blob--purple" />

      <div className="dashboard-container">
        <header className="dashboard-header">
          <div className="dashboard-logo">
            <div className="dashboard-logo-mark" />
            <span className="dashboard-logo-text">LeafyLedger</span>
          </div>
          <div className="dashboard-header-actions">
            <div className="relative inline-block">
              <button
                onClick={() => setLocationMenuOpen(!locationMenuOpen)}
                className="ll-btn ll-btn-ghost dashboard-btn flex items-center gap-2"
              >
                <FaCompass /> Set Location
              </button>
              {/* OPEN THE MENU TO SELECT LOCATION */}
              {locationMenuOpen && (
                <div className="absolute left-1/2 transform -translate-x-1/2 mt-1 w-48 bg-white shadow-md rounded p-2 flex flex-col gap-1 z-10">
                  <input
                    type="text"
                    placeholder="ZIP / Postal code"
                    value={zip}
                    onChange={(e) => setZip(e.target.value)}
                    className="border p-1 rounded text-sm"
                  />
                  <select
                    value={country}
                    onChange={(e) => setCountry(e.target.value)}
                    className="border p-1 rounded text-sm"
                  >
                    {countries.map((c) => (
                      <option key={c.code} value={c.code}>
                        {c.name}
                      </option>
                    ))}
                  </select>
                  <button
                    onClick={handleSetLocation}
                    className="ll-btn ll-btn-primary mt-1 text-sm py-1"
                  >
                    Save
                  </button>
                </div>

              )}
            </div>
            <button 
              onClick={handleGetLocation} 
              className="ll-btn ll-btn-ghost dashboard-btn"
              style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
            >
              <FaMapMarkerAlt /> Give Location
            </button>
            <button 
              onClick={() => navigate("/settings")} 
              className="ll-btn ll-btn-ghost dashboard-btn"
              style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
            >
              <FaCog /> Settings
            </button>
            <button 
              onClick={handleLogout} 
              className="ll-btn ll-btn-ghost dashboard-btn"
              style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}
            >
              <FaSignOutAlt /> Logout
            </button>
          </div>
        </header>

        <main className="dashboard-main">
          <div className="dashboard-welcome">
            <h1 className="dashboard-welcome-title">
              Welcome back, <span>{user.displayName}</span>! <FaSeedling style={{ display: 'inline', verticalAlign: 'middle' }} />
            </h1>
            <p className="dashboard-welcome-subtitle">
              Manage your gardens and track your plants
            </p>
          </div>

          <div className="dashboard-grid">
            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon"><FaLeaf /></div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">0</div>
                <div className="dashboard-stat-label">Gardens</div>
              </div>
            </div>

            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon"><FaSpa /></div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">0</div>
                <div className="dashboard-stat-label">Plants</div>
              </div>
            </div>

            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon"><FaChartBar /></div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">0</div>
                <div className="dashboard-stat-label">Species</div>
              </div>
            </div>
          </div>

          <div className="dashboard-card">
            <h2 className="dashboard-card-title">Quick Actions</h2>
            <div className="dashboard-actions">
              <button className="ll-btn ll-btn-primary dashboard-action-btn">
                <span className="dashboard-action-icon"><FaPlus /></span>
                Create New Garden
              </button>
              <button 
                className="ll-btn ll-btn-ghost dashboard-action-btn"
                onClick={() => navigate('/saved-species')}
              >
                <span className="dashboard-action-icon"><FaBookmark /></span>
                View Saved Plants
              </button>
              <button 
                className="ll-btn ll-btn-ghost dashboard-action-btn"
                onClick={() => navigate('/browse')}
              >
                <span className="dashboard-action-icon"><FaSearch /></span>
                Browse Species
              </button>
            </div>
          </div>
        </main>
      </div>
    </div>
  );
};


