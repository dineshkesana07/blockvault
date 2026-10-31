import React, { useState } from 'react';

export default function UnlockScreen({ onUnlockSuccess, onShowToast }) {
  const [masterPassword, setMasterPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [isDeriving, setIsDeriving] = useState(false);
  const [derivationStep, setDerivationStep] = useState('');

  const handleUnlock = (e) => {
    if (e) e.preventDefault();
    if (!masterPassword && !isDeriving) {
      setMasterPassword('master-password-demo');
    }

    setIsDeriving(true);
    setDerivationStep('Allocating 64MB memory block (Argon2id)...');

    setTimeout(() => {
      setDerivationStep('Computing 4 memory-hard iterations...');
      setTimeout(() => {
        setDerivationStep('Decrypting local enclave keystore (AES-256-GCM)...');
        setTimeout(() => {
          setIsDeriving(false);
          onUnlockSuccess();
          onShowToast('Vault enclave unlocked. Cryptographic keystore active in secure memory.', 'success');
        }, 500);
      }, 500);
    }, 500);
  };

  const handleBiometricUnlock = () => {
    setIsDeriving(true);
    setDerivationStep('Prompting FIDO2 / WebAuthn Hardware Authenticator...');

    setTimeout(() => {
      setDerivationStep('Hardware token verified (Ed25519 signature valid)...');
      setTimeout(() => {
        setIsDeriving(false);
        onUnlockSuccess();
        onShowToast('Biometric FIDO2 authentication verified.', 'success');
      }, 600);
    }, 700);
  };

  return (
    <div className="w-full min-h-screen bg-background text-on-surface antialiased flex flex-col items-center justify-center p-4 relative overflow-hidden select-none font-sans">
      {/* Background Cyber Mesh */}
      <div className="absolute inset-0 pointer-events-none opacity-20">
        <svg className="w-full h-full text-secondary" xmlns="http://www.w3.org/2000/svg">
          <defs>
            <pattern height="48" id="crypto-grid" patternUnits="userSpaceOnUse" width="48">
              <path d="M 48 0 L 0 0 0 48" fill="none" stroke="currentColor" strokeOpacity="0.25" strokeWidth="0.5"></path>
              <circle cx="48" cy="48" fill="currentColor" fillOpacity="0.3" r="1.5"></circle>
            </pattern>
          </defs>
          <rect fill="url(#crypto-grid)" height="100%" width="100%"></rect>
        </svg>
      </div>
      <div className="absolute -top-32 left-1/2 -translate-x-1/2 w-96 h-96 bg-primary/10 rounded-full blur-3xl pointer-events-none"></div>
      <div className="absolute bottom-4 left-1/4 w-80 h-80 bg-secondary/5 rounded-full blur-3xl pointer-events-none"></div>

      {/* High-Level Node Status Bar */}
      <div className="w-full max-w-xl flex items-center justify-between mb-4 px-2 z-10 text-[11px] font-mono-key text-on-surface-variant">
        <div className="flex items-center gap-2">
          <span className="w-1.5 h-1.5 rounded-full bg-secondary animate-ping"></span>
          <span>NODE: SV-EUS-0914</span>
          <span className="text-outline-variant">/</span>
          <span>IP: 198.51.100.24 (MTLS)</span>
        </div>
        <div className="flex items-center gap-2">
          <span>CIPHER: AES-256-GCM</span>
          <span className="px-1.5 py-0.5 rounded bg-surface-container-high text-secondary border border-secondary/20">
            HSM L3 ACTIVE
          </span>
        </div>
      </div>

      {/* Main Unlock Card */}
      <div className="w-full max-w-xl bg-surface-container-low rounded-xl shadow-2xl relative z-10 p-6 sm:p-8 border border-cyber-border-highlight">
        {/* Corner Neon Accents */}
        <div className="absolute top-0 left-0 w-3 h-3 bg-secondary rounded-tl-xl opacity-60"></div>
        <div className="absolute top-0 right-0 w-3 h-3 bg-primary rounded-tr-xl opacity-60"></div>
        <div className="absolute bottom-0 left-0 w-3 h-3 bg-primary rounded-bl-xl opacity-60"></div>
        <div className="absolute bottom-0 right-0 w-3 h-3 bg-secondary rounded-br-xl opacity-60"></div>

        {/* Card Header */}
        <div className="flex items-center justify-between mb-6 pb-4 border-b border-cyber-border/40">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-lg bg-surface-container-lowest flex items-center justify-center p-1 border border-primary/40 shadow-inner">
              <span className="material-symbols-outlined text-primary text-2xl">enhanced_encryption</span>
            </div>
            <div className="flex flex-col text-left">
              <div className="flex items-center gap-2">
                <span className="font-headline-sm text-base font-bold text-on-surface">BlockVault</span>
                <span className="font-badge-label text-[10px] text-secondary bg-surface-container px-1.5 py-0.5 rounded border border-secondary/20">
                  Z-K KEYSPACE
                </span>
              </div>
              <span className="font-mono-key text-[10px] text-on-surface-variant">
                Personal Cryptographic Vault
              </span>
            </div>
          </div>
          <div className="flex items-center gap-1.5 px-2.5 py-1 rounded bg-error/10 text-error font-mono-key text-[11px] border border-error/20">
            <span className="w-2 h-2 rounded-full bg-error animate-pulse"></span>
            <span>LOCKED (IDLE TIMEOUT)</span>
          </div>
        </div>

        {/* User Identity Profile Container */}
        <div className="bg-surface-container rounded-lg p-3.5 flex items-center justify-between mb-6 border border-cyber-border/40">
          <div className="flex items-center gap-3">
            <div className="relative">
              <div className="w-11 h-11 rounded-lg bg-primary/20 border border-primary/40 flex items-center justify-center text-primary font-bold text-sm">
                EV
              </div>
              <div className="absolute -bottom-1 -right-1 w-3.5 h-3.5 rounded-full bg-surface-container flex items-center justify-center">
                <span className="w-2 h-2 rounded-full bg-secondary"></span>
              </div>
            </div>
            <div className="flex flex-col text-left">
              <div className="flex items-center gap-1.5">
                <span className="font-body-md text-sm font-semibold text-on-surface">
                  Dr. Elena Vance
                </span>
                <span className="material-symbols-outlined text-secondary text-sm">
                  verified_user
                </span>
              </div>
              <span className="font-mono-key text-[10px] text-on-surface-variant">
                elena.vance@sovereign-sec.io
              </span>
            </div>
          </div>
          <div className="flex flex-col items-end">
            <span className="font-mono-key text-[10px] text-on-surface-variant uppercase">
              Vault Identity
            </span>
            <span className="font-mono-data text-xs text-secondary font-mono-numbers">
              0x89F4...D12A
            </span>
          </div>
        </div>

        {/* Unlock Form */}
        <form onSubmit={handleUnlock} className="flex flex-col gap-4 text-left">
          <div className="flex flex-col gap-1.5">
            <div className="flex items-center justify-between text-xs">
              <label className="font-semibold text-on-surface flex items-center gap-1">
                <span>Enter Master Password</span>
                <span className="material-symbols-outlined text-xs text-outline cursor-pointer hover:text-secondary" title="Derived client-side via Argon2id">
                  info
                </span>
              </label>
              <button
                type="button"
                onClick={() => setMasterPassword('master-password-demo')}
                className="font-mono-key text-[10px] text-secondary hover:underline cursor-pointer"
              >
                Insert Demo Key
              </button>
            </div>
            <div className="relative">
              <input
                type={showPassword ? 'text' : 'password'}
                value={masterPassword}
                onChange={e => setMasterPassword(e.target.value)}
                placeholder="••••••••••••••••••••••••"
                className="w-full pl-3 pr-10 py-2.5 bg-surface-container rounded-lg border border-cyber-border text-on-surface font-mono-data text-sm focus:outline-none focus:border-primary shadow-inner"
              />
              <button
                type="button"
                onClick={() => setShowPassword(!showPassword)}
                className="absolute right-3 top-1/2 -translate-y-1/2 text-on-surface-variant hover:text-on-surface cursor-pointer"
              >
                <span className="material-symbols-outlined text-base">
                  {showPassword ? 'visibility_off' : 'visibility'}
                </span>
              </button>
            </div>
          </div>

          {/* Derivation Progress if active */}
          {isDeriving && (
            <div className="p-3 rounded-lg bg-surface-container-lowest border border-secondary/30 flex items-center gap-2.5 font-mono-data text-xs text-secondary animate-pulse">
              <span className="material-symbols-outlined text-base animate-spin">refresh</span>
              <span>{derivationStep}</span>
            </div>
          )}

          {/* Unlock Submit Buttons */}
          <div className="flex flex-col sm:flex-row gap-3 pt-2">
            <button
              type="submit"
              disabled={isDeriving}
              className="flex-1 flex items-center justify-center gap-2 bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs py-3 rounded-lg shadow-[0_0_16px_rgba(124,92,255,0.4)] transition-all cursor-pointer disabled:opacity-50"
            >
              <span className="material-symbols-outlined text-sm">lock_open</span>
              <span>Unlock Local Enclave</span>
            </button>
            <button
              type="button"
              disabled={isDeriving}
              onClick={handleBiometricUnlock}
              className="flex items-center justify-center gap-2 bg-surface-container hover:bg-surface-container-high text-on-surface text-xs py-3 px-4 rounded-lg border border-cyber-border transition-all cursor-pointer"
            >
              <span className="material-symbols-outlined text-secondary text-sm">fingerprint</span>
              <span>FIDO2 Passkey</span>
            </button>
          </div>
        </form>

        {/* Footer Emergency Recovery */}
        <div className="flex items-center justify-between pt-6 mt-6 border-t border-cyber-border/40 text-[11px] font-mono-key text-on-surface-variant">
          <span className="hover:text-secondary cursor-pointer">
            Lost Master Password? Emergency Kit (24 Words)
          </span>
          <span className="text-outline">Argon2id v1.3</span>
        </div>
      </div>
    </div>
  );
}
