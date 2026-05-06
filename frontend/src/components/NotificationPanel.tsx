import React, { useState, useEffect } from 'react';
import { createPortal } from 'react-dom';
import { useNavigate } from 'react-router-dom';
import { FaBell, FaTimes, FaCalendarAlt, FaLeaf, FaChevronDown, FaChevronUp, FaSeedling, FaEnvelope } from 'react-icons/fa';
import '../styles/NotificationPanel.css';
import { api } from '../utils/api';
import { useAuth } from '../context/AuthContext';
import { useToast } from '../context/ToastContext';
import GardenPlantCard from './GardenPlantCard';

interface AlertData {
  plantInstanceId: number;
  gardenId: number;
  gardenName: string;
  species: any;
  plantedDate: string;
  notificationDate: string;
  count?: number;
  plantInstanceIds?: number[];
}

const NotificationPanel: React.FC = () => {
  const { user } = useAuth();
  const { toast } = useToast();
  const navigate = useNavigate();
  const [isOpen, setIsOpen] = useState(false);
  const [alerts, setAlerts] = useState<AlertData[]>([]);
  const [loading, setLoading] = useState(false);
  const [emailSending, setEmailSending] = useState(false);
  const [expandedAlertId, setExpandedAlertId] = useState<number | null>(null);

  useEffect(() => {
    if (user?.id) {
      fetchAlerts();
    }
  }, [user]);

  const fetchAlerts = async () => {
    setLoading(true);
    try {
      const res = await api.get(`/garden/alerts/${user?.id}`);
      const rawAlerts: AlertData[] = res.data;
      
      const consolidated: Record<string, AlertData> = {};
      
      rawAlerts.forEach(alert => {
        const speciesId = alert.species?.id || alert.species?.perenualId || 'unknown';
        const key = `${alert.gardenId}-${speciesId}-${alert.plantedDate}`;
        
        if (consolidated[key]) {
          consolidated[key].count = (consolidated[key].count || 1) + 1;
          if (!consolidated[key].plantInstanceIds) {
            consolidated[key].plantInstanceIds = [consolidated[key].plantInstanceId];
          }
          consolidated[key].plantInstanceIds!.push(alert.plantInstanceId);
        } else {
          consolidated[key] = {
            ...alert,
            count: 1,
            plantInstanceIds: [alert.plantInstanceId]
          };
        }
      });
      
      setAlerts(Object.values(consolidated));
    } catch (err) {
      console.error('Failed to fetch alerts:', err);
    } finally {
      setLoading(false);
    }
  };

  const togglePanel = () => {
    if (!isOpen) {
      fetchAlerts();
    }
    setIsOpen(!isOpen);
  };

  const handleSendEmail = async () => {
    if (!user?.id || emailSending) return;
    setEmailSending(true);
    try {
      const res = await api.post(`/garden/send-alert-email/${user.id}`, {});
      if (res.data.sent) {
        toast(`Alert email sent! Check your inbox.`, 'success');
      } else {
        toast('No active alerts to email right now.', 'info');
      }
    } catch {
      toast('Failed to send email. Check your app password in .env.', 'error');
    } finally {
      setEmailSending(false);
    }
  };

  const formatRelativeDays = (dateStr: string) => {
    const today = new Date();
    today.setHours(0, 0, 0, 0);
    const target = new Date(dateStr);
    target.setHours(0, 0, 0, 0);
    
    const diffTime = target.getTime() - today.getTime();
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));
    
    if (diffDays === 0) return 'Today';
    if (diffDays === 1) return 'Tomorrow';
    if (diffDays > 1) return `In ${diffDays} days`;
    if (diffDays === -1) return 'Yesterday';
    return `${Math.abs(diffDays)} days ago`;
  };

  const formatDate = (dateStr: string) => {
    return new Date(dateStr).toLocaleDateString(undefined, {
      month: 'short',
      day: 'numeric',
      year: 'numeric'
    });
  };

  return (
    <>
      <button 
        className="ll-btn ll-btn-ghost dashboard-btn notification-bell-btn"
        onClick={togglePanel}
        aria-label="Notifications"
      >
        <FaBell /> 
        <span style={{ marginLeft: '0.5rem', display: 'none' }}>Alerts</span>
        {alerts.length > 0 && (
          <span className="notification-badge">{alerts.length}</span>
        )}
      </button>

      {createPortal(
        <>
          <div className={`notification-panel-overlay ${isOpen ? 'open' : ''}`} onClick={() => setIsOpen(false)} />

          <div className={`notification-panel-drawer ${isOpen ? 'open' : ''}`}>
            <div className="notification-panel-header">
              <h2><FaBell style={{ color: '#ef4444' }} /> Planting Alerts</h2>
              <button className="notification-panel-close" onClick={() => setIsOpen(false)}>
                <FaTimes />
              </button>
            </div>

            <div className="notification-panel-content">
              {loading ? (
                <div className="notification-empty">
                  <div className="spinner" style={{ margin: '0 auto 1rem', width: '30px', height: '30px', border: '3px solid rgba(255,255,255,0.1)', borderTopColor: '#38bdf8', borderRadius: '50%', animation: 'spin 1s linear infinite' }}></div>
                  <p>Checking for alerts...</p>
                </div>
              ) : alerts.length === 0 ? (
                <div className="notification-empty">
                  <FaLeaf className="notification-empty-icon" />
                  <p>No upcoming planting tasks.</p>
                  <p style={{ fontSize: '0.85rem', opacity: 0.7, marginTop: '0.5rem' }}>
                    We'll notify you 7 days before any plant needs to be planted in your gardens.
                  </p>
                </div>
              ) : (
                alerts.map((alert) => (
                  <div key={alert.plantInstanceId} className="notification-card">
                    <div className="notification-card-header">
                      <div>
                        <h3 className="notification-card-title">
                          {alert.species.commonName || 'Unknown Plant'}
                          {alert.count && alert.count > 1 ? ` (x${alert.count})` : ''}
                        </h3>
                        <p className="notification-card-subtitle">{alert.species.scientificName}</p>
                      </div>
                      <div className="notification-card-date-badge">
                        <FaCalendarAlt />
                        {formatRelativeDays(alert.plantedDate)}
                      </div>
                    </div>

                    <div className="notification-card-body">
                      {alert.count && alert.count > 1 && (
                        <p style={{ marginBottom: '0.45rem', color: '#fbbf24', fontWeight: '600', fontSize: '0.9rem' }}>
                          You have {alert.count} plants of this kind to plant.
                        </p>
                      )}
                      <p>
                        <strong style={{ color: '#e2e8f0' }}>Target Date:</strong> {formatDate(alert.plantedDate)}
                      </p>
                      <p style={{ marginTop: '0.35rem' }}>
                        <strong style={{ color: '#e2e8f0' }}>Garden:</strong> {alert.gardenName}
                      </p>
                      <p style={{ marginTop: '0.5rem', fontSize: '0.75rem', color: '#64748b' }}>
                        Sent: {formatDate(alert.notificationDate)}
                      </p>
                    </div>

                    <div className="notification-card-footer">
                      <button 
                        className="notification-action-btn details"
                        onClick={() => setExpandedAlertId(expandedAlertId === alert.plantInstanceId ? null : alert.plantInstanceId)}
                      >
                        {expandedAlertId === alert.plantInstanceId ? <FaChevronUp /> : <FaChevronDown />}
                        {expandedAlertId === alert.plantInstanceId ? 'Hide Details' : 'View Details'}
                      </button>
                      <button 
                        className="notification-action-btn garden"
                        onClick={() => {
                          setIsOpen(false);
                          // Navigate to gardens page (user can then select the garden)
                          navigate('/gardens');
                        }}
                      >
                        <FaSeedling />
                        Go to Garden
                      </button>
                    </div>

                    {expandedAlertId === alert.plantInstanceId && (
                      <div className="notification-expanded-area">
                        <GardenPlantCard 
                          plant={{
                            id: alert.plantInstanceId,
                            species: alert.species,
                            plantedDate: alert.plantedDate,
                            soil: { type: 'Unknown' } // generic fallback
                          } as any}
                        />
                      </div>
                    )}
                  </div>
                ))
              )}
            </div>

            {/* Footer: on-demand email button */}
            {alerts.length > 0 && (
              <div style={{
                padding: '1rem 1.25rem',
                borderTop: '1px solid rgba(148,163,184,0.1)',
                display: 'flex',
                justifyContent: 'center',
              }}>
                <button
                  onClick={handleSendEmail}
                  disabled={emailSending}
                  style={{
                    display: 'flex',
                    alignItems: 'center',
                    gap: '0.5rem',
                    padding: '0.6rem 1.25rem',
                    background: emailSending ? 'rgba(74,222,128,0.08)' : 'rgba(74,222,128,0.12)',
                    border: '1px solid rgba(74,222,128,0.3)',
                    borderRadius: '8px',
                    color: '#86efac',
                    fontSize: '0.85rem',
                    fontWeight: 600,
                    cursor: emailSending ? 'not-allowed' : 'pointer',
                    transition: 'all 0.2s',
                    opacity: emailSending ? 0.7 : 1,
                  }}
                >
                  {emailSending ? (
                    <span style={{
                      display: 'inline-block',
                      width: '12px',
                      height: '12px',
                      border: '2px solid rgba(134,239,172,0.3)',
                      borderTopColor: '#86efac',
                      borderRadius: '50%',
                      animation: 'spin 0.7s linear infinite',
                    }} />
                  ) : (
                    <FaEnvelope />
                  )}
                  {emailSending ? 'Sending…' : 'Email me this week\'s alerts'}
                </button>
              </div>
            )}
          </div>
        </>,
        document.body
      )}
      <style>{`
        @keyframes spin {
          to { transform: rotate(360deg); }
        }
      `}</style>
    </>
  );
};

export default NotificationPanel;
