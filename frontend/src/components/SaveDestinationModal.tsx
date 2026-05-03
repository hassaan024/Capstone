import React, { useState, useEffect } from 'react';
import { FaTimes, FaGlobeAmericas, FaSeedling, FaCheck } from 'react-icons/fa';

interface Garden {
  id: number;
  name: string;
}

interface SaveDestinationModalProps {
  isOpen: boolean;
  onClose: () => void;
  plantName: string;
  gardens: Garden[];
  isGloballySaved: boolean;
  gardenSaveStates: Record<number, boolean>; // gardenId → isSaved
  onConfirm: (saveGlobal: boolean, gardenIds: number[]) => void;
}

const SaveDestinationModal: React.FC<SaveDestinationModalProps> = ({
  isOpen,
  onClose,
  plantName,
  gardens,
  isGloballySaved,
  gardenSaveStates,
  onConfirm,
}) => {
  const [globalChecked, setGlobalChecked] = useState(isGloballySaved);
  const [gardenChecks, setGardenChecks] = useState<Record<number, boolean>>({});

  // Sync internal state when modal opens or props change
  useEffect(() => {
    if (isOpen) {
      setGlobalChecked(isGloballySaved);
      setGardenChecks({ ...gardenSaveStates });
    }
  }, [isOpen, isGloballySaved, gardenSaveStates]);

  if (!isOpen) return null;

  const handleGardenToggle = (gardenId: number) => {
    setGardenChecks((prev) => ({
      ...prev,
      [gardenId]: !prev[gardenId],
    }));
  };

  const handleConfirm = () => {
    const selectedGardenIds = Object.entries(gardenChecks)
      .filter(([, checked]) => checked)
      .map(([id]) => Number(id));
    onConfirm(globalChecked, selectedGardenIds);
    onClose();
  };

  const hasChanges =
    globalChecked !== isGloballySaved ||
    gardens.some((g) => !!gardenChecks[g.id] !== !!gardenSaveStates[g.id]);

  return (
    <div className="save-dest-overlay" onClick={onClose}>
      <div className="save-dest-modal" onClick={(e) => e.stopPropagation()}>
        {/* Header */}
        <div className="save-dest-header">
          <h3 className="save-dest-title">Save Plant</h3>
          <button className="save-dest-close" onClick={onClose}>
            <FaTimes />
          </button>
        </div>

        <p className="save-dest-subtitle">
          Choose where to save <strong>{plantName}</strong>
        </p>

        {/* Options */}
        <div className="save-dest-options">
          {/* Global save */}
          <label className={`save-dest-option ${globalChecked ? 'checked' : ''}`}>
            <div className="save-dest-checkbox">
              {globalChecked && <FaCheck />}
            </div>
            <div className="save-dest-option-icon global">
              <FaGlobeAmericas />
            </div>
            <div className="save-dest-option-text">
              <span className="save-dest-option-name">Save Globally</span>
              <span className="save-dest-option-desc">Available across all gardens in the app</span>
            </div>
            <input
              type="checkbox"
              checked={globalChecked}
              onChange={() => setGlobalChecked(!globalChecked)}
              style={{ display: 'none' }}
            />
          </label>

          {/* Divider */}
          {gardens.length > 0 && (
            <div className="save-dest-divider">
              <span>Your Gardens</span>
            </div>
          )}

          {/* Garden options */}
          {gardens.map((garden) => {
            const isChecked = !!gardenChecks[garden.id];
            return (
              <label
                key={garden.id}
                className={`save-dest-option ${isChecked ? 'checked' : ''}`}
              >
                <div className="save-dest-checkbox">
                  {isChecked && <FaCheck />}
                </div>
                <div className="save-dest-option-icon garden">
                  <FaSeedling />
                </div>
                <div className="save-dest-option-text">
                  <span className="save-dest-option-name">{garden.name}</span>
                  <span className="save-dest-option-desc">Save to this garden</span>
                </div>
                <input
                  type="checkbox"
                  checked={isChecked}
                  onChange={() => handleGardenToggle(garden.id)}
                  style={{ display: 'none' }}
                />
              </label>
            );
          })}

          {gardens.length === 0 && (
            <div className="save-dest-empty">
              <FaSeedling style={{ fontSize: '1.5rem', opacity: 0.4, marginBottom: '0.5rem' }} />
              <p>No gardens yet. Create one in the app to save plants to specific gardens.</p>
            </div>
          )}
        </div>

        {/* Actions */}
        <div className="save-dest-actions">
          <button className="save-dest-cancel" onClick={onClose}>
            Cancel
          </button>
          <button
            className="save-dest-confirm"
            onClick={handleConfirm}
            disabled={!hasChanges}
          >
            {hasChanges ? 'Save Changes' : 'No Changes'}
          </button>
        </div>
      </div>
    </div>
  );
};

export default SaveDestinationModal;
