import React, { useState, useRef, useEffect } from 'react';
import { api } from '../utils/api';
import { useAuth } from '../context/AuthContext';
import { FaPaperPlane, FaTimes, FaSeedling, FaExpand, FaCompress } from 'react-icons/fa';
import '../styles/ChatWidget.css';

interface ChatMessage {
  role: 'user' | 'model';
  text: string;
  isError?: boolean;
}

const SUGGESTIONS = [
  "What plants are good for beginners?",
  "How often should I water succulents?",
  "What care tips do you have for my plants?",
];

/**
 * Simple markdown-to-JSX renderer for bold, lists, and line breaks.
 */
function renderMarkdown(text: string): React.ReactNode[] {
  const lines = text.split('\n');
  const elements: React.ReactNode[] = [];
  let listItems: string[] = [];
  let listKey = 0;

  const flushList = () => {
    if (listItems.length > 0) {
      elements.push(
        <ul key={`list-${listKey++}`}>
          {listItems.map((item, i) => (
            <li key={i}>{renderInline(item)}</li>
          ))}
        </ul>
      );
      listItems = [];
    }
  };

  const renderInline = (line: string): React.ReactNode => {
    // Handle **bold** and *italic*
    const parts = line.split(/(\*\*[^*]+\*\*|\*[^*]+\*)/g);
    return parts.map((part, i) => {
      if (part.startsWith('**') && part.endsWith('**')) {
        return <strong key={i}>{part.slice(2, -2)}</strong>;
      }
      if (part.startsWith('*') && part.endsWith('*')) {
        return <em key={i}>{part.slice(1, -1)}</em>;
      }
      return part;
    });
  };

  lines.forEach((line, i) => {
    const trimmed = line.trim();

    // Bullet list items
    if (trimmed.startsWith('- ') || trimmed.startsWith('• ') || /^\d+\.\s/.test(trimmed)) {
      const content = trimmed.replace(/^[-•]\s|^\d+\.\s/, '');
      listItems.push(content);
      return;
    }

    flushList();

    if (trimmed === '') {
      // Skip empty lines
      return;
    }

    elements.push(<p key={`p-${i}`}>{renderInline(trimmed)}</p>);
  });

  flushList();
  return elements;
}

