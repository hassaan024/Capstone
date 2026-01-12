// src/pages/LoginPage.tsx
import React from "react";
import { useNavigate } from "react-router-dom";
import { useGoogleLogin } from "@react-oauth/google";

const LoginPage: React.FC = () => {
  const navigate = useNavigate();
  const [isRegistering, setIsRegistering] = React.useState(false);
  const [email, setEmail] = React.useState("");
  const [password, setPassword] = React.useState("");
  const [firstName, setFirstName] = React.useState("");
  const [lastName, setLastName] = React.useState("");

  const handleGoogleLogin = useGoogleLogin({
    onSuccess: async (codeResponse) => {
        console.log("Google Login Success:", codeResponse);
        try {
            const res = await fetch("http://localhost:3000/auth/google", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ token: codeResponse.access_token }), // Adjust depending on flow (id_token vs access_token)
            });
            const data = await res.json();
            console.log("Backend response:", data);
        } catch (err) {
            console.error("Backend error:", err);
        }
    },
    onError: (error) => console.log("Google Login Failed:", error),
  });

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault();
    console.log("Registering:", { email, firstName, lastName });
    try {
        const res = await fetch("http://localhost:3000/auth/register", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ email, password, firstName, lastName }),
        });
        const data = await res.json();
        console.log("Registration response:", data);
        if (res.ok) {
            alert("Account created! Please log in.");
            setIsRegistering(false);
        } else {
            alert("Registration failed: " + data.message);
        }
    } catch (err) {
        console.error("Registration error:", err);
    }
  };

  const handleEmailLogin = async (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    console.log("Logging in:", email);
    try {
        const res = await fetch("http://localhost:3000/auth/login", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ email, password }),
        });
        const data = await res.json();
        console.log("Login response:", data);
         if (res.ok) {
            alert("Login successful!");
            // set user context, redirect...
        } else {
            alert("Login failed: " + (data.message || "Unknown error"));
        }
    } catch (err) {
        console.error("Login error:", err);
    }
  };

  const handleForgotPassword = () => {
    console.log("Forgot password clicked (UI only for now)");
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
            <div className="leafy-login-header-title">
              {isRegistering ? "Create Account" : "Sign in"}
            </div>
            <button
              type="button"
              className="leafy-login-header-link"
              onClick={() => navigate("/")}
            >
              Learn more
            </button>
          </div>

          {!isRegistering ? (
            /* LOGIN FORM */
            <form className="leafy-login-form" onSubmit={handleEmailLogin}>
              <input
                className="leafy-login-input"
                type="email"
                placeholder="Email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
              />
              <input
                className="leafy-login-input"
                type="password"
                placeholder="Password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
              />

              <div className="leafy-login-forgot-row">
                <button
                  type="button"
                  className="leafy-login-forgot-btn"
                  onClick={handleForgotPassword}
                >
                  Forgot password?
                </button>
              </div>

              <div className="leafy-login-buttons">
                <button
                  type="button"
                  className="ll-btn ll-btn-primary leafy-login-google-btn"
                  onClick={() => handleGoogleLogin()}
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
                  onClick={() => setIsRegistering(true)}
                >
                  Create account
                </button>
              </div>
            </form>
          ) : (
            /* REGISTER FORM */
            <form className="leafy-login-form" onSubmit={handleRegister}>
              <div style={{ display: "flex", gap: "10px" }}>
                <input
                  className="leafy-login-input"
                  type="text"
                  placeholder="First Name"
                  value={firstName}
                  onChange={(e) => setFirstName(e.target.value)}
                  required
                />
                <input
                  className="leafy-login-input"
                  type="text"
                  placeholder="Last Name"
                  value={lastName}
                  onChange={(e) => setLastName(e.target.value)}
                  required
                />
              </div>
              <input
                className="leafy-login-input"
                type="email"
                placeholder="Email"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                required
              />
              <input
                className="leafy-login-input"
                type="password"
                placeholder="Password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                required
              />

              <div className="leafy-login-buttons">
                <button type="submit" className="ll-btn ll-btn-primary">
                  Sign Up
                </button>

                <button
                  type="button"
                  className="ll-btn ll-btn-ghost"
                  onClick={() => setIsRegistering(false)}
                >
                  Back to Login
                </button>
              </div>
            </form>
          )}
        </div>
      </div>
    </div>
  );
};

export default LoginPage;
