// src/pages/LandingPage.tsx
import React, { useState, useMemo } from "react";
import { useNavigate } from "react-router-dom";
import FeaturePieChart, {
  FeatureCategory,
  FeatureCategoryId,
} from "../components/FeaturePieChart";
import FeatureDetailPanel from "../components/FeatureDetailPanel";
import FeatureCarousel from "../components/FeatureCarousel";

const LandingPage: React.FC = () => {
  const navigate = useNavigate();
  const [selectedFeatureId, setSelectedFeatureId] =
    useState<FeatureCategoryId | null>("lifecycle");

  const featureData: FeatureCategory[] = useMemo(
    () => [
      {
        id: "lifecycle",
        label: "Lifecycles",
        value: 25,
        accentColor: "#4ade80",
      },
      {
        id: "timeline",
        label: "Timeline & Sim",
        value: 20,
        accentColor: "#22c55e",
      },
      {
        id: "climate",
        label: "Climate Resilience",
        value: 20,
        accentColor: "#38bdf8",
      },
      {
        id: "finance",
        label: "Finance",
        value: 20,
        accentColor: "#f97316",
      },
      {
        id: "layout",
        label: "Layout",
        value: 15,
        accentColor: "#a855f7",
      },
    ],
    []
  );

  return (
    <div className="leafy-landing-root">
      <div className="ll-max-width">
        <div className="leafy-landing-layout">
          {/* Left: hero */}
          <section className="ll-card leafy-hero-card">
            <div className="leafy-hero-orbit" />
            <div className="leafy-hero-content">
              <div className="leafy-title-row">
                <div className="leafy-logo-dot" />
                <div>
                  <div className="ll-pill">LeafyLedger</div>
                  <div className="leafy-subtitle" style={{ marginTop: "0.3rem" }}>
                    Plan, simulate, and price your garden like a portfolio.
                  </div>
                </div>
              </div>

              <h1 className="leafy-title-main">
                Grow{" "}
                <span className="leafy-title-highlight">
                  smarter gardens
                </span>{" "}
                with data, not guesswork.
              </h1>

              <p className="leafy-subtitle">
                LeafyLedger combines plant lifetimes, climate tolerance, and
                financial modeling so you know{" "}
                <strong>what to plant, when to plant it,</strong> and whether
                it’s actually worth it.
              </p>

              <div className="leafy-hero-meta">
                <div className="ll-pill">
                  🌱 Lifecycle-aware simulations
                </div>
                <div className="ll-pill">🌦 Climate & stress testing</div>
                <div className="ll-pill">💰 Yearly profit estimates</div>
              </div>

              <div className="leafy-hero-actions">
                <button
                  className="ll-btn ll-btn-primary"
                  onClick={() => navigate("/login")}
                >
                  Get started
                </button>
                <button
                  className="ll-btn ll-btn-ghost"
                  onClick={() => {
                    const el = document.getElementById("leafy-features");
                    if (el) el.scrollIntoView({ behavior: "smooth" });
                  }}
                >
                  Explore features
                </button>
              </div>

              <div className="leafy-hero-footnote">
                Visual simulation powered by Unreal Engine. Analytics dashboard
                powered by LeafyLedger.
              </div>
            </div>
          </section>

          {/* Right: pie chart + detail */}
          <section className="leafy-landing-right" id="leafy-features">
            <div className="ll-card leafy-feature-panel">
              <div className="leafy-feature-header">
                <div>
                  <div className="leafy-feature-title">
                    Everything a garden needs — in one timeline.
                  </div>
                  <div className="leafy-feature-badge-row">
                    <span className="ll-pill">Lifecycle</span>
                    <span className="ll-pill">Climate</span>
                    <span className="ll-pill">Layout</span>
                    <span className="ll-pill">Finance</span>
                  </div>
                </div>
                <div style={{ fontSize: "0.75rem", color: "#9ca3af" }}>
                  Click a segment to learn more
                </div>
              </div>

              <div className="leafy-feature-body">
                <div>
                  <FeaturePieChart
                    data={featureData}
                    selectedId={selectedFeatureId}
                    onSelect={(id) => setSelectedFeatureId(id)}
                  />
                </div>
                <div>
                  <p className="leafy-feature-description">
                    LeafyLedger maps{" "}
                    <strong>plant lifetimes, climate inputs, layout</strong> and{" "}
                    <strong>costs</strong> into a single, navigable timeline.
                    Move time forward or backward and watch your garden grow,
                    bloom, and wither with real data behind every transition.
                  </p>
                  <div className="leafy-feature-highlight">
                    • See when each plant needs to be seeded to hit a shared
                    bloom window.  
                    • Adapt to floods or heatwaves and instantly refresh the
                    simulation.  
                    • Check if the garden still pays off at the end of the year.
                  </div>
                </div>
              </div>

              <FeatureDetailPanel selectedId={selectedFeatureId} />
            </div>

            <div className="ll-card leafy-carousel-card">
              <FeatureCarousel />
            </div>
          </section>
        </div>
      </div>
    </div>
  );
};

export default LandingPage;
