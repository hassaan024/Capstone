// src/pages/LoginPage.tsx
import React from "react";
import { useNavigate } from "react-router-dom";
import { useGoogleLogin } from "@react-oauth/google";
import { useAuth } from "../context/AuthContext";
import { BACKEND_BASE_URL, GOOGLE_OAUTH_CLIENT_ID} from "../utils/constants";
import { FaEye, FaEyeSlash, FaSeedling, FaCloudSun, FaCoins } from 'react-icons/fa';

const LoginPage: React.FC = () => {
  const navigate = useNavigate();
  const { login } = useAuth();
  const [isRegistering, setIsRegistering] = React.useState(false);
  
  // Form fields
  const [email, setEmail] = React.useState("");
  const [password, setPassword] = React.useState("");
  const [confirmPassword, setConfirmPassword] = React.useState("");
  const [firstName, setFirstName] = React.useState("");
  const [lastName, setLastName] = React.useState("");
  
  // UI state
  const [showPassword, setShowPassword] = React.useState(false);
  const [showConfirmPassword, setShowConfirmPassword] = React.useState(false);
  const [message, setMessage] = React.useState<{ type: 'success' | 'error', text: string } | null>(null);
  const [isLoading, setIsLoading] = React.useState(false);

  const handleGoogleLogin = useGoogleLogin({
    flow: 'auth-code', // tells it to return a authentication code for the backend to exchange with a token
    onSuccess: async ({ code }) => {
        // console.log("User granted permission through google.");
        // console.log(code)
        try {
          const res = await fetch(`${BACKEND_BASE_URL}/auth/google/react`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ code }),
          });
          const data = await res.json();
          
          if (res.ok) {
            console.log("Logged-in user through google:", data);
            login(data);
            navigate("/dashboard");
          } else {
             console.error("Backend login failed:", data);
             setMessage({ type: 'error', text: data.message || 'Google login failed' });
          }
        } catch (err) {
            console.error("Error hitting backend oauth endpoint:", err);
            setMessage({ type: 'error', text: 'Could not connect to server' });
        }
    },
    onError: (error) => console.log("Google Login Failed:", error),
  });

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault();
    setMessage(null);
    
    // Validation
    if (password !== confirmPassword) {
      setMessage({ type: 'error', text: 'Passwords do not match' });
      return;
    }
    
    if (password.length < 6) {
      setMessage({ type: 'error', text: 'Password must be at least 6 characters' });
      return;
    }
    
    setIsLoading(true);
    
    try {
        const res = await fetch(`${BACKEND_BASE_URL}/auth/register`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ email, password, firstName, lastName }),
        });
        const data = await res.json();
        
        if (res.ok) {
            setMessage({ type: 'success', text: 'Account created! Please log in.' });
            setIsRegistering(false);
            // Clear form fields
            setConfirmPassword("");
            setFirstName("");
            setLastName("");
        } else {
            const errorMsg = data.message || (Array.isArray(data.message) ? data.message.join(', ') : 'Unknown error');
            setMessage({ type: 'error', text: errorMsg });
        }
    } catch (err) {
        console.error("Registration error:", err);
        setMessage({ type: 'error', text: 'Could not connect to server' });
    } finally {
        setIsLoading(false);
    }
  };

  const handleEmailLogin = async (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    setMessage(null);
    setIsLoading(true);
    
    try {
        const res = await fetch(`${BACKEND_BASE_URL}/auth/login`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ email, password }),
        });
        const data = await res.json();
        
        if (res.ok) {
            setMessage({ type: 'success', text: 'Login successful! Redirecting...' });
            // Store user data and redirect
            login(data.user);
            setTimeout(() => navigate("/dashboard"), 500);
        } else {
            setMessage({ type: 'error', text: data.message || 'Login failed' });
        }
    } catch (err) {
        console.error("Login error:", err);
        setMessage({ type: 'error', text: 'Could not connect to server' });
    } finally {
        setIsLoading(false);
    }
  };

  const handleForgotPassword = () => {
    setMessage({ type: 'error', text: 'Password reset coming soon!' });
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
                <span><FaSeedling /></span>
                <span>Plant lifetimes</span>
              </div>
              <div className="leafy-login-icon-pill">
                <span><FaCloudSun /></span>
                <span>Climate tolerance</span>
              </div>
              <div className="leafy-login-icon-pill">
                <span><FaCoins /></span>
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
              Sign in {/* {isRegistering ? "Create Account" : "Sign in"} */}
            </div>
            <button
              type="button"
              className="leafy-login-header-link"
              onClick={() => navigate("/")}
            >
              Learn more
            </button>
          </div>

          {/* Email/Password login options have been disabled/commented-out per request. */}

          {/* New Google Only Login */}
          <div className="leafy-login-form">
            <div className="leafy-login-buttons" style={{ marginTop: '2rem' }}>
              <button
                type="button"
                className="ll-btn ll-btn-ghost leafy-login-google-btn"
                style={{ border: "1px solid rgba(148, 163, 184, 0.4)", width: "100%", padding: "1rem" }}
                onClick={() => handleGoogleLogin()}
              >
                  <img
                    src="https://www.gstatic.com/firebasejs/ui/2.0.0/images/auth/google.svg"
                    alt="Google logo"
                  />
                  Sign in with Google
              </button>
            </div>
            {message && (
              <div className={`leafy-login-message leafy-login-message--${message.type}`} style={{marginTop: '1rem'}}>
                {message.text}
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};

export default LoginPage;
