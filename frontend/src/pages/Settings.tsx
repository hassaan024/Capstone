// src/pages/Settings.tsx
import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";
import { BACKEND_BASE_URL } from "../utils/constants";
import { FaCog } from 'react-icons/fa';

const Settings: React.FC = () => {
  const navigate = useNavigate();
  const { user, logout, updateUser } = useAuth();

  // Form states
  const [displayName, setDisplayName] = React.useState(user?.displayName || "");
  const [oldPassword, setOldPassword] = useState("");
  const [newPassword, setNewPassword] = useState("");
  const [confirmPassword, setConfirmPassword] = useState("");
  
  // Separate messages for each section
  const [nameMessage, setNameMessage] = useState<{ type: 'success' | 'error', text: string } | null>(null);
  const [passwordMessage, setPasswordMessage] = useState<{ type: 'success' | 'error', text: string } | null>(null);
  const [deleteMessage, setDeleteMessage] = useState<{ type: 'success' | 'error', text: string } | null>(null);
  
  const [isLoading, setIsLoading] = useState(false);
  const [showDeleteConfirm, setShowDeleteConfirm] = useState(false);

  if (!user) {
    navigate("/login");
    return null;
  }

  const handleUpdateName = async (e: React.FormEvent) => {
    e.preventDefault();
    setNameMessage(null);
    setIsLoading(true);

    try {
      const res = await fetch(`${BACKEND_BASE_URL}/user/${user.id}`, {
        method: "PATCH",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ displayName }),
      });
      
      const data = await res.json();
      
      if (res.ok) {
        updateUser({ ...user, displayName });
        setNameMessage({ type: 'success', text: 'Name updated successfully!' });
      } else {
        setNameMessage({ type: 'error', text: data.message || 'Failed to update name' });
      }
    } catch (err) {
      setNameMessage({ type: 'error', text: 'Could not connect to server' });
    } finally {
      setIsLoading(false);
    }
  };

  const handleToggleSetting = async (setting: 'plantRecommendations' | 'pageInfoRecommendations') => {
    if (!user) return;
    const currentValue = user[setting] !== false; // Defaults to true if undefined
    setIsLoading(true);
    try {
      const payload = { [setting]: !currentValue };
      const res = await fetch(`${BACKEND_BASE_URL}/user/${user.id}`, {
        method: "PATCH",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });
      
      const data = await res.json();
      if (res.ok) {
        updateUser(data);
      } else {
        setNameMessage({ type: 'error', text: 'Failed to update settings' }); // Reuse nameMessage for global toast
      }
    } catch (err) {
      setNameMessage({ type: 'error', text: 'Could not connect to server' });
    } finally {
      setIsLoading(false);
    }
  };

  const handleChangePassword = async (e: React.FormEvent) => {
    e.preventDefault();
    setPasswordMessage(null);

    if (newPassword !== confirmPassword) {
      setPasswordMessage({ type: 'error', text: 'New passwords do not match' });
      return;
    }

    if (newPassword.length < 6) {
      setPasswordMessage({ type: 'error', text: 'New password must be at least 6 characters' });
      return;
    }

    setIsLoading(true);

    try {
      const res = await fetch(`${BACKEND_BASE_URL}/auth/change-password`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ 
          userId: user.id,
          oldPassword, 
          newPassword 
        }),
      });
      
      const data = await res.json();
      
      if (res.ok) {
        setPasswordMessage({ type: 'success', text: 'Password changed successfully!' });
        setOldPassword("");
        setNewPassword("");
        setConfirmPassword("");
      } else {
        setPasswordMessage({ type: 'error', text: data.message || 'Failed to change password' });
      }
    } catch (err) {
      setPasswordMessage({ type: 'error', text: 'Could not connect to server' });
    } finally {
      setIsLoading(false);
    }
  };

  const handleDeleteAccount = async () => {
    setDeleteMessage(null);
    setIsLoading(true);

    try {
      const res = await fetch(`${BACKEND_BASE_URL}/user/${user.id}`, {
        method: "DELETE",
      });
      
      if (res.ok) {
        logout();
        navigate("/login");
      } else {
        const data = await res.json();
        setDeleteMessage({ type: 'error', text: data.message || 'Failed to delete account' });
      }
    } catch (err) {
      setDeleteMessage({ type: 'error', text: 'Could not connect to server' });
    } finally {
      setIsLoading(false);
      setShowDeleteConfirm(false);
    }
  };

  return (
    <div className="settings-root">
      <div className="settings-background-gradient" />
      <div className="settings-blob settings-blob--green" />
      <div className="settings-blob settings-blob--blue" />

      <div className="settings-container">
        <header className="settings-header">
          <div className="settings-header-title">
            <span style={{ fontSize: '1.2rem', display: 'flex', alignItems: 'center', marginRight: '0.5rem' }}><FaCog /></span>
            Account Settings
          </div>
          <button 
            onClick={() => navigate("/dashboard")} 
            className="settings-back-btn"
          >
            Back to Dashboard
          </button>
        </header>

        <div className="settings-content">

          {/* Account Info */}
          <form className="settings-card" onSubmit={handleUpdateName}>
            <h2 className="settings-card-title">Account Information</h2>
            
            <div className="settings-field">
              <label className="settings-label">Display Name</label>
              <input
                type="text"
                className="settings-input"
                value={displayName}
                onChange={(e) => setDisplayName(e.target.value)}
                required
              />
            </div>

            <div className="settings-field">
              <label className="settings-label">Email (cannot be changed)</label>
              <input 
                type="email" 
                className="settings-input" 
                value={user.email} 
                disabled 
              />
            </div>
            
            <div className="settings-field">
              <label className="settings-label">Account Created</label>
              <input 
                type="text" 
                className="settings-input" 
                value={user.creationTimestamp ? new Date(user.creationTimestamp).toLocaleDateString() : 'Unknown'} 
                disabled 
              />
            </div>

            {/* Name section message */}
            {nameMessage && (
              <div className={`settings-message settings-message--${nameMessage.type}`}>
                {nameMessage.text}
              </div>
            )}
            
            <button 
              type="submit" 
              className="ll-btn ll-btn-primary settings-btn"
              disabled={isLoading || displayName === user.displayName}
            >
              {isLoading ? 'Updating...' : 'Update Settings'}
            </button>
          </form>

          {/* Chatbot Preferences */}
          <div className="settings-card">
            <h2 className="settings-card-title">Chatbot Preferences</h2>
            
            <div className="settings-field" style={{ marginBottom: '1.5rem' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <span className="settings-label" style={{ marginBottom: 0 }}>Plant Recommendations</span>
                <label className="settings-toggle">
                  <input 
                    type="checkbox" 
                    checked={user.plantRecommendations !== false}
                    onChange={() => handleToggleSetting('plantRecommendations')}
                    disabled={isLoading}
                  />
                  <span className="settings-toggle-slider"></span>
                </label>
              </div>
              <div style={{ fontSize: '0.85rem', color: 'rgba(148, 163, 184, 0.7)', marginTop: '0.5rem' }}>
                Turn on to receive AI insight popups whenever you click to view a specific plant.
              </div>
            </div>

            <div className="settings-field">
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <span className="settings-label" style={{ marginBottom: 0 }}>Page Info Recommendations</span>
                <label className="settings-toggle">
                  <input 
                    type="checkbox" 
                    checked={user.pageInfoRecommendations !== false}
                    onChange={() => handleToggleSetting('pageInfoRecommendations')}
                    disabled={isLoading}
                  />
                  <span className="settings-toggle-slider"></span>
                </label>
              </div>
              <div style={{ fontSize: '0.85rem', color: 'rgba(148, 163, 184, 0.7)', marginTop: '0.5rem' }}>
                Turn on to receive a helpful guide popup the first time you visit a new section of the app.
              </div>
            </div>
          </div>



          {/* Change Password */}
          {/* Change Password Commented Out
          <form className="settings-card" onSubmit={handleChangePassword}>
            <h2 className="settings-card-title">Change Password</h2>
            <div className="settings-field">
              <label className="settings-label">Current Password</label>
              <input
                type="password"
                className="settings-input"
                placeholder="Enter current password"
                value={oldPassword}
                onChange={(e) => setOldPassword(e.target.value)}
                required
              />
            </div>
            <div className="settings-field">
              <label className="settings-label">New Password</label>
              <input
                type="password"
                className="settings-input"
                placeholder="Enter new password"
                value={newPassword}
                onChange={(e) => setNewPassword(e.target.value)}
                required
              />
            </div>
            <div className="settings-field">
              <label className="settings-label">Confirm New Password</label>
              <input
                type="password"
                className="settings-input"
                placeholder="Confirm new password"
                value={confirmPassword}
                onChange={(e) => setConfirmPassword(e.target.value)}
                required
              />
            </div>
            
            {passwordMessage && (
              <div className={`settings-message settings-message--${passwordMessage.type}`}>
                {passwordMessage.text}
              </div>
            )}
            
            <button 
              type="submit" 
              className="ll-btn ll-btn-primary settings-btn"
              disabled={isLoading}
            >
              {isLoading ? 'Changing...' : 'Change Password'}
            </button>
          </form>
          */}

          {/* Delete Account */}
          <div className="settings-card settings-card--danger">
            <h2 className="settings-card-title">Danger Zone</h2>
            <p className="settings-danger-text">
              Once you delete your account, there is no going back. This action cannot be undone.
            </p>
            
            {/* Delete section message */}
            {deleteMessage && (
              <div className={`settings-message settings-message--${deleteMessage.type}`}>
                {deleteMessage.text}
              </div>
            )}
            
            {!showDeleteConfirm ? (
              <button 
                onClick={() => setShowDeleteConfirm(true)}
                className="ll-btn settings-btn-danger"
              >
                Delete Account
              </button>
            ) : (
              <div className="settings-delete-confirm">
                <p className="settings-confirm-text">Are you absolutely sure?</p>
                <div className="settings-confirm-actions">
                  <button 
                    onClick={handleDeleteAccount}
                    className="ll-btn settings-btn-danger"
                    disabled={isLoading}
                  >
                    {isLoading ? 'Deleting...' : 'Yes, Delete My Account'}
                  </button>
                  <button 
                    onClick={() => setShowDeleteConfirm(false)}
                    className="ll-btn ll-btn-ghost"
                  >
                    Cancel
                  </button>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};

export default Settings;
