import React, { useState, useEffect } from 'react';
import { createPortal } from 'react-dom';
import { useNavigate } from 'react-router-dom';
import { FaBell, FaTimes, FaCalendarAlt, FaLeaf, FaChevronDown, FaChevronUp, FaSeedling, FaEnvelope, FaClock } from 'react-icons/fa';
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
  bloomDate?: string | null;   // actual instance bloom date from Unreal
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
  const [scheduledTime, setScheduledTime] = useState('');
  const [scheduleStatus, setScheduleStatus] = useState<'idle' | 'scheduling' | 'scheduled'>('idle');
  const [scheduledLabel, setScheduledLabel] = useState('');
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
      // Pre-fill scheduled time to now + 2 minutes each time panel opens
      const d = new Date(Date.now() + 2 * 60 * 1000);
      d.setSeconds(0, 0);
      const pad = (n: number) => String(n).padStart(2, '0');
      const local = `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}T${pad(d.getHours())}:${pad(d.getMinutes())}`;
      setScheduledTime(local);
      setScheduleStatus('idle');
    }
    setIsOpen(!isOpen);
  };

  const handleSendEmail = async () => {
    if (!user?.id || emailSending) return;
    setEmailSending(true);
    try {
      const res = await api.post(`/garden/send-alert-email/${user.id}`, {});
      if (res.data.sent) {
        toast('Alert email sent! Check your inbox.', 'success');
      } else {
        toast('No active alerts to email right now.', 'info');
      }
    } catch {
      toast('Failed to send email. Check your app password in .env.', 'error');
    } finally {
      setEmailSending(false);
    }
  };

  const handleScheduleEmail = async () => {
    if (!user?.id || !scheduledTime || scheduleStatus === 'scheduling') return;
    setScheduleStatus('scheduling');
    try {
      const isoScheduledAt = new Date(scheduledTime).toISOString();
      const res = await api.post(`/garden/schedule-alert-email/${user.id}`, { scheduledAt: isoScheduledAt });
      if (res.data.scheduled) {
        const t = new Date(scheduledTime);
        const label = t.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
        setScheduledLabel(label);
        setScheduleStatus('scheduled');
        toast(`Email scheduled for ${label}!`, 'success');
      } else {
        toast('No active alerts found to schedule.', 'info');
        setScheduleStatus('idle');
      }
    } catch {
      toast('Failed to schedule. Check backend is running.', 'error');
      setScheduleStatus('idle');
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
                            bloomDate: alert.bloomDate ?? undefined,
                            soil: { type: 'Unknown' }
                          } as any}
                        />
                      </div>
                    )}
                  </div>
                ))
              )}
            </div>

            {/* Footer: schedule email */}
            {alerts.length > 0 && (
              <div style={{ padding: '1rem 1.25rem', borderTop: '1px solid rgba(148,163,184,0.1)' }}>
                {scheduleStatus === 'scheduled' ? (
                  /* Confirmation banner */
                  <div style={{ display: 'flex', alignItems: 'center', gap: '0.6rem', padding: '0.65rem 1rem', background: 'rgba(74,222,128,0.1)', border: '1px solid rgba(74,222,128,0.25)', borderRadius: '8px', color: '#86efac', fontSize: '0.85rem', fontWeight: 600 }}>
                    <FaClock style={{ flexShrink: 0 }} />
                    Email scheduled for {scheduledLabel}
                    <button
                      onClick={() => setScheduleStatus('idle')}
                      style={{ marginLeft: 'auto', background: 'none', border: 'none', color: 'rgba(148,163,184,0.5)', cursor: 'pointer', fontSize: '0.78rem' }}
                    >Reschedule</button>
                  </div>
                ) : (
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '0.55rem' }}>
                    <p style={{ margin: 0, fontSize: '0.75rem', color: 'rgba(148,163,184,0.55)', fontWeight: 600, textTransform: 'uppercase', letterSpacing: '0.06em' }}>
                      Schedule alert email
                    </p>
                    <div style={{ display: 'flex', gap: '0.5rem', alignItems: 'center' }}>
                      <input
                        type="datetime-local"
                        value={scheduledTime}
                        onChange={e => { setScheduledTime(e.target.value); setScheduleStatus('idle'); }}
                        style={{ flex: 1, padding: '0.48rem 0.7rem', background: 'rgba(15,23,42,0.6)', border: '1px solid rgba(148,163,184,0.2)', borderRadius: '7px', color: '#e2e8f0', fontSize: '0.83rem', colorScheme: 'dark', outline: 'none' }}
                      />
                      <button
                        onClick={handleScheduleEmail}
                        disabled={scheduleStatus === 'scheduling' || !scheduledTime}
                        style={{ display: 'flex', alignItems: 'center', gap: '0.4rem', padding: '0.48rem 0.9rem', background: 'rgba(74,222,128,0.12)', border: '1px solid rgba(74,222,128,0.3)', borderRadius: '7px', color: '#86efac', fontSize: '0.83rem', fontWeight: 600, cursor: scheduleStatus === 'scheduling' ? 'not-allowed' : 'pointer', whiteSpace: 'nowrap', opacity: scheduleStatus === 'scheduling' ? 0.7 : 1, transition: 'all 0.2s' }}
                      >
                        {scheduleStatus === 'scheduling' ? (
                          <span style={{ display: 'inline-block', width: '11px', height: '11px', border: '2px solid rgba(134,239,172,0.3)', borderTopColor: '#86efac', borderRadius: '50%', animation: 'spin 0.7s linear infinite' }} />
                        ) : <FaClock />}
                        {scheduleStatus === 'scheduling' ? 'Scheduling…' : 'Schedule'}
                      </button>
                    </div>
                    <button
                      onClick={handleSendEmail}
                      disabled={emailSending}
                      style={{ background: 'none', border: 'none', color: 'rgba(148,163,184,0.4)', fontSize: '0.76rem', cursor: 'pointer', padding: 0, textAlign: 'left', textDecoration: 'underline' }}
                    >
                      {emailSending ? 'Sending…' : 'Or send immediately'}
                    </button>
                  </div>
                )}
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
