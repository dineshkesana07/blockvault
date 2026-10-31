import React, { useState } from 'react';
import { calculateEntropy, generatePassword } from '../services/cryptoService';

export default function RegisterPage({ onRegisterSuccess, onNavigateToLogin, onShowToast }) {
  const [step, setStep] = useState(1); // 1: Info, 2: Master Key, 3: Recovery Phrase
  const [email, setEmail] = useState('');
  const [vaultName, setVaultName] = useState('Personal Enclave');
  const [masterPassword, setMasterPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [hasBackedUp, setHasBackedUp] = useState(false);
  const [copiedPhrase, setCopiedPhrase] = useState(false);

  const [recoveryPhrase] = useState(() => [
    'obsidian', 'quantum', 'argon', 'enclave', 'shannon', 'entropy',
    'sovereign', 'glacier', 'shield', 'circuit', 'vertex', 'poly1305',
    'orbital', 'strata', 'plasma', 'cascade', 'monolith', 'matrix',
    'sentinel', 'phantom', 'beacon', 'spectrum', 'zenith', 'pulse'
  ]);

  const entropy = calculateEntropy(masterPassword);

  const handleStep1Submit = (e) => {
    e.preventDefault();
    if (!email.trim()) {
      onShowToast('Please provide an account identifier or email.', 'warning');
      return;
    }
    setStep(2);
  };

  const handleStep2Submit = (e) => {
    e.preventDefault();
    if (!masterPassword) {
      onShowToast('Master passphrase cannot be empty.', 'warning');
      return;
    }
    if (masterPassword !== confirmPassword) {
      onShowToast('Passphrases do not match. Please re-enter.', 'warning');
      return;
    }
    if (entropy.bits < 50) {
      onShowToast('Master passphrase entropy is too low. Aim for at least 60 bits.', 'warning');
      return;
    }
    setStep(3);
  };

  const handleCopyPhrase = () => {
    navigator.clipboard.writeText(recoveryPhrase.join(' '));
    setCopiedPhrase(true);
    onShowToast('24-word recovery phrase copied to clipboard.', 'success');
    setTimeout(() => setCopiedPhrase(false), 2500);
  };

  const handleDownloadKit = () => {
    const content = `BLOCKVAULT ZERO-KNOWLEDGE RECOVERY KIT
===============================================
Vault Name: ${vaultName}
Account Identifier: ${email}
Created: ${new Date().toISOString()}

CRITICAL WARNING:
This is your unrecoverable emergency cryptographic recovery phrase.
BlockVault servers hold ZERO knowledge of your keys or passwords.
If you lose your master passphrase and recovery phrase, your data CANNOT be recovered.

24-WORD EMERGENCY RECOVERY PHRASE:
${recoveryPhrase.map((w, i) => `${i + 1}. ${w}`).join('\n')}

Cryptographic Derivation: Argon2id (m=64MB, t=4, p=1) + AES-256-GCM
===============================================`;

    const blob = new Blob([content], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `blockvault-recovery-kit-${Date.now()}.txt`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    onShowToast('Emergency Recovery Kit (.txt) downloaded.', 'success');
  };

  const handleCompleteRegistration = () => {
    if (!hasBackedUp) {
      onShowToast('Please confirm you have safely stored your recovery phrase.', 'warning');
      return;
    }
    onRegisterSuccess({
      email,
      vaultName
    });
    onShowToast('Zero-knowledge vault initialized! Enclave active.', 'success');
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

      {/* Brand Header */}
      <div className="w-full max-w-lg flex items-center justify-between mb-4 z-10 text-[11px] font-mono-key text-on-surface-variant">
        <div className="flex items-center gap-2">
          <div className="w-6 h-6 rounded bg-surface-container flex items-center justify-center text-primary border border-primary/30">
            <span className="material-symbols-outlined text-sm">enhanced_encryption</span>
          </div>
          <span className="text-on-surface font-bold uppercase tracking-wider">BlockVault Setup</span>
        </div>
        <div className="flex items-center gap-2">
          <span className="text-secondary">STEP {step} OF 3</span>
          <span className="text-outline">/</span>
          <span>ZERO-KNOWLEDGE INIT</span>
        </div>
      </div>

      {/* Main Registration Card */}
      <div className="w-full max-w-lg bg-surface-container-low rounded-xl shadow-2xl relative z-10 p-6 sm:p-8 border border-cyber-border-highlight text-left">
        {/* Step Indicator Bar */}
        <div className="w-full h-1 bg-surface-container-highest rounded-full overflow-hidden mb-6 flex">
          <div className={`h-full transition-all duration-300 ${step === 1 ? 'w-1/3 bg-primary' : step === 2 ? 'w-2/3 bg-primary' : 'w-full bg-secondary'}`} />
        </div>

        {/* STEP 1: ACCOUNT IDENTITY */}
        {step === 1 && (
          <form onSubmit={handleStep1Submit} className="space-y-5">
            <div>
              <h1 className="font-headline-sm text-xl font-bold text-on-surface">
                Create Your Sovereign Vault
              </h1>
              <p className="font-body-md text-xs text-on-surface-variant mt-1 leading-relaxed">
                Initialize an isolated client-side cryptographic keystore. No plaintexts or master passwords ever leave your machine.
              </p>
            </div>

            <div className="space-y-3">
              <div className="flex flex-col gap-1.5 text-xs">
                <label className="text-on-surface font-medium">Account Identifier / Email</label>
                <input
                  type="email"
                  required
                  value={email}
                  onChange={e => setEmail(e.target.value)}
                  placeholder="e.g. elena.vance@org.io"
                  className="w-full px-3.5 py-2.5 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-sm focus:outline-none focus:border-primary font-mono-data"
                />
              </div>

              <div className="flex flex-col gap-1.5 text-xs">
                <label className="text-on-surface font-medium">Vault Namespace</label>
                <input
                  type="text"
                  value={vaultName}
                  onChange={e => setVaultName(e.target.value)}
                  placeholder="Personal Enclave"
                  className="w-full px-3.5 py-2.5 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-sm focus:outline-none focus:border-primary"
                />
              </div>
            </div>

            <div className="p-3 rounded-lg bg-surface-container border border-secondary/20 flex items-start gap-2.5 text-xs text-on-surface-variant">
              <span className="material-symbols-outlined text-secondary text-base shrink-0 mt-0.5">verified_user</span>
              <span>
                Zero-knowledge architecture ensures even BlockVault engineers cannot view or restore your credentials.
              </span>
            </div>

            <div className="flex items-center justify-between pt-2">
              <button
                type="button"
                onClick={onNavigateToLogin}
                className="text-xs text-secondary hover:underline cursor-pointer font-mono-key"
              >
                Already registered? Sign In →
              </button>
              <button
                type="submit"
                className="px-5 py-2.5 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs shadow-md transition-all cursor-pointer flex items-center gap-1.5"
              >
                <span>Continue</span>
                <span className="material-symbols-outlined text-sm">arrow_forward</span>
              </button>
            </div>
          </form>
        )}

        {/* STEP 2: MASTER PASSPHRASE */}
        {step === 2 && (
          <form onSubmit={handleStep2Submit} className="space-y-5">
            <div>
              <h1 className="font-headline-sm text-xl font-bold text-on-surface">
                Establish Master Passphrase
              </h1>
              <p className="font-body-md text-xs text-on-surface-variant mt-1 leading-relaxed">
                This secret derives your 256-bit AES-GCM encryption key using memory-hard Argon2id parameters. Choose a unique, strong passphrase.
              </p>
            </div>

            <div className="space-y-4">
              <div className="flex flex-col gap-1.5 text-xs">
                <label className="text-on-surface font-medium">Master Passphrase</label>
                <div className="relative">
                  <input
                    type={showPassword ? 'text' : 'password'}
                    required
                    value={masterPassword}
                    onChange={e => setMasterPassword(e.target.value)}
                    placeholder="Enter high-entropy passphrase..."
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

                {/* Entropy Meter */}
                {masterPassword && (
                  <div className="space-y-1.5 pt-1">
                    <div className="w-full h-1.5 bg-surface-container-highest rounded-full overflow-hidden">
                      <div
                        className={`h-full ${entropy.bgColor} transition-all`}
                        style={{ width: `${Math.min(100, entropy.score)}%` }}
                      />
                    </div>
                    <div className="flex items-center justify-between text-[11px] font-mono-key">
                      <span className={entropy.color}>{entropy.label} ({entropy.bits} BITS)</span>
                      <span className="text-on-surface-variant">Crack time: {entropy.crackTime}</span>
                    </div>
                  </div>
                )}
              </div>

              <div className="flex flex-col gap-1.5 text-xs">
                <label className="text-on-surface font-medium">Confirm Master Passphrase</label>
                <input
                  type="password"
                  required
                  value={confirmPassword}
                  onChange={e => setConfirmPassword(e.target.value)}
                  placeholder="Re-enter master passphrase..."
                  className="w-full px-3.5 py-2.5 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-sm focus:outline-none focus:border-primary font-mono-data tracking-wider"
                />
              </div>
            </div>

            <div className="flex items-center justify-between pt-2">
              <button
                type="button"
                onClick={() => setStep(1)}
                className="text-xs text-on-surface-variant hover:text-on-surface cursor-pointer"
              >
                ← Back
              </button>
              <button
                type="submit"
                className="px-5 py-2.5 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs shadow-md transition-all cursor-pointer flex items-center gap-1.5"
              >
                <span>Generate Recovery Phrase</span>
                <span className="material-symbols-outlined text-sm">arrow_forward</span>
              </button>
            </div>
          </form>
        )}

        {/* STEP 3: 24-WORD RECOVERY PHRASE */}
        {step === 3 && (
          <div className="space-y-5">
            <div>
              <h1 className="font-headline-sm text-xl font-bold text-on-surface">
                Emergency 24-Word Recovery Phrase
              </h1>
              <p className="font-body-md text-xs text-on-surface-variant mt-1 leading-relaxed">
                Write down these 24 words in exact order. If you forget your master passphrase, this is the <strong className="text-amber-400">ONLY way</strong> to recover your credentials.
              </p>
            </div>

            {/* 24-Word Grid */}
            <div className="grid grid-cols-3 sm:grid-cols-4 gap-2 bg-surface-container p-3.5 rounded-xl border border-cyber-border font-mono-data text-[11px] max-h-48 overflow-y-auto">
              {recoveryPhrase.map((word, i) => (
                <div key={i} className="flex items-center gap-1.5 p-1 rounded bg-surface-container-lowest border border-cyber-border/40">
                  <span className="text-outline text-[10px] w-4 text-right">{i + 1}.</span>
                  <span className="text-secondary font-medium">{word}</span>
                </div>
              ))}
            </div>

            <div className="flex items-center gap-2">
              <button
                type="button"
                onClick={handleCopyPhrase}
                className={`flex-1 py-2 rounded-lg text-xs font-mono-key border transition-all cursor-pointer flex items-center justify-center gap-1.5 ${
                  copiedPhrase
                    ? 'bg-secondary/20 text-secondary border-secondary/40 shadow-[0_0_12px_rgba(34,211,238,0.3)]'
                    : 'bg-surface-container hover:bg-surface-container-high text-on-surface border-cyber-border'
                }`}
              >
                <span className="material-symbols-outlined text-sm">{copiedPhrase ? 'check' : 'content_copy'}</span>
                <span>{copiedPhrase ? 'COPIED 24 WORDS' : 'COPY RECOVERY PHRASE'}</span>
              </button>

              <button
                type="button"
                onClick={handleDownloadKit}
                className="py-2 px-3 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface border border-cyber-border text-xs font-mono-key transition-colors cursor-pointer flex items-center gap-1.5"
                title="Download Recovery Kit (.txt)"
              >
                <span className="material-symbols-outlined text-sm text-secondary">file_download</span>
                <span>KIT (.TXT)</span>
              </button>
            </div>

            <label className="flex items-start gap-2.5 p-3 rounded-lg bg-surface-container/60 border border-cyber-border/70 text-xs text-on-surface cursor-pointer">
              <input
                type="checkbox"
                checked={hasBackedUp}
                onChange={e => setHasBackedUp(e.target.checked)}
                className="accent-primary w-4 h-4 rounded mt-0.5 cursor-pointer"
              />
              <span>
                I have securely stored my 24-word recovery phrase offline. I understand that without it, lost credentials cannot be recovered.
              </span>
            </label>

            <div className="flex items-center justify-between pt-2">
              <button
                type="button"
                onClick={() => setStep(2)}
                className="text-xs text-on-surface-variant hover:text-on-surface cursor-pointer"
              >
                ← Back
              </button>
              <button
                type="button"
                onClick={handleCompleteRegistration}
                className="px-6 py-2.5 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-bold text-xs shadow-[0_0_18px_rgba(124,92,255,0.4)] transition-all cursor-pointer flex items-center gap-1.5"
              >
                <span className="material-symbols-outlined text-sm">shield</span>
                <span>Initialize & Launch Vault</span>
              </button>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

