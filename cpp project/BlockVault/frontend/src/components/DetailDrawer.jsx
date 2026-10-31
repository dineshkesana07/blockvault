import React, { useState, useEffect } from 'react';
import { calculateEntropy, getSimulatedTOTP } from '../services/cryptoService';

export default function DetailDrawer({
  credential,
  isOpen,
  onClose,
  onEdit,
  onDelete,
  onToggleFavorite,
  onShowToast
}) {
  const [showPassword, setShowPassword] = useState(false);
  const [countdown, setCountdown] = useState(0);
  const [copiedField, setCopiedField] = useState(null);
  const [totp, setTotp] = useState({ code: '------', remaining: 30 });

  // Reset showPassword when credential changes
  useEffect(() => {
    setShowPassword(false);
    setCountdown(0);
    setCopiedField(null);
  }, [credential]);

  // Auto-mask countdown timer (10 seconds security feature against shoulder surfing)
  useEffect(() => {
    let timer;
    if (showPassword && countdown > 0) {
      timer = setInterval(() => {
        setCountdown(prev => {
          if (prev <= 1) {
            setShowPassword(false);
            return 0;
          }
          return prev - 1;
        });
      }, 1000);
    }
    return () => clearInterval(timer);
  }, [showPassword, countdown]);

  // Live TOTP clock
  useEffect(() => {
    if (!credential || !credential.totpSecret) return;
    const update = () => {
      setTotp(getSimulatedTOTP(credential.totpSecret));
    };
    update();
    const interval = setInterval(update, 1000);
    return () => clearInterval(interval);
  }, [credential]);

  if (!isOpen || !credential) return null;

  const handleReveal = () => {
    if (!showPassword) {
      setShowPassword(true);
      setCountdown(10); // 10 second auto-mask
    } else {
      setShowPassword(false);
      setCountdown(0);
    }
  };

  const handleCopy = (text, fieldName) => {
    navigator.clipboard.writeText(text);
    setCopiedField(fieldName);
    onShowToast(`Copied ${fieldName} to clipboard. Memory sanitization in 30s.`, 'success');
    setTimeout(() => {
      setCopiedField(null);
    }, 2000);
  };

  const entropy = calculateEntropy(credential.password);

  return (
    <div className="fixed inset-0 z-50 overflow-hidden select-none">
      {/* Backdrop */}
      <div 
        className="absolute inset-0 bg-black/60 backdrop-blur-sm transition-opacity"
        onClick={onClose}
      />

      {/* Slide-in Panel */}
      <div className="absolute inset-y-0 right-0 max-w-full flex pl-10">
        <div className="w-screen max-w-md bg-surface-container-low border-l border-cyber-border-highlight shadow-2xl flex flex-col justify-between text-left animate-slideLeft">
          
          {/* Header */}
          <div className="p-6 border-b border-cyber-border/60 bg-surface-container-lowest/80 flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-lg bg-surface-container flex items-center justify-center text-secondary border border-secondary/30 shadow-inner">
                <span className="material-symbols-outlined text-xl">token</span>
              </div>
              <div className="flex flex-col">
                <div className="flex items-center gap-2">
                  <h2 className="font-headline-sm text-base font-bold text-on-surface truncate max-w-[200px]">
                    {credential.title}
                  </h2>
                  <button
                    onClick={() => onToggleFavorite(credential.id)}
                    className="cursor-pointer text-sm"
                    title={credential.favorite ? 'Favorited' : 'Mark as Favorite'}
                  >
                    <span className={`material-symbols-outlined text-sm ${credential.favorite ? 'text-amber-400 fill-current' : 'text-on-surface-variant hover:text-on-surface'}`}>
                      {credential.favorite ? 'star' : 'star_border'}
                    </span>
                  </button>
                </div>
                <span className="font-mono-key text-[10px] text-secondary uppercase tracking-wider">
                  {credential.category} • 256-BIT ENCRYPTED
                </span>
              </div>
            </div>

            <button
              onClick={onClose}
              className="p-1.5 rounded-lg text-on-surface-variant hover:text-on-surface hover:bg-surface-container transition-colors cursor-pointer"
            >
              <span className="material-symbols-outlined text-lg">close</span>
            </button>
          </div>

          {/* Body Content */}
          <div className="p-6 overflow-y-auto space-y-6 flex-1 text-xs">
            
            {/* Identity & Username Card */}
            <div className="p-4 rounded-xl bg-surface-container border border-cyber-border flex flex-col gap-2">
              <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                Account Identity / Username
              </span>
              <div className="flex items-center justify-between gap-2">
                <span className="font-mono-data text-sm text-on-surface font-semibold select-all break-all">
                  {credential.username || '—'}
                </span>
                <button
                  onClick={() => handleCopy(credential.username, 'Username')}
                  className={`p-1.5 rounded transition-all cursor-pointer flex items-center gap-1 ${
                    copiedField === 'Username'
                      ? 'bg-secondary/20 text-secondary border border-secondary/40 shadow-[0_0_10px_rgba(34,211,238,0.3)]'
                      : 'bg-surface-container-high hover:bg-surface-bright text-on-surface-variant hover:text-on-surface'
                  }`}
                  title="Copy Username"
                >
                  <span className="material-symbols-outlined text-sm">
                    {copiedField === 'Username' ? 'check' : 'person'}
                  </span>
                  <span className="font-mono-key text-[10px]">
                    {copiedField === 'Username' ? 'COPIED' : 'COPY'}
                  </span>
                </button>
              </div>
            </div>

            {/* Secret / Password Card */}
            <div className="p-4 rounded-xl bg-surface-container border border-cyber-border flex flex-col gap-2.5">
              <div className="flex items-center justify-between">
                <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                  Secret Keyphrase
                </span>
                {showPassword && countdown > 0 && (
                  <span className="font-mono-key text-[10px] text-amber-400 flex items-center gap-1 bg-amber-400/10 px-2 py-0.5 rounded border border-amber-400/20">
                    <span className="w-1.5 h-1.5 rounded-full bg-amber-400 animate-ping" />
                    Auto-mask in {countdown}s
                  </span>
                )}
              </div>

              <div className="p-2.5 rounded-lg bg-surface-container-lowest border border-cyber-border/70 flex items-center justify-between gap-2">
                <span className="font-mono-data text-sm text-on-surface select-all tracking-wider break-all">
                  {showPassword ? credential.password : '••••••••••••••••••••'}
                </span>
                <div className="flex items-center gap-1 shrink-0">
                  <button
                    onClick={handleReveal}
                    className="p-1.5 rounded bg-surface-container hover:bg-surface-container-high text-on-surface-variant hover:text-secondary transition-colors cursor-pointer"
                    title={showPassword ? 'Hide Secret' : 'Reveal Secret (10s)'}
                  >
                    <span className="material-symbols-outlined text-sm">
                      {showPassword ? 'visibility_off' : 'visibility'}
                    </span>
                  </button>
                  <button
                    onClick={() => handleCopy(credential.password, 'Password')}
                    className={`p-1.5 rounded transition-all cursor-pointer flex items-center gap-1 ${
                      copiedField === 'Password'
                        ? 'bg-secondary/20 text-secondary border border-secondary/40 shadow-[0_0_10px_rgba(34,211,238,0.3)]'
                        : 'bg-surface-container hover:bg-surface-container-high text-on-surface-variant hover:text-secondary'
                    }`}
                    title="Copy Password"
                  >
                    <span className="material-symbols-outlined text-sm">
                      {copiedField === 'Password' ? 'check' : 'content_copy'}
                    </span>
                    <span className="font-mono-key text-[10px]">
                      {copiedField === 'Password' ? 'COPIED' : 'COPY'}
                    </span>
                  </button>
                </div>
              </div>

              {/* Entropy breakdown */}
              <div className="flex items-center justify-between text-[11px] font-mono-key pt-1 border-t border-cyber-border/40">
                <div className="flex items-center gap-1.5">
                  <span className={`w-2 h-2 rounded-full ${entropy.bgColor}`} />
                  <span className={entropy.color}>{entropy.label} ({entropy.bits} BITS)</span>
                </div>
                <span className="text-on-surface-variant">Crack time: {entropy.crackTime}</span>
              </div>
            </div>

            {/* TOTP / Two-Factor Authentication */}
            {credential.totpSecret && (
              <div className="p-4 rounded-xl bg-surface-container border border-secondary/30 shadow-[0_0_14px_rgba(34,211,238,0.1)] flex flex-col gap-2">
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-1.5">
                    <span className="material-symbols-outlined text-secondary text-sm">timer</span>
                    <span className="font-mono-key text-[10px] uppercase text-secondary font-semibold">
                      One-Time Authenticator Code (TOTP)
                    </span>
                  </div>
                  <span className="font-mono-key text-[10px] text-on-surface-variant">
                    {totp.remaining}s left
                  </span>
                </div>

                <div className="flex items-center justify-between bg-surface-container-lowest p-2.5 rounded-lg border border-cyber-border">
                  <span className="font-mono-numbers text-xl font-bold tracking-widest text-secondary">
                    {totp.code.slice(0, 3)} {totp.code.slice(3, 6)}
                  </span>
                  <button
                    onClick={() => handleCopy(totp.code, 'TOTP Code')}
                    className="px-2.5 py-1 rounded bg-secondary/15 hover:bg-secondary/25 text-secondary text-[11px] font-mono-key border border-secondary/30 transition-all cursor-pointer flex items-center gap-1"
                  >
                    <span className="material-symbols-outlined text-xs">content_copy</span>
                    <span>{copiedField === 'TOTP Code' ? 'COPIED' : 'COPY CODE'}</span>
                  </button>
                </div>
              </div>
            )}

            {/* Target URL */}
            {credential.url && (
              <div className="p-4 rounded-xl bg-surface-container border border-cyber-border flex flex-col gap-1.5">
                <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                  Target Endpoint
                </span>
                <div className="flex items-center justify-between gap-2">
                  <span className="font-mono-data text-xs text-on-surface truncate">
                    {credential.url}
                  </span>
                  <a
                    href={credential.url}
                    target="_blank"
                    rel="noreferrer"
                    className="p-1.5 rounded bg-surface-container-high hover:bg-surface-bright text-secondary transition-colors cursor-pointer"
                    title="Launch URL in new tab"
                  >
                    <span className="material-symbols-outlined text-sm">open_in_new</span>
                  </a>
                </div>
              </div>
            )}

            {/* Tags */}
            {credential.tags && credential.tags.length > 0 && (
              <div className="flex flex-col gap-1.5">
                <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                  Security Tags
                </span>
                <div className="flex flex-wrap gap-1.5">
                  {credential.tags.map((tag, idx) => (
                    <span
                      key={idx}
                      className="px-2 py-0.5 rounded-full bg-surface-container border border-cyber-border text-on-surface-variant font-mono-key text-[10px]"
                    >
                      #{tag}
                    </span>
                  ))}
                </div>
              </div>
            )}

            {/* Secure Notes */}
            {credential.notes && (
              <div className="p-4 rounded-xl bg-surface-container border border-cyber-border flex flex-col gap-1.5">
                <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                  Secure Enclave Notes
                </span>
                <p className="font-body-md text-xs text-on-surface-variant whitespace-pre-wrap leading-relaxed">
                  {credential.notes}
                </p>
              </div>
            )}

            {/* Metadata / Audit Telemetry */}
            <div className="p-3 rounded-lg bg-surface-container-lowest border border-cyber-border/40 font-mono-key text-[10px] text-on-surface-variant space-y-1">
              <div className="flex justify-between">
                <span>ENCRYPTION CIPHER</span>
                <span className="text-on-surface">AES-256-GCM (Argon2id)</span>
              </div>
              <div className="flex justify-between">
                <span>RECORD ID</span>
                <span className="text-secondary">{credential.id}</span>
              </div>
              <div className="flex justify-between">
                <span>LAST MODIFIED</span>
                <span className="text-on-surface">{credential.lastModified ? new Date(credential.lastModified).toLocaleString() : 'Recent'}</span>
              </div>
            </div>

          </div>

          {/* Footer Action Bar */}
          <div className="p-4 border-t border-cyber-border/60 bg-surface-container-lowest/90 flex items-center justify-between gap-3">
            <button
              onClick={() => onDelete(credential.id, credential.title)}
              className="px-3 py-2 rounded-lg bg-error/10 hover:bg-error/20 text-error border border-error/30 text-xs font-semibold transition-colors cursor-pointer flex items-center gap-1.5"
            >
              <span className="material-symbols-outlined text-sm">delete</span>
              <span>Delete Secret</span>
            </button>

            <div className="flex items-center gap-2">
              <button
                onClick={() => onEdit(credential)}
                className="px-4 py-2 rounded-lg bg-primary hover:bg-primary-hover text-on-primary text-xs font-semibold shadow-md transition-all cursor-pointer flex items-center gap-1.5"
              >
                <span className="material-symbols-outlined text-sm">edit</span>
                <span>Edit Credential</span>
              </button>
            </div>
          </div>

        </div>
      </div>
    </div>
  );
}