const ChatWidget: React.FC = () => {
  const { user } = useAuth();
  const [isOpen, setIsOpen] = useState(false);
  const [isExpanded, setIsExpanded] = useState(false);
  const [messages, setMessages] = useState<ChatMessage[]>([]);
  const [input, setInput] = useState('');
  const [loading, setLoading] = useState(false);
  const [suggestionPopup, setSuggestionPopup] = useState<string | null>(null);
  const [contextPlant, setContextPlant] = useState<string | null>(null);
  const [usedQuickReplies, setUsedQuickReplies] = useState<string[]>([]);
  const messagesEndRef = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);

  // Listen for suggestion events from across the app
  useEffect(() => {
    const handleSuggest = (e: Event) => {
      const customEvent = e as CustomEvent;
      const suggestionText = customEvent.detail;
      setSuggestionPopup(suggestionText);

      // Track contextual plant if the suggestion came from PlantDetailsModal
      const plantMatch = suggestionText.match(/about (.+)!/);
      if (plantMatch) {
         setContextPlant(plantMatch[1]);
         setUsedQuickReplies([]); // Reset quick replies for new plant
      } else {
         setContextPlant(null);
      }

      // Auto close after 5s
      setTimeout(() => {
        setSuggestionPopup(null);
      }, 5000);
    };
    window.addEventListener('suggestChat', handleSuggest);
    return () => window.removeEventListener('suggestChat', handleSuggest);
  }, []);

  const handleSuggestionClick = () => {
    if (suggestionPopup) {
      setIsOpen(true);
      sendMessage(suggestionPopup);
      setSuggestionPopup(null);
    }
  };

  // Scroll to bottom on new messages
  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [messages, loading]);

  // Focus input when panel opens
  useEffect(() => {
    if (isOpen) {
      setTimeout(() => inputRef.current?.focus(), 100);
    }
  }, [isOpen]);

  const sendMessage = async (text?: string) => {
    const msgText = (text || input).trim();
    if (!msgText || loading) return;

    const userMsg: ChatMessage = { role: 'user', text: msgText };
    setMessages(prev => [...prev, userMsg]);
    setInput('');
    setLoading(true);

    try {
      // Build history in Gemini format
      const history = messages.map(m => ({
        role: m.role,
        parts: [{ text: m.text }],
      }));

      const { data } = await api.post('/chat/message', {
        message: msgText,
        userId: user?.id,
        history,
      });

      if (data.error) {
        setMessages(prev => [
          ...prev,
          { role: 'model', text: data.message, isError: true },
        ]);
      } else {
        setMessages(prev => [
          ...prev,
          { role: 'model', text: data.reply },
        ]);
      }
    } catch (err) {
      console.error('Chat error:', err);
      setMessages(prev => [
        ...prev,
        {
          role: 'model',
          text: 'Something went wrong connecting to the AI. Please try again.',
          isError: true,
        },
      ]);
    } finally {
      setLoading(false);
    }
  };

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    sendMessage();
  };

  return (
    <>
      {/* Suggestion Popup Tooltip */}
      {suggestionPopup && !isOpen && (
        <div 
          className="chat-suggestion-tooltip"
          onClick={handleSuggestionClick}
        >
          <div className="chat-suggestion-text-container">
            <span className="chat-suggestion-bulb">💡</span>
            <div>
              <div className="chat-suggestion-title">Ask me about this!</div>
              <div className="chat-suggestion-subtext">"{suggestionPopup}"</div>
            </div>
          </div>
          <button 
            className="chat-suggestion-close" 
            onClick={(e) => { e.stopPropagation(); setSuggestionPopup(null); }}
          >
             <FaTimes size={12} />
          </button>
          <div className="chat-suggestion-timer-bar" key={suggestionPopup} />
        </div>
      )}

      {/* Floating Action Button */}
      <button
        className={`chat-fab ${isOpen ? 'chat-fab--open' : ''}`}
        onClick={() => setIsOpen(!isOpen)}
        title={isOpen ? 'Close chat' : 'Ask Leafy 🌿'}
      >
        {isOpen ? <FaTimes /> : <FaSeedling />}
      </button>

      {/* Chat Panel */}
      {isOpen && (
        <div className={`chat-panel ${isExpanded ? 'chat-panel--expanded' : ''}`}>
          {/* Header */}
          <div className="chat-header">
            <div className="chat-header-left">
              <div className="chat-avatar">🌿</div>
              <div className="chat-header-info">
                <h4>Leafy</h4>
                <span>
                  <span className="chat-online-dot"></span>
                  Plant care assistant
                </span>
              </div>
            </div>
            <button
              className="chat-expand-btn"
              onClick={() => setIsExpanded(prev => !prev)}
              title={isExpanded ? 'Minimize chat' : 'Expand chat'}
            >
              {isExpanded ? <FaCompress /> : <FaExpand />}
            </button>
          </div>

          {/* Messages */}
          <div className="chat-messages">
            {messages.length === 0 && !loading && (
              <div className="chat-welcome">
                <div className="chat-welcome-icon">🌱</div>
                <h4>Hi{user ? `, ${user.displayName}` : ''}! I'm Leafy</h4>
                <p>Your personal plant care assistant. Ask me anything about plants, gardening, or your collection!</p>
                <div className="chat-welcome-suggestions">
                  {SUGGESTIONS.map((s, i) => (
                    <button
                      key={i}
                      className="chat-suggestion-btn"
                      onClick={() => sendMessage(s)}
                    >
                      {s}
                    </button>
                  ))}
                </div>
              </div>
            )}

            {messages.map((msg, i) => (
              <div
                key={i}
                className={`chat-msg ${
                  msg.role === 'user'
                    ? 'chat-msg--user'
                    : msg.isError
                      ? 'chat-msg--error'
                      : 'chat-msg--bot'
                }`}
              >
                {msg.role === 'user' ? msg.text : renderMarkdown(msg.text)}
              </div>
            ))}

            {/* Quick Replies styled exactly like welcome suggestions */}
            {messages.length > 0 && contextPlant && !loading && (
              <div className="chat-welcome-suggestions" style={{ marginTop: '0.5rem', alignSelf: 'center', width: '85%' }}>
                {!usedQuickReplies.includes("weather") && (
                  <button 
                    className="chat-suggestion-btn"
                    onClick={() => {
                      sendMessage(`Can ${contextPlant} grow well in my place's weather?`);
                      setUsedQuickReplies(prev => [...prev, "weather"]);
                    }}
                  >
                    Can it grow in my weather?
                  </button>
                )}
                {!usedQuickReplies.includes("pests") && (
                  <button 
                    className="chat-suggestion-btn"
                    onClick={() => {
                      sendMessage(`What are common pests for ${contextPlant}?`);
                      setUsedQuickReplies(prev => [...prev, "pests"]);
                    }}
                  >
                    Common pests?
                  </button>
                )}
              </div>
            )}

            {loading && (
              <div className="chat-typing">
                <div className="chat-typing-dot"></div>
                <div className="chat-typing-dot"></div>
                <div className="chat-typing-dot"></div>
              </div>
            )}

            <div ref={messagesEndRef} />
          </div>

          {/* Input */}
          <div className="chat-input-area">
            <form className="chat-input-form" onSubmit={handleSubmit}>
              <input
                ref={inputRef}
                type="text"
                className="chat-input"
                placeholder="Ask about plant care..."
                value={input}
                onChange={e => setInput(e.target.value)}
                disabled={loading}
              />
              <button
                type="submit"
                className="chat-send-btn"
                disabled={!input.trim() || loading}
              >
                <FaPaperPlane />
              </button>
            </form>
          </div>
        </div>
      )}
    </>
  );
};

export default ChatWidget;
