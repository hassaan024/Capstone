import React, { useState } from "react";
import { Link, useNavigate } from "react-router-dom";

export default function Login() {
  const nav = useNavigate();
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");

  const onSubmit = (e) => {
    e.preventDefault();
    // placeholder until OAuth is wired
    nav("/dashboard");
  };

  const onGoogle = () => {
    // placeholder for OAuth redirect
    // later: window.location.href = `${BACKEND_URL}/auth/google` or provider URL
    nav("/dashboard");
  };

  return (
    <div className="login">
      {/* particles on the left */}
      <div className="particles" aria-hidden="true">
        {Array.from({ length: 14 }).map((_, i) => (
          <span
            key={i}
            style={{
              left: `${(i * 9) % 100}%`,
              bottom: `-${12 + (i % 7) * 16}px`,
              animationDelay: `${(i % 7) * 0.6}s`,
              animationDuration: `${9 + (i % 6) * 1.3}s`,
              opacity: 0.22 + (i % 5) * 0.12,
            }}
          />
        ))}
      </div>

      <section className="loginLeft">
        <div className="loginLeftInner">
          <div className="heroCard">
            <div className="brand" style={{ marginBottom: 12 }}>
              <div className="brandMark" />
              <div>LeafyLedger</div>
            </div>

            <div className="kicker">Welcome back</div>
            <div className="h1" style={{ fontSize: "40px", marginTop: 10 }}>
              Grow smarter with
              <span style={{
                background: "linear-gradient(135deg, var(--mint), var(--leaf))",
                WebkitBackgroundClip: "text",
                backgroundClip: "text",
                color: "transparent",
              }}> timelines</span>.
            </div>

            <p className="mini">
              Sign in to access your dashboard, climate insights, and garden value projections.
              OAuth will be enabled here (Google sign-in) in the next sprint.
            </p>

            <div className="leafWave" aria-hidden="true">
              <div className="waveRow" />
              <div className="waveRow" />
              <div className="waveRow" />
            </div>

            <div style={{ marginTop: 18, color: "var(--subtle)", fontWeight: 800, fontSize: 13 }}>
              Tip: Keep your plant assumptions realistic — the simulator becomes more accurate as your inputs improve.
            </div>
          </div>
        </div>
      </section>

      <section className="loginRight">
        <div className="glass loginCard">
          <div className="loginHeader">
            <div>
              <div className="loginTitle">Sign in</div>
              <p className="loginSub">Use Google (recommended) or continue with email for now.</p>
            </div>

            <Link className="btn btn-ghost" to="/" style={{ padding: "10px 14px" }}>
              Home
            </Link>
          </div>

          <button className="oauthBtn" type="button" onClick={onGoogle}>
            <span className="googleMark" />
            Continue with Google
          </button>

          <div style={{ display: "flex", alignItems: "center", gap: 12, margin: "18px 0" }}>
            <div className="hr" style={{ flex: 1 }} />
            <div style={{ color: "var(--subtle)", fontWeight: 900, fontSize: 12 }}>OR</div>
            <div className="hr" style={{ flex: 1 }} />
          </div>

          <form className="loginForm" onSubmit={onSubmit}>
            <div>
              <label className="label" htmlFor="email">Email</label>
              <input
                id="email"
                className="input"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                placeholder="you@school.edu"
                autoComplete="email"
              />
            </div>

            <div>
              <label className="label" htmlFor="password">Password</label>
              <input
                id="password"
                className="input"
                type="password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                placeholder="••••••••"
                autoComplete="current-password"
              />
            </div>

            <button className="btn btn-primary" type="submit">
              Continue
            </button>

            <div className="tinyLinks">
              <span>New here? <a href="#" onClick={(e) => e.preventDefault()}>Create account</a></span>
              <a href="#" onClick={(e) => e.preventDefault()}>Forgot password?</a>
            </div>
          </form>

          <div style={{ marginTop: 18, color: "var(--subtle)", fontWeight: 800, fontSize: 12, lineHeight: 1.6 }}>
            OAuth note: when backend is ready, the Google button will redirect to the provider and return to a callback route.
          </div>
        </div>
      </section>
    </div>
  );
}
