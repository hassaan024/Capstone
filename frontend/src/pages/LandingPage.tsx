// src/pages/LandingPage.tsx
import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import FeaturePieChart from "../components/FeaturePieChart";

type FeatureId = "chatbot" | "gardens" | "database" | "simulation" | "weather";

interface Feature {
  id: FeatureId;
  label: string;
  short: string;
  description: string;
  bullets: string[];
  color: string;
}

const features: Feature[] = [
  {
    id: "simulation",
    label: "Planting Prediction",
    short: "Know exactly when to plant each species.",
    description:
      "Set a target bloom date for your garden and LeafyLedger works backwards — factoring in each species' growth profile and local weather conditions to tell you the optimal planting date for every plant.",
    bullets: [
      "Per-species planting date based on your bloom target",
      "Weather-informed growth estimates, not just averages",
      "Never guess when to start your seeds again"
    ],
    color: "#f97316",
  },
  {
    id: "gardens",
    label: "Garden Digital Twin",
    short: "A living 3D mirror of your real garden.",
    description:
      "Your garden exists both in the real world and inside the app as an interactive 3D environment. Plant species in the app, watch them grow through simulated stages, and use it as a planning canvas before touching a single seed.",
    bullets: [
      "3D visualization synced to your real garden layout",
      "Growth stages from Seeded → Leafy → Bloom → Harvest",
      "Experiment with plant combinations before buying seeds"
    ],
    color: "#38bdf8",
  },
  {
    id: "chatbot",
    label: "AI Chatbot Assistant",
    short: "Your context-aware gardening coach.",
    description:
      "A floating AI assistant powered by Gemini that understands exactly what page or plant you are looking at to provide personalized tips and quick-replies.",
    bullets: [
      "Context-aware AI assistance anytime",
      "Dynamic prompt suggestions and quick-replies",
      "Customizable popup behaviors in Account Settings"
    ],
    color: "#22c55e",
  },
  {
    id: "database",
    label: "Global Plant Database",
    short: "Search thousands of species instantly.",
    description:
      "Tap into the Perenual API to browse, discover, and save plants while learning their sunlight, water, hardiness, and edibility metrics.",
    bullets: [
      "Extensive browsing and search capabilities",
      "Detailed health, edibility, and lifecycle stats",
      "Save favorites to your personal collection"
    ],
    color: "#a855f7",
  },
  {
    id: "weather",
    label: "Weather Sync",
    short: "Hyper-local climate conditions.",
    description:
      "Automatically map your garden locations to the Open-Meteo API to bring in real-time temperature, humidity, and atmospheric data that feeds directly into planting predictions.",
    bullets: [
      "Automated coordinate to weather-station mapping",
      "Live tracking of extreme temperature risks",
      "Weather data feeds the planting prediction model"
    ],
    color: "#eab308",
  },
];

