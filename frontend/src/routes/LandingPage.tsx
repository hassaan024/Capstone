import React from "react";
import { Link } from "react-router-dom";
import FeaturePieChart from "../components/FeaturePieChart";

const APP_NAME = "LeafyLedger";

export default function LandingPage(): JSX.Element {
  const scrollToFeatures = () => {
    const el = document.getElementById("features");
    if (!el) return;
    el.scrollIntoView({ behavior: "smooth", block: "start" });
  };

  return (
    <div className="landing">
      <div
        className="blob"
        style={{ top: "-170px", left: "-160px", background: "rgba(27,182,115,0.20)" }}
      />
      <div
        className="blob"
        style={{
          bottom: "-220px",
          right: "-180px",
          background: "rgba(255,213,106,0.12)",
          animationDelay: "1.6s",
        }}
      />

      <div className="container">
        {/* Navbar (left brand, right actions) */}
        <header className="nav landingNav">
          <div className="brand">
            <div className="brandMark" />
            <div>{APP_NAME}</div>
          </div>

          <div className="navActions landingNavActions">
            <Link className="btn btn-primary" to="/login">
              Get started
            </Link>
            <Link className="btn btn-ghost" to="/login">
              Login
            </Link>
          </div>
        </header>

        {/* Hero: only "View features" CTA */}
        <section className="landingHero glass">
          <div className="landingHeroInner">
            <div className="badge">
              <span className="dot" /> Plants • Climate • Value • Time
            </div>

            <h1 className="h1 landingHeroTitle">The app you need for your garden.</h1>

            <p className="p landingHeroTagline">
              Plan what to plant, when to plant it, and what it’s worth — then visualize growth across time.
            </p>

            <div className="landingHeroCtas">
              <button className="btn btn-primary" type="button" onClick={scrollToFeatures}>
                View features
              </button>
            </div>

            <div className="landingHeroMini">
              <div className="landingMiniCard">
                <div className="landingMiniTop">Timeline</div>
                <div className="landingMiniSub">Move forward or backward</div>
              </div>
              <div className="landingMiniCard">
                <div className="landingMiniTop">Climate fit</div>
                <div className="landingMiniSub">Temp • humidity • sun-time</div>
              </div>
              <div className="landingMiniCard">
                <div className="landingMiniTop">Financials</div>
                <div className="landingMiniSub">Cost • value • profit</div>
              </div>
            </div>
          </div>
        </section>

        {/* Features */}
        <section id="features" className="landingFeaturesSection">
          <div className="landingSectionHeader">
            <div className="kicker">Features</div>
            <div className="landingSectionTitle">Explore what LeafyLedger can do</div>
            <div className="landingSectionSub">
              Click a slice to see a short explanation. Close it to re-center the chart.
            </div>
          </div>

          <FeaturePieChart />
        </section>

        <footer className="landingFooter">© {new Date().getFullYear()} {APP_NAME}</footer>
      </div>
    </div>
  );
}
