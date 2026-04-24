import React, { useState } from 'react';
import { FaTimes, FaMapMarkerAlt, FaSeedling, FaCrosshairs } from 'react-icons/fa';
import { api } from '../utils/api';
import { fetchLongLatFromZipAndCountry } from '../utils/api';

interface CreateGardenModalProps {
  isOpen: boolean;
  onClose: () => void;
  userId: number;
  onCreated: () => void; // callback to refresh garden list
}

const CreateGardenModal: React.FC<CreateGardenModalProps> = ({
  isOpen,
  onClose,
  userId,
  onCreated,
}) => {
  const [name, setName] = useState('');
  const [description, setDescription] = useState('');
  const [bloomDate, setBloomDate] = useState('');
  const [zipCode, setZipCode] = useState('');
  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState('');
  const [step, setStep] = useState<'form' | 'success'>('form');
  const [locatingBrowser, setLocatingBrowser] = useState(false);
  const [resolvedLocation, setResolvedLocation] = useState<{ lat: number; lng: number; label: string } | null>(null);

  if (!isOpen) return null;

  const fetchTimezone = async (lat: number, lng: number): Promise<string | undefined> => {
    try {
      const res = await fetch(
        `https://api.open-meteo.com/v1/forecast?latitude=${lat}&longitude=${lng}&timezone=auto&forecast_days=1`
      );
      if (res.ok) {
        const data = await res.json();
        return data.timezone;
      }
    } catch {
      // optional
    }
    return undefined;
  };

  const resolveCityName = async (lat: number, lng: number): Promise<string> => {
    try {
      const res = await fetch(
        `https://api.bigdatacloud.net/data/reverse-geocode-client?latitude=${lat}&longitude=${lng}&localityLanguage=en`
      );
      if (res.ok) {
        const data = await res.json();
        if (data?.city || data?.locality) {
          return `${data.city || data.locality}, ${data.principalSubdivision || data.countryCode}`;
        }
      }
    } catch {
      // fallback
    }
    return `${lat.toFixed(4)}, ${lng.toFixed(4)}`;
  };

  const handleUseMyLocation = () => {
    if (!navigator.geolocation) {
      setError('Geolocation is not supported by your browser.');
      return;
    }

    setLocatingBrowser(true);
    setError('');
    setResolvedLocation(null);
    setZipCode('');

    navigator.geolocation.getCurrentPosition(
      async (position) => {
        const lat = position.coords.latitude;
        const lng = position.coords.longitude;
        const label = await resolveCityName(lat, lng);
        setResolvedLocation({ lat, lng, label });
        setLocatingBrowser(false);
      },
      (err) => {
        console.error('Geolocation error:', err);
        setError('Could not get your location. Please enter a ZIP code instead.');
        setLocatingBrowser(false);
      }
    );
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');

    if (!name.trim()) {
      setError('Please enter a garden name.');
      return;
    }
    if (!resolvedLocation && !zipCode.trim()) {
      setError('Please provide a location — use your browser location or enter a ZIP code.');
      return;
    }

    setSubmitting(true);
    try {
      let lat: number;
      let lng: number;

      if (resolvedLocation) {
        lat = resolvedLocation.lat;
        lng = resolvedLocation.lng;
      } else {
        const coords = await fetchLongLatFromZipAndCountry(zipCode, 'US');
        lat = coords.lat;
        lng = coords.lng;
      }

      const timezone = await fetchTimezone(lat, lng);

      await api.post('/garden', {
        ownerId: userId,
        name: name.trim(),
        description: description.trim() || null,
        latitude: lat,
        longitude: lng,
        timezone: timezone || null,
        bloomDate: bloomDate || undefined,
      });

      setStep('success');
      setTimeout(() => {
        onCreated();
        handleClose();
      }, 1500);
    } catch (err: any) {
      console.error('Failed to create garden:', err);
      if (err.message?.includes('No coordinates')) {
        setError('Could not find coordinates for that ZIP code. Please check and try again.');
      } else {
        setError('Failed to create garden. Please try again.');
      }
    } finally {
      setSubmitting(false);
    }
  };

  const handleClose = () => {
    setName('');
    setDescription('');
    setBloomDate('');
    setZipCode('');
    setError('');
    setStep('form');
    setSubmitting(false);
    setLocatingBrowser(false);
    setResolvedLocation(null);
    onClose();
  };

  const hasLocation = !!resolvedLocation || !!zipCode.trim();

  const inputStyle: React.CSSProperties = {
    width: '100%',
    padding: '0.75rem 1rem',
    background: 'rgba(255,255,255,0.05)',
    border: '1px solid rgba(148,163,184,0.2)',
    borderRadius: '10px',
    color: 'white',
    fontSize: '0.95rem',
    outline: 'none',
    transition: 'border-color 0.2s',
    boxSizing: 'border-box',
  };

  return (
    <div className="save-dest-overlay" onClick={handleClose}>
      <div
        className="save-dest-modal"
        onClick={(e) => e.stopPropagation()}
        style={{ maxWidth: '480px' }}
      >
        {/* Header */}
        <div className="save-dest-header">
          <h3 className="save-dest-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
            <FaSeedling style={{ color: '#4ade80' }} /> Create Garden
          </h3>
          <button className="save-dest-close" onClick={handleClose}>
            <FaTimes />
          </button>
        </div>

        {step === 'success' ? (
          <div style={{ padding: '3rem 1.5rem', textAlign: 'center' }}>
            <div style={{ fontSize: '3rem', marginBottom: '1rem' }}>🌱</div>
            <h3 style={{ color: 'white', margin: '0 0 0.5rem', fontSize: '1.2rem' }}>
              Garden Created!
            </h3>
            <p style={{ color: 'rgba(255,255,255,0.5)', fontSize: '0.9rem' }}>
              <strong>{name}</strong> is ready. Save plants to it from the Browse page.
            </p>
          </div>
        ) : (
          <form onSubmit={handleSubmit}>
            <div style={{ padding: '1rem 1.5rem', display: 'flex', flexDirection: 'column', gap: '1rem' }}>
              <p className="save-dest-subtitle" style={{ padding: 0, margin: 0 }}>
                Create a garden to organize and save plants for your space.
              </p>

              {/* Name */}
              <div>
                <label style={{ display: 'block', fontSize: '0.8rem', color: 'rgba(255,255,255,0.5)', textTransform: 'uppercase', letterSpacing: '0.05em', marginBottom: '0.4rem' }}>
                  Garden Name *
                </label>
                <input
                  type="text"
                  value={name}
                  onChange={(e) => setName(e.target.value)}
                  placeholder="e.g. Backyard Garden, Balcony Pots"
                  style={inputStyle}
                  onFocus={(e) => (e.target.style.borderColor = 'rgba(74,222,128,0.5)')}
                  onBlur={(e) => (e.target.style.borderColor = 'rgba(148,163,184,0.2)')}
                  autoFocus
                />
              </div>

              {/* Description */}
              <div>
                <label style={{ display: 'block', fontSize: '0.8rem', color: 'rgba(255,255,255,0.5)', textTransform: 'uppercase', letterSpacing: '0.05em', marginBottom: '0.4rem' }}>
                  Description (optional)
                </label>
                <textarea
                  value={description}
                  onChange={(e) => setDescription(e.target.value)}
                  placeholder="What kind of garden is this?"
                  rows={2}
                  style={{
                    ...inputStyle,
                    resize: 'vertical',
                    fontFamily: 'inherit',
                  }}
                  onFocus={(e) => (e.target.style.borderColor = 'rgba(74,222,128,0.5)')}
                  onBlur={(e) => (e.target.style.borderColor = 'rgba(148,163,184,0.2)')}
                />
              </div>

              {/* Bloom Date */}
              <div>
                <label style={{ display: 'block', fontSize: '0.8rem', color: 'rgba(255,255,255,0.5)', textTransform: 'uppercase', letterSpacing: '0.05em', marginBottom: '0.4rem' }}>
                  Target Bloom Date (optional)
                </label>
                <input
                  type="date"
                  value={bloomDate}
                  onChange={(e) => setBloomDate(e.target.value)}
                  min={new Date().toISOString().split('T')[0]}
                  style={{...inputStyle, cursor: 'text'}}
                  onFocus={(e) => (e.target.style.borderColor = 'rgba(74,222,128,0.5)')}
                  onBlur={(e) => (e.target.style.borderColor = 'rgba(148,163,184,0.2)')}
                />
                <p style={{ fontSize: '0.75rem', color: 'rgba(148,163,184,0.5)', marginTop: '0.35rem' }}>
                  When do you want this garden fully bloomed? We'll calculate planting times.
                </p>
              </div>

              {/* Location */}
              <div>
                <label style={{ display: 'block', fontSize: '0.8rem', color: 'rgba(255,255,255,0.5)', textTransform: 'uppercase', letterSpacing: '0.05em', marginBottom: '0.4rem' }}>
                  <FaMapMarkerAlt style={{ marginRight: '0.25rem' }} /> Location *
                </label>

                {/* Use My Location button */}
                <button
                  type="button"
                  onClick={handleUseMyLocation}
                  disabled={locatingBrowser}
                  style={{
                    width: '100%',
                    padding: '0.7rem 1rem',
                    background: resolvedLocation
                      ? 'rgba(74, 222, 128, 0.1)'
                      : 'rgba(56, 189, 248, 0.08)',
                    border: `1px solid ${resolvedLocation ? 'rgba(74, 222, 128, 0.35)' : 'rgba(56, 189, 248, 0.25)'}`,
                    borderRadius: '10px',
                    color: resolvedLocation ? '#86efac' : '#7dd3fc',
                    fontSize: '0.9rem',
                    cursor: locatingBrowser ? 'wait' : 'pointer',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                    gap: '0.5rem',
                    fontWeight: 600,
                    transition: 'all 0.2s',
                    marginBottom: '0.6rem',
                  }}
                >
                  <FaCrosshairs style={{ animation: locatingBrowser ? 'spin 1s linear infinite' : 'none' }} />
                  {locatingBrowser
                    ? 'Getting your location…'
                    : resolvedLocation
                      ? `📍 ${resolvedLocation.label}`
                      : 'Use My Location'}
                </button>

                {/* Divider */}
                <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', margin: '0.25rem 0' }}>
                  <div style={{ flex: 1, height: '1px', background: 'rgba(148,163,184,0.15)' }} />
                  <span style={{ fontSize: '0.7rem', color: 'rgba(148,163,184,0.4)', textTransform: 'uppercase', letterSpacing: '0.05em' }}>or</span>
                  <div style={{ flex: 1, height: '1px', background: 'rgba(148,163,184,0.15)' }} />
                </div>

                {/* ZIP code input */}
                <input
                  type="text"
                  value={zipCode}
                  onChange={(e) => {
                    setZipCode(e.target.value);
                    if (e.target.value.trim()) {
                      setResolvedLocation(null); // clear browser location when typing zip
                    }
                  }}
                  placeholder="Enter ZIP / Postal Code"
                  style={{ ...inputStyle, marginTop: '0.4rem' }}
                  onFocus={(e) => (e.target.style.borderColor = 'rgba(74,222,128,0.5)')}
                  onBlur={(e) => (e.target.style.borderColor = 'rgba(148,163,184,0.2)')}
                />

                <p style={{ fontSize: '0.75rem', color: 'rgba(148,163,184,0.5)', marginTop: '0.35rem' }}>
                  Used to determine weather and growing conditions for your garden.
                </p>
              </div>

              {/* Error */}
              {error && (
                <div style={{
                  padding: '0.75rem 1rem',
                  background: 'rgba(239, 68, 68, 0.1)',
                  border: '1px solid rgba(239, 68, 68, 0.3)',
                  borderRadius: '8px',
                  color: '#fca5a5',
                  fontSize: '0.85rem',
                }}>
                  {error}
                </div>
              )}
            </div>

            {/* Actions */}
            <div className="save-dest-actions" style={{ paddingTop: '0.5rem' }}>
              <button type="button" className="save-dest-cancel" onClick={handleClose}>
                Cancel
              </button>
              <button
                type="submit"
                className="save-dest-confirm"
                disabled={submitting || !name.trim() || !hasLocation}
              >
                {submitting ? 'Creating…' : 'Create Garden'}
              </button>
            </div>
          </form>
        )}
      </div>
    </div>
  );
};

export default CreateGardenModal;
