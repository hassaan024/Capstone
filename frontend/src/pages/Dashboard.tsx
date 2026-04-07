// src/pages/Dashboard.tsx
import React, { useEffect, useState } from "react";
import { useNavigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";
import { FaCog, FaLeaf, FaSeedling, FaChartBar, FaSignOutAlt, FaSpa, FaSearch, FaBookmark, FaMapMarkerAlt, FaCompass, FaTimes } from 'react-icons/fa';
import { BACKEND_BASE_URL, countries } from "../utils/constants";
import { validatePostalCode } from "../utils/helper_functions";
import { fetchLongLatFromZipAndCountry, api } from "../utils/api";
import { sendLocationToBackend } from "../utils/backend_api";
import { WeatherInfo } from "../components/WeatherInfoCard";

export const Dashboard: React.FC = () => {
  const navigate = useNavigate();
  const { user, logout, updateUser } = useAuth();
  const [zip, setZip] = useState("");
  const [country, setCountry] = useState("US");
  const [error, setError] = useState("");
  const [locationModalOpen, setLocationModalOpen] = useState(false);
  const [locationSaving, setLocationSaving] = useState(false);
  const [locationSuccess, setLocationSuccess] = useState("");
  const [stats, setStats] = useState({ gardens: 0, plantsInGardens: 0, savedSpecies: 0 });

  // Redirect to login if user is not authenticated
  useEffect(() => {
    if (!user) {
      navigate("/login");
    }
  }, [user, navigate]);

  // Suggest Chatbot interaction for Dashboard
  useEffect(() => {
    if (user?.pageInfoRecommendations !== false) {
      const timer = setTimeout(() => {
        const event = new CustomEvent('suggestChat', { detail: 'What can I do on the Dashboard?' });
        window.dispatchEvent(event);
      }, 2000);
      return () => clearTimeout(timer);
    }
  }, [user?.pageInfoRecommendations]);

  // Fetch user location on mount
  useEffect(() => {
    if (!user?.id) return;

    const fetchUserLocation = async () => {
      try {
        const res = await fetch(`${BACKEND_BASE_URL}/user/location/${user.id}`);
        if (!res.ok) {
          throw new Error("Failed to fetch user location");
        }
        const data = await res.json();

        const lat = data.latitude ? Number(data.latitude) : undefined;
        const lng = data.longitude ? Number(data.longitude) : undefined;

        updateUser({ ...user, latitude: lat, longitude: lng });
      } catch (err) {
        console.error("Error fetching user location:", err);
      }
    };

    fetchUserLocation();
  }, [user?.id]);

  useEffect(() => {
    if (!user?.id) return;
    const loadStats = async () => {
      try {
        const [gardensRes, savedRes] = await Promise.all([
          api.get(`/garden/by-user/${user.id}`),
          api.get(`/species/saved?userId=${user.id}`),
        ]);
        const gardens = gardensRes.data as { _count?: { plants: number } }[];
        const plantsInGardens = gardens.reduce(
          (sum, g) => sum + (g._count?.plants ?? 0),
          0,
        );
        const saved = savedRes.data as unknown[];
        setStats({
          gardens: gardens.length,
          plantsInGardens,
          savedSpecies: saved.length,
        });
      } catch (e) {
        console.error("Dashboard stats load failed", e);
      }
    };
    loadStats();
  }, [user?.id]);

  const handleLogout = () => {
    logout();
    navigate("/login");
  };

  // Close modal on Escape key
  useEffect(() => {
    const handleEsc = (e: KeyboardEvent) => {
      if (e.key === 'Escape') setLocationModalOpen(false);
    };
    if (locationModalOpen) {
      window.addEventListener('keydown', handleEsc);
    }
    return () => window.removeEventListener('keydown', handleEsc);
  }, [locationModalOpen]);

  const handleSetLocation = async () => {
    if (!user?.id) {
      console.error("User ID is missing");
      return;
    }

    if (!validatePostalCode(zip, country)) {
      setError("Invalid postal code for selected country.");
      return;
    }
    setError("");
    setLocationSaving(true);

    try {
      const { lat, lng } = await fetchLongLatFromZipAndCountry(zip, country);

      await sendLocationToBackend(
        user.id,
        Number(lat),
        Number(lng),
      );
      updateUser({ ...user, latitude: Number(lat), longitude: Number(lng) });
      setLocationSuccess("Location saved successfully!");
      setTimeout(() => {
        setLocationModalOpen(false);
        setLocationSuccess("");
        setZip("");
      }, 1500);
    } catch (err) {
      console.error("Failed to save location in Dashboard:", err);
      setError("Failed to save location. Please try again.");
    } finally {
      setLocationSaving(false);
    }
  };

  const handleGetLocation = () => {
    if (!user?.id) {
      console.error("User ID is missing");
      return;
    }

    setLocationSaving(true);
    setError("");

    navigator.geolocation.getCurrentPosition(
      async (position) => {
        const { latitude, longitude } = position.coords;
        try {
          await sendLocationToBackend(user.id, latitude, longitude);
          updateUser({ ...user, latitude, longitude });
          setLocationSuccess("Location saved successfully!");
          setTimeout(() => {
            setLocationModalOpen(false);
            setLocationSuccess("");
          }, 1500);
        } catch (err) {
          console.error("Failed to save location in Dashboard:", err);
          setError("Failed to save location. Please try again.");
        } finally {
          setLocationSaving(false);
        }
      },
      (geoError) => {
        console.error("Error getting location:", geoError.message);
        setError("Could not get your location. Please enter a zip code instead.");
        setLocationSaving(false);
      }
    );
  };

  if (!user) {
    return null;
  }

  return (
    <div className="dashboard-root">
      <div className="dashboard-background-gradient" />
      <div className="dashboard-blob dashboard-blob--green" />
      <div className="dashboard-blob dashboard-blob--blue" />
      <div className="dashboard-blob dashboard-blob--purple" />

      {/* Location Modal */}
      {locationModalOpen && (
        <div className="location-modal-overlay" onClick={() => setLocationModalOpen(false)}>
          <div className="location-modal" onClick={(e) => e.stopPropagation()}>
            <button className="location-modal-close" onClick={() => setLocationModalOpen(false)}>
              <FaTimes />
            </button>
            <div className="location-modal-header">
              <div className="location-modal-icon">📍</div>
              <h3>Set Your Location</h3>
              <p>Help us personalize your plant recommendations for your climate</p>
            </div>

            {locationSuccess ? (
              <div className="location-modal-success">
                <div className="location-success-icon">✅</div>
                <p>{locationSuccess}</p>
              </div>
            ) : (
              <div className="location-modal-body">
                <div className="location-modal-section">
                  <label className="location-modal-label">Zip / Postal Code</label>
                  <input
                    type="text"
                    placeholder="e.g. 90210"
                    value={zip}
                    onChange={(e) => { setZip(e.target.value); setError(""); }}
                    className="location-modal-input"
                    disabled={locationSaving}
                  />
                </div>

                <div className="location-modal-section">
                  <label className="location-modal-label">Country</label>
                  <select
                    value={country}
                    onChange={(e) => setCountry(e.target.value)}
                    className="location-modal-select"
                    disabled={locationSaving}
                  >
                    {countries.map((c) => (
                      <option key={c.code} value={c.code}>
                        {c.name}
                      </option>
                    ))}
                  </select>
                </div>

                {error && <div className="location-modal-error">{error}</div>}

                <button
                  onClick={handleSetLocation}
                  className="location-modal-save-btn"
                  disabled={locationSaving || !zip.trim()}
                >
                  {locationSaving ? "Saving..." : "Save Location"}
                </button>

                <div className="location-modal-divider">
                  <span>or</span>
                </div>

                <button
                  onClick={handleGetLocation}
                  className="location-modal-geo-btn"
                  disabled={locationSaving}
                >
                  <FaMapMarkerAlt /> Use My Current Location
                </button>
              </div>
            )}
          </div>
        </div>
      )}

      <div className="dashboard-container">
        <header className="dashboard-header">
          <div className="dashboard-logo">
            <div className="dashboard-logo-mark" />
            <span className="dashboard-logo-text">LeafyLedger</span>
          </div>
          <div className="dashboard-header-actions">
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
          
          <div className="dashboard-card">
            <h2 className="dashboard-card-title">Quick Actions</h2>
            <div className="dashboard-actions">
              <button
                className="ll-btn ll-btn-primary dashboard-action-btn"
                onClick={() => navigate("/gardens")}
              >
                <span className="dashboard-action-icon"><FaSeedling /></span>
                View my gardens
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

          <div className="dashboard-grid">
            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon"><FaLeaf /></div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">{stats.gardens}</div>
                <div className="dashboard-stat-label">Gardens</div>
              </div>
            </div>

            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon"><FaSpa /></div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">{stats.plantsInGardens}</div>
                <div className="dashboard-stat-label">Plants in gardens</div>
              </div>
            </div>

            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon"><FaChartBar /></div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">{stats.savedSpecies}</div>
                <div className="dashboard-stat-label">Saved species</div>
              </div>
            </div>
          </div>

          <WeatherInfo 
            latitude={user.latitude} 
            longitude={user.longitude} 
            onSetLocationClick={() => { setLocationModalOpen(true); setError(""); setLocationSuccess(""); }}
          />
        </main>
      </div>
    </div>
  );
};




