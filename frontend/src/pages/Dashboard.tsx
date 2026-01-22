// src/pages/Dashboard.tsx
import React from "react";
import { useNavigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";

const Dashboard: React.FC = () => {
  const navigate = useNavigate();
  const { user, logout } = useAuth();

  const handleLogout = () => {
    logout();
    navigate("/login");
  };

  if (!user) {
    navigate("/login");
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
            >
              ⚙️ Settings
            </button>
            <button 
              onClick={handleLogout} 
              className="ll-btn ll-btn-ghost dashboard-btn"
            >
              Logout
            </button>
          </div>
        </header>

        <main className="dashboard-main">
          <div className="dashboard-welcome">
            <h1 className="dashboard-welcome-title">
              Welcome back, <span>{user.displayName}</span>! 🌱
            </h1>
            <p className="dashboard-welcome-subtitle">
              Manage your gardens and track your plants
            </p>
          </div>

          <div className="dashboard-grid">
            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon">🌿</div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">0</div>
                <div className="dashboard-stat-label">Gardens</div>
              </div>
            </div>

            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon">🌺</div>
              <div className="dashboard-stat-content">
                <div className="dashboard-stat-value">0</div>
                <div className="dashboard-stat-label">Plants</div>
              </div>
            </div>

            <div className="dashboard-stat-card">
              <div className="dashboard-stat-icon">📊</div>
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
                <span className="dashboard-action-icon">🌿</span>
                Create New Garden
              </button>
              <button className="ll-btn ll-btn-ghost dashboard-action-btn">
                <span className="dashboard-action-icon">📊</span>
                View Analytics
              </button>
              <button 
                className="ll-btn ll-btn-ghost dashboard-action-btn"
                onClick={() => navigate('/browse')}
              >
                <span className="dashboard-action-icon">🌱</span>
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
