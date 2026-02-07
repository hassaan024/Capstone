// src/pages/Dashboard.tsx
import React, { useEffect } from "react";
import { useNavigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";
import { FaCog, FaLeaf, FaSeedling, FaChartBar, FaPlus, FaSignOutAlt, FaSpa, FaSearch, FaBookmark } from 'react-icons/fa';

const Dashboard: React.FC = () => {
  const navigate = useNavigate();
  const { user, logout } = useAuth();

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

export default Dashboard;
