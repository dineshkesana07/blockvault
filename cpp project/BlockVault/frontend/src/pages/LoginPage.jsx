import React, { useState } from 'react';

export default function LoginPage({ onLoginSuccess, onNavigateToRegister, onShowToast }) {
  const [email, setEmail] = useState('elena.vance@sovereign-sec.io');
  const [password, setPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [isDeriving, setIsDeriving] = useState(false);
  const [derivationStage, setDerivationStage] = useState('');
  const [showRecoveryModal, setShowRecoveryModal] = useState(false);
  const [recoveryInput, setRecoveryInput] = useState('');

  const handlePasswordLogin = (e) => {
    if (e) e.preventDefault();
    if (!password) {
      setPassword('master-password-demo');
    }

    setIsDeriving(true);
    setDerivationStage('Allocating 64MB memory arena (Argon2id)...');

    setTimeout(() => {
      setDerivationStage('Executing 4 memory-hard key derivation iterations...');
      setTimeout(() => {
        setDerivationStage('Decrypting local AES-256-GCM enclave keystore...');
        setTimeout(() => {
          setIsDeriving(false);
          onLoginSuccess({ email });
          onShowToast('Vault enclave unlocked. Cryptographic keystore active.', 'success');
        }, 500);
      }, 500);
    }, 500);
  };

  const handleFidoLogin = () => {
    setIsDeriving(true);
    setDerivationStage('Prompting FIDO2 / WebAuthn Hardware Authenticator...');

    setTimeout(() => {
      setDerivationStage('Hardware Ed25519 signature validated...');
      setTimeout(() => {
        setIsDeriving(false);
        onLoginSuccess({ email });
        onShowToast('FIDO2 hardware passkey verified. Vault unlocked.', 'success');
      }, 500);
    }, 700);
  };

  const handleRecoverySubmit = (e) => {
    e.preventDefault();
    const words = recoveryInput.trim().split(/\s+/);
    if (words.length < 12) {
      onShowToast('Emergency recovery phrase requires at least 12 valid cryptographic words.', 'warning');
      return;
    }
    setShowRecoveryModal(false);
    onLoginSuccess({ email: 'recovered-account@blockvault.local' });
    onShowToast('Emergency recovery verified. Master passphrase reset required.', 'success');
  };

  return (
    <div className="w-full min-h-screen bg-background text-on-surface antialiased flex flex-col items-center justify-center p-4 relative overflow-hidden font-sans select-none">
      {/* Background Ambience */}
      <div className="absolute inset-0 pointer-events-none opacity-20">
        <svg className="w-full h-full text-secondary" xmlns="http://www.w3.org/2000/svg">
          <defs>
            <pattern height="48" id="crypto-grid" patternUnits="userSpaceOnUse" width="48">
              <path d="M 48 0 L 0 0 0 48" fill="none" stroke="currentColor" strokeOpacity="0.25" strokeWidth="0.5" />
              <circle cx="48" cy="48" fill="currentColor" fillOpacity="0.3" r="1.5" />
            </pattern>
          </defs>
          <rect fill="url(#crypto-grid)" height="100%" width="100%" />
        </svg>
      </div>
      <div className="absolute -top-32 left-1/2 -translate-x-1/2 w-96 h-96 bg-primary/10 rounded-full blur-3xl pointer-events-none" />
      <div className="absolute bottom-4 left-1/4 w-80 h-80 bg-secondary/5 rounded-full blur-3xl pointer-events-none" />

      {/* Header Telemetry */}
      <div className="w-full max-w-md flex items-center justify-between mb-4 z-10 text-[11px] font-mono-key text-on-surface-variant">
        <div className="flex items-center gap-2">
          <span className="w-1.5 h-1.5 rounded-full bg-secondary animate-ping" />
          <span>NODE: SV-EUS-0914</span>
          <span className="text-outline">/</span>
          <span>MTLS PINNED</span>
        </div>
        <span className="px-1.5 py-0.5 rounded bg-surface-container text-secondary border border-secondary/20">
          HSM L3 ACTIVE
        </span>
      </div>

      {/* Main Login Card */}
      <div className="w-full max-w-md bg-surface-container-low rounded-xl shadow-2xl relative z-10 p-6 sm:p-8 border border-cyber-border-highlight text-left">
        {/* Card Header */}
        <div className="flex items-center justify-between pb-4 mb-5 border-b border-cyber-border/50">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-lg bg-surface-container flex items-center justify-center text-primary border border-primary/30 shadow-inner">
              <span className="material-symbols-outlined text-2xl">enhanced_encryption</span>
            </div>
            <div>
              <h1 className="font-headline-sm text-lg font-bold text-on-surface">
                Sign In to BlockVault
              </h1>
              <span className="font-mono-key text-[10px] text-secondary tracking-widest uppercase">
                Zero-Knowledge Authentication
              </span>
            </div>
          </div>
          <span className="font-mono-key text-[10px] text-secondary bg-surface-container px-2 py-0.5 rounded border border-secondary/20">
            v3.4-ZKP
          </span>
        </div>

        {/* Derivation Progress State */}
        {isDeriving ? (
          <div className="py-8 flex flex-col items-center justify-center text-center space-y-4">
            <div className="w-12 h-12 rounded-full border-2 border-secondary/30 border-t-secondary animate-spin" />
            <div className="space-y-1">
              <span className="font-mono-key text-xs text-secondary font-semibold uppercase tracking-wider block">
                Executing Cryptographic Derivation
              </span>
              <p className="font-mono-data text-[11px] text-on-surface-variant max-w-xs">
                {derivationStage}
              </p>
            </div>
          </div>
        ) : (
          <form onSubmit={handlePasswordLogin} className="space-y-4">
            {/* Account Identifier */}
            <div className="flex flex-col gap-1.5 text-xs">
              <label className="text-on-surface font-medium">Account Identifier / Email</label>
              <input
                type="email"
                required
                value={email}
                onChange={e => setEmail(e.target.value)}
                placeholder="name@domain.com"
                className="w-full px-3.5 py-2.5 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-sm focus:outline-none focus:border-primary font-mono-data"
              />
            </div>

            {/* Master Passphrase */}
            <div className="flex flex-col gap-1.5 text-xs">
              <div className="flex items-center justify-between">
                <label className="text-on-surface font-medium">Master Passphrase</label>
                <button
                  type="button"
                  onClick={() => setShowRecoveryModal(true)}
                  className="text-[11px] text-secondary hover:underline cursor-pointer font-mono-key"
                >
                  Emergency Recovery?
                </button>
              </div>
              <div className="relative">
                <input
                  type={showPassword ? 'text' : 'password'}
                  value={password}
                  onChange={e => setPassword(e.target.value)}
                  placeholder="Enter your master passphrase..."
                  className="w-full pl-3.5 pr-10 py-2.5 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-sm focus:outline-none focus:border-primary font-mono-data tracking-wider"
                />
                <button
                  type="button"
                  onClick={() => setShowPassword(!showPassword)}
                  className="absolute right-2.5 top-1/2 -translate-y-1/2 text-on-surface-variant hover:text-on-surface cursor-pointer p-1"
                >
                  <span className="material-symbols-outlined text-base">
                    {showPassword ? 'visibility_off' : 'visibility'}
                  </span>
                </button>
              </div>
            </div>

            {/* Submit Passphrase Button */}
            <button
              type="submit"
              className="w-full py-2.5 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs shadow-[0_0_16px_rgba(124,92,255,0.4)] transition-all cursor-pointer flex items-center justify-center gap-2 mt-2"
            >
              <span className="material-symbols-outlined text-sm">lock_open</span>
              <span>Unlock with Argon2id</span>
            </button>

            {/* Divider */}
            <div className="relative my-4 flex items-center justify-center">
              <div className="border-t border-cyber-border/60 w-full" />
              <span className="bg-surface-container-low px-2 font-mono-key text-[10px] text-on-surface-variant uppercase tracking-wider relative">
                Or Authenticate Via
              </span>
            </div>

            {/* FIDO2 Biometric Button */}
            <button
              type="button"
              onClick={handleFidoLogin}
              className="w-full py-2.5 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface text-xs border border-cyber-border transition-colors cursor-pointer flex items-center justify-center gap-2"
            >
              <span className="material-symbols-outlined text-secondary text-base">fingerprint</span>
              <span>Touch FIDO2 Hardware Passkey</span>
            </button>

            {/* Register Link */}
            <div className="pt-3 text-center border-t border-cyber-border/40">
              <button
                type="button"
                onClick={onNavigateToRegister}
                className="text-xs text-on-surface-variant hover:text-secondary cursor-pointer font-mono-key"
              >
                Need to create a new vault? <span className="text-secondary font-semibold">Initialize Vault →</span>
              </button>
            </div>
          </form>
        )}
      </div>

      {/* Emergency Recovery Modal */}
      {showRecoveryModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/80 backdrop-blur-md animate-fadeIn">
          <div className="w-full max-w-md bg-surface-container-low border border-cyber-border-highlight rounded-xl shadow-2xl p-6 text-left">
            <div className="flex items-center justify-between pb-3 mb-4 border-b border-cyber-border/60">
              <div className="flex items-center gap-2">
                <span className="material-symbols-outlined text-amber-400 text-xl">key_vertical</span>
                <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
                  Emergency Recovery Phrase
                </h3>
              </div>
              <button
                onClick={() => setShowRecoveryModal(false)}
                className="text-on-surface-variant hover:text-on-surface cursor-pointer"
              >
                ✕
              </button>
            </div>

            <form onSubmit={handleRecoverySubmit} className="space-y-4 text-xs font-sans">
              <p className="text-on-surface-variant">
                Enter your 24-word emergency recovery phrase separated by spaces to re-derive your zero-knowledge keyspace.
              </p>

              <textarea
                rows={4}
                required
                value={recoveryInput}
                onChange={e => setRecoveryInput(e.target.value)}
                placeholder="obsidian quantum argon enclave shannon entropy sovereign glacier..."
                className="w-full p-3 bg-surface-container rounded-lg border border-cyber-border text-on-surface font-mono-data text-xs focus:outline-none focus:border-secondary"
              />

              <div className="flex items-center justify-end gap-2.5 pt-2">
                <button
                  type="button"
                  onClick={() => setShowRecoveryModal(false)}
                  className="px-3.5 py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-xs text-on-surface border border-cyber-border"
                >
                  Cancel
                </button>
                <button
                  type="submit"
                  className="px-4 py-2 rounded-lg bg-secondary text-on-secondary font-bold text-xs shadow-md"
                >
                  Verify & Recover Vault
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}

