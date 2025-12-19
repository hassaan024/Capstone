import React from "react";
import { Link } from "react-router-dom";

export default function LandingPage() {
  return (
    <div className="landing">
      {/* floating blobs */}
      <div className="blob" style={{ top: "-160px", left: "-140px", background: "rgba(81,242,156,0.35)" }} />
      <div className="blob" style={{ bottom: "-220px", right: "-160px", background: "rgba(255,213,106,0.22)", animationDelay: "1.6s" }} />

      {/* particles */}
      <div className="particles" aria-hidden="true">
        {Array.from({ length: 16 }).map((_, i) => (
          <span
            key={i}
            style={{
              left: `${(i * 7) % 100}%`,
              bottom: `-${10 + (i % 6) * 18}px`,
              animationDelay: `${(i % 8) * 0.55}s`,
              animationDuration: `${9 + (i % 5) * 1.2}s`,
              opacity: 0.25 + (i % 5) * 0.12,
              transform: `rotate(${35 + i * 9}deg)`,
            }}
          />
        ))}
      </div>

      <div className="container">
        <header className="nav">
          <div className="brand">
            <div className="brandMark" />
            <div>LeafyLedger</div>
          </div>

          <div className="navActions">
            <Link className="btn btn-ghost" to="/login">Log in</Link>
            <Link className="btn btn-primary" to="/login">Get started</Link>
          </div>
        </header>

        <section className="hero">
          <div className="heroLeft glass">
            <div className="kickerRow">
              <span className="badge"><span className="dot" /> Climate + Growth + Value</span>
              <span className="badge" style={{ background: "rgba(255,213,106,0.10)", borderColor: "rgba(255,213,106,0.18)", color: "rgba(255,213,106,0.95)" }}>
                Timeline-ready
              </span>
            </div>

            <h1 className="h1">
              Plan your garden like a{" "}
              <span className="highlight">system</span>.
            </h1>

            <p className="p">
              LeafyLedger helps you model plant lifecycles, forecast climate fit, and estimate value — all in one place.
              Build once, simulate across time, and make decisions with confidence.
            </p>

            <div className="heroCtas">
              <Link className="btn btn-primary" to="/login">Continue</Link>
              <a className="btn btn-ghost" href="#preview">See preview</a>
            </div>

            <div className="metrics">
              <div className="metric">
                <div className="num">Lifecycle</div>
                <div className="lbl">Seed → Bloom → Wither</div>
              </div>
              <div className="metric">
                <div className="num">Climate</div>
                <div className="lbl">Temp • Humidity • Sun</div>
              </div>
              <div className="metric">
                <div className="num">Value</div>
                <div className="lbl">Costs • Returns • Profit</div>
              </div>
            </div>
          </div>

          <div className="heroRight glass" id="preview">
            <div className="cardTitle">
              <div style={{ fontWeight: 900, letterSpacing: "-0.01em" }}>Today’s Snapshot</div>
              <div className="pill">Demo UI</div>
            </div>

            <div className="preview">
              <div className="previewRow">
                <div className="previewLeft">
                  <div className="previewTop"><span className="iconDot" /> Weather</div>
                  <div className="previewSub">Local forecast impact</div>
                </div>
                <div className="previewVal">72°F • 58% • 7h sun</div>
              </div>

              <div className="previewRow">
                <div className="previewLeft">
                  <div className="previewTop"><span className="iconDot" /> Garden Value</div>
                  <div className="previewSub">Projected bloom return</div>
                </div>
                <div className="previewVal">$1,240</div>
              </div>

              <div className="previewRow">
                <div className="previewLeft">
                  <div className="previewTop"><span className="iconDot" /> Risk</div>
                  <div className="previewSub">Cold snap sensitivity</div>
                </div>
                <div className="previewVal">Low</div>
              </div>
            </div>

            <div className="footerStrip">
              <div className="chip">Time travel</div>
              <div className="chip">Event markers</div>
              <div className="chip">Price tracking</div>
            </div>
          </div>
        </section>
      </div>
    </div>
  );
}
