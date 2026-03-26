// src/pages/LandingPage.tsx
import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import FeaturePieChart from "../components/FeaturePieChart";

type FeatureId = "growth" | "climate" | "layout" | "finance" | "events";

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
    id: "growth",
    label: "Growth Timeline",
    short: "Seed to bloom to wither.",
    description:
      "Model how each plant moves through its life cycle so you can line up seed dates, bloom windows, and wither times.",
    bullets: [
      "Time to sprout, bloom, and wither",
      "See your garden at any date on a timeline",
      "Spot gaps or overlaps in color and coverage",
    ],
    color: "#22c55e",
  },
  {
    id: "climate",
    label: "Climate & Soil",
    short: "Match plants to your conditions.",
    description:
      "Use local climate expectations and soil nutrition to pick plants that can actually thrive where you live.",
    bullets: [
      "Temperature and humidity tolerances",
      "Sunlight needs vs. your location",
      "Stretch goal: soil nutrition modeling",
    ],
    color: "#38bdf8",
  },
  {
    id: "layout",
    label: "Garden Layout",
    short: "Design the space, not just the list.",
    description:
      "Shape the house and garden in Unreal, place plants, and see how the layout evolves over time.",
    bullets: [
      "Place plants in the engine",
      "House and garden shape customization",
      "See visual changes as the timeline moves",
    ],
    color: "#a855f7",
  },
  {
    id: "finance",
    label: "Financial View",
    short: "See cost and profit clearly.",
    description:
      "Estimate what your garden costs now and what it could be worth at peak bloom, especially for lumber or resale.",
    bullets: [
      "Cost of seeds and setup",
      "Yearly maintenance cost",
      "Value at bloom minus cost of seeds",
    ],
    color: "#f97316",
  },
  {
    id: "events",
    label: "Unexpected Events",
    short: "Floods, frosts, and surprises.",
    description:
      "Tell the system about floods, heat waves, or other events and refresh the simulation to see the impact.",
    bullets: [
      "User-entered events like floods",
      "Re-simulate plant health and outcomes",
      "Compare “before vs. after” timelines",
    ],
    color: "#eab308",
  },
];

const LandingPage: React.FC = () => {
  const navigate = useNavigate();
  const [selectedId, setSelectedId] = useState<FeatureId>("growth");

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
              Grow <span>smarter gardens</span>, not spreadsheets.
            </h1>
            <p className="leafy-landing-subtitle">
              LeafyLedger pulls plant lifetimes, climate limits, layout, and
              costs into one timeline so you can see how your garden will look
              and perform before you plant it.
            </p>

            <div className="leafy-landing-tag-row">
              <div className="ll-pill">Timeline of plant lifecycles</div>
              <div className="ll-pill">Climate &amp; soil checks</div>
              <div className="ll-pill">Cost vs. yield view</div>
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
                          (isActive
                            ? " leafy-landing-feature-chip--active"
                            : "")
                        }
                        onClick={() => setSelectedId(feature.id)}
                      >
                        {feature.label}
                      </button>
                    );
                  })}
                </div>

                <div className="leafy-landing-feature-main">
                  <div className="leafy-landing-feature-heading">
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
                      <li key={b}>{b}</li>
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
