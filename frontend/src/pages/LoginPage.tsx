// src/pages/LoginPage.tsx
import React from "react";
import { useNavigate } from "react-router-dom";

const LoginPage: React.FC = () => {
  const navigate = useNavigate();

  // Placeholder: later this will trigger your real OAuth redirect
  const handleGoogleLogin = () => {
    console.log("Google OAuth login clicked");
    // window.location.href = "https://your-oauth-endpoint"; // later
  };

  const handleEmailLogin = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    console.log("Email/password sign-in (UI only for now)");
  };

  const handleForgotPassword = () => {
    console.log("Forgot password clicked (UI only for now)");
  };

  const handleCreateAccount = () => {
    console.log("Create account clicked (UI only for now)");
    // Later: navigate("/signup") or trigger signup flow
  };

  return (
    <div className="leafy-login-root">
      {/* Left visual side */}
      <div className="ll-card leafy-login-visual">
        <div className="leafy-login-visual-gradient" />
        <div className="leafy-login-blob leafy-login-blob--green" />
        <div className="leafy-login-blob leafy-login-blob--blue" />
        <div className="leafy-login-blob leafy-login-blob--amber" />

        <div className="leafy-login-overlay">
          <header className="leafy-login-logo-row">
            <div className="leafy-login-logo-mark" />
            <div className="leafy-login-logo-text">LeafyLedger</div>
          </header>

          <div className="leafy-login-hero-copy">
            <h1 className="leafy-login-hero-title">
              Plan <span>smarter gardens</span> with clear data.
            </h1>
            <p className="leafy-login-hero-subtitle">
              LeafyLedger helps you line up seed dates, bloom windows, climate
              limits, and yearly costs so you can see how your garden evolves
              before you plant it.
            </p>

            <div className="leafy-login-icon-row">
              <div className="leafy-login-icon-pill">
                <span>🌱</span>
                <span>Plant lifetimes</span>
              </div>
              <div className="leafy-login-icon-pill">
                <span>🌦</span>
                <span>Climate tolerance</span>
              </div>
              <div className="leafy-login-icon-pill">
                <span>💰</span>
                <span>Yearly profit view</span>
              </div>
            </div>

            <div className="leafy-login-growth-card">
              <div className="leafy-login-growth-header">
                <div className="leafy-login-growth-title">
                  Growth timeline preview
                </div>
                <div className="leafy-login-growth-badge">Live simulation</div>
              </div>

              <div className="leafy-login-growth-bar-shell">
                <div className="leafy-login-growth-bar-fill" />
              </div>

              <div className="leafy-login-growth-stages">
                <div className="leafy-login-growth-stage">
                  <div className="leafy-login-growth-stage-label">Start</div>
                  <div>Seeded</div>
                </div>
                <div className="leafy-login-growth-stage">
                  <div className="leafy-login-growth-stage-label">Mid</div>
                  <div>Leafy growth</div>
                </div>
                <div className="leafy-login-growth-stage">
                  <div className="leafy-login-growth-stage-label">Peak</div>
                  <div>Bloom window</div>
                </div>
                <div className="leafy-login-growth-stage">
                  <div className="leafy-login-growth-stage-label">End</div>
                  <div>Wither & profit</div>
                </div>
              </div>
            </div>

            <div className="leafy-login-bottom-hint">
              Visual simulation runs in Unreal Engine. LeafyLedger keeps the
              numbers behind it honest.
            </div>
          </div>
        </div>

        <div className="leafy-growth-arc" />
      </div>

      {/* Right form side */}
      <div className="leafy-login-form-side">
        <div className="leafy-login-card">
          {/* Heading + Learn more */}
          <div className="leafy-login-header-row">
            <div className="leafy-login-header-title">Sign in</div>
            <button
              type="button"
              className="leafy-login-header-link"
              onClick={() => navigate("/")}
            >
              Learn more
            </button>
          </div>

          {/* Form: email → password → forgot → Google → create */}
          <form className="leafy-login-form" onSubmit={handleEmailLogin}>
            {/* 1. Email */}
            <input
              className="leafy-login-input"
              type="email"
              placeholder="Email"
            />

            {/* 2. Password */}
            <input
              className="leafy-login-input"
              type="password"
              placeholder="Password"
            />

            {/* 3. Forgot password */}
            <div className="leafy-login-forgot-row">
              <button
                type="button"
                className="leafy-login-forgot-btn"
                onClick={handleForgotPassword}
              >
                Forgot password?
              </button>
            </div>

            {/* 4 + 5. Google + Create account buttons */}
            <div className="leafy-login-buttons">
              <button
                type="button"
                className="ll-btn ll-btn-primary leafy-login-google-btn"
                onClick={handleGoogleLogin}
              >
                <img
                  src="https://www.gstatic.com/firebasejs/ui/2.0.0/images/auth/google.svg"
                  alt="Google logo"
                />
                Continue with Google
              </button>

              <button
                type="button"
                className="ll-btn ll-btn-ghost leafy-login-create-btn"
                onClick={handleCreateAccount}
              >
                Create account
              </button>
            </div>
          </form>
        </div>
      </div>
    </div>
  );
};

export default LoginPage;
