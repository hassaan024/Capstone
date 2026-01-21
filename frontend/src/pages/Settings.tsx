// src/pages/Settings.tsx
import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { useAuth } from "../context/AuthContext";
import { API_BASE_URL } from "../utils/api";

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
      const res = await fetch(`${API_BASE_URL}/user/${user.id}`, {
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
      const res = await fetch(`${API_BASE_URL}/auth/change-password`, {
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
      const res = await fetch(`${API_BASE_URL}/user/${user.id}`, {
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
          <button 
            onClick={() => navigate("/dashboard")} 
            className="settings-back-btn"
          >
            ← Back to Dashboard
          </button>
        </header>

        <div className="settings-content">
          <div className="settings-title-section">
            <h1 className="settings-title">Account Settings</h1>
            <p className="settings-subtitle">Manage your account preferences</p>
          </div>

          {/* Account Info - Read Only */}
          <div className="settings-card">
            <h2 className="settings-card-title">Account Information</h2>
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
                value={new Date(user.creationTimestamp).toLocaleDateString()} 
                disabled 
              />
            </div>
          </div>

          {/* Update Name */}
          <form className="settings-card" onSubmit={handleUpdateName}>
            <h2 className="settings-card-title">Update Name</h2>
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
              {isLoading ? 'Updating...' : 'Update Name'}
            </button>
          </form>

          {/* Change Password */}
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
            
            {/* Password section message */}
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