const LandingPage: React.FC = () => {
  const navigate = useNavigate();
  const [selectedId, setSelectedId] = useState<FeatureId>("simulation");

  const selectedFeature =
    features.find((f) => f.id === selectedId) ?? features[0];

  return (
    <div className="leafy-landing-root">
      {/* Background accents */}
      <div className="leafy-landing-bg-gradient" />
      <div className="leafy-landing-bg-blob leafy-landing-bg-blob--left" />
      <div className="leafy-landing-bg-blob leafy-landing-bg-blob--right" />

      {/* NAVBAR */}
      <header className="leafy-landing-nav">
        <div className="leafy-landing-logo-row">
          <div className="leafy-landing-logo-mark" />
          <div className="leafy-landing-logo-text">LeafyLedger</div>
        </div>

        <nav className="leafy-landing-nav-actions">
          <button
            type="button"
            className="ll-btn ll-btn-ghost leafy-landing-nav-btn"
            onClick={() => navigate("/login")}
          >
            Log in
          </button>
          <button
            type="button"
            className="ll-btn ll-btn-primary leafy-landing-nav-btn"
            onClick={() => navigate("/login")}
          >
            Get started
          </button>
        </nav>
      </header>

      {/* MAIN CONTENT */}
      <main className="leafy-landing-main">
        {/* Centered hero banner (same width as map box) */}
        <section className="leafy-landing-hero">
          <div className="leafy-landing-hero-inner leafy-landing-shell">
            <div className="leafy-hero-glow" />
            <h1 className="leafy-landing-title">
              Plant at the <span>right time.</span> Watch your garden bloom.
            </h1>
            <p className="leafy-landing-subtitle">
              LeafyLedger predicts the optimal planting date for every species in your garden so everything blooms together — on your schedule. Pair that with a live 3D digital twin and AI-powered guidance.
            </p>

            <div className="leafy-landing-tag-row">
              <div className="ll-pill">Bloom Date Prediction</div>
              <div className="ll-pill">Garden Digital Twin</div>
              <div className="ll-pill">Weather-Informed</div>
            </div>

            {/* <div className="leafy-landing-stats-row leafy-landing-stats-row--center">
              <div className="leafy-landing-stat">
                <div className="leafy-landing-stat-label">Simulation</div>
                <div className="leafy-landing-stat-value">
                  Unreal Engine visuals
                </div>
              </div>
              <div className="leafy-landing-stat">
                <div className="leafy-landing-stat-label">Planner</div>
                <div className="leafy-landing-stat-value">
                  Web dashboard insights
                </div>
              </div>
            </div> */}

            {/* <div className="leafy-landing-cta-row leafy-landing-cta-row--center">
              <button
                type="button"
                className="ll-btn ll-btn-primary"
                onClick={() => navigate("/login")}
              >
                Get started
              </button>
              <button
                type="button"
                className="ll-btn ll-btn-ghost"
                onClick={() =>
                  document
                    .getElementById("leafy-feature-map")
                    ?.scrollIntoView({ behavior: "smooth" })
                }
              >
                Explore features
              </button>
            </div> */}
          </div>
        </section>

        {/* Feature / pie chart section below hero */}
        <section
          id="leafy-feature-map"
          className="leafy-landing-feature-section"
        >
          <div
            className="leafy-landing-pie-card leafy-landing-shell"
            aria-label="LeafyLedger feature map"
          >
            <div className="leafy-landing-pie-header">
              {/* <div> */}
                <div className="leafy-landing-pie-title">Feature map</div>
                <div className="leafy-landing-pie-subtitle">
                  Tap any slice or chip to explore a part of LeafyLedger.
                </div>
              {/* </div> */}
            </div>

            <div className="leafy-landing-pie-layout">
              <div className="leafy-landing-pie-wrapper">
                <FeaturePieChart
                  features={features.map((f) => ({
                    id: f.id,
                    label: f.label,
                    color: f.color,
                  }))}
                  selectedId={selectedId}
                  onSelect={(id) => setSelectedId(id as FeatureId)}
                />
              </div>

              <div className="leafy-landing-feature-panel">
                {/* Feature names inside the section: chips + heading */}
                <div className="leafy-landing-feature-chip-row">
                  {features.map((feature) => {
                    const isActive = feature.id === selectedId;
                    return (
                      <button
                        key={feature.id}
                        type="button"
                        className={
                          "leafy-landing-feature-chip" +
                          (isActive ? " leafy-landing-feature-chip--active" : "")
                        }
                        style={isActive ? {
                          borderColor: feature.color,
                          background: `${feature.color}22`,
                          color: feature.color,
                        } : {}}
                        onClick={() => setSelectedId(feature.id)}
                      >
                        {feature.label}
                      </button>
                    );
                  })}
                </div>

                <div className="leafy-landing-feature-main">
                  <div
                    className="leafy-landing-feature-heading"
                    style={{ color: selectedFeature.color }}
                  >
                    {selectedFeature.label}
                  </div>
                  <div className="leafy-landing-feature-short">
                    {selectedFeature.short}
                  </div>
                  <p className="leafy-landing-feature-description">
                    {selectedFeature.description}
                  </p>
                  <ul className="leafy-landing-feature-list">
                    {selectedFeature.bullets.map((b) => (
                      <li key={b} style={{ marginBottom: "4px" }}>{b}</li>
                    ))}
                  </ul>
                </div>
              </div>
            </div>
          </div>
        </section>
      </main>
    </div>
  );
};

export default LandingPage;
