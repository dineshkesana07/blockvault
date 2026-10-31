import React, { useState } from 'react';
import ConfirmModal from '../components/ConfirmModal';

export default function SettingsPage({ onShowToast, onOpenExport }) {
  const [autoLockTimeout, setAutoLockTimeout] = useState('15');
  const [argonMemory, setArgonMemory] = useState('64');
  const [argonIterations, setArgonIterations] = useState('4');
  const [biometricEnabled, setBiometricEnabled] = useState(true);
  const [clipboardClearSeconds, setClipboardClearSeconds] = useState('30');
  const [showRecoveryWords, setShowRecoveryWords] = useState(false);
  const [showPurgeModal, setShowPurgeModal] = useState(false);

  const recoveryPhrase = [
    'matrix', 'quantum', 'lattice', 'argon', 'shannon', 'entropy',
    'sovereign', 'glacier', 'shield', 'obsidian', 'circuit', 'vertex',
    'enclave', 'poly1305', 'orbital', 'plasma', 'cascade', 'monolith',
    'sentinel', 'phantom', 'beacon', 'spectrum', 'zenith', 'pulse'
  ];

  const handleSaveSettings = (e) => {
    e.preventDefault();
    onShowToast('Enclave security policies committed to hardware storage.', 'success');
  };

  const handleDownloadKit = () => {
    const text = `BLOCKVAULT EMERGENCY RECOVERY KIT
====================================
Vault: Personal Cryptographic Enclave
User: Dr. Elena Vance (0x89F4...D12A)
Timestamp: ${new Date().toISOString()}

RECOVERY PHRASE (24 WORDS):
${recoveryPhrase.map((w, i) => `${i + 1}. ${w}`).join('\n')}

Argon2id Config: Memory=${argonMemory}MB, Iterations=${argonIterations}
Cipher: AES-256-GCM
====================================`;

    const blob = new Blob([text], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `blockvault-recovery-mnemonic-${Date.now()}.txt`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    onShowToast('Emergency Recovery Kit downloaded.', 'success');
  };

  const handlePurgeKeystore = () => {
    localStorage.removeItem('blockvault_credentials_v1');
    localStorage.removeItem('blockvault_merkle_ledger_v1');
    onShowToast('Keystore purged. Default mock vault re-initialized.', 'warning');
    setTimeout(() => {
      window.location.reload();
    }, 1000);
  };

  return (
    <div className="flex flex-col w-full pb-16 space-y-6 text-left font-sans select-none">
      <div className="flex flex-col gap-1 pt-2">
        <h1 className="font-headline-lg text-2xl font-bold text-on-surface tracking-tight">
          Settings & Cryptographic Recovery
        </h1>
        <p className="font-body-md text-xs text-on-surface-variant">
          Configure hardware enclave parameters, Argon2id derivation cost factors, auto-lock policies, and disaster recovery keys.
        </p>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Enclave Security Settings Form */}
        <form onSubmit={handleSaveSettings} className="bg-surface-container-low rounded-xl p-6 border border-cyber-border shadow-md flex flex-col gap-5">
          <div className="flex items-center justify-between pb-2 border-b border-cyber-border/40">
            <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
              Enclave Execution Policies
            </h3>
            <span className="font-mono-key text-[10px] text-secondary">ACTIVE PROTOCOL</span>
          </div>

          {/* Auto-Lock Timeout */}
          <div className="flex flex-col gap-1.5 text-xs">
            <label className="text-on-surface font-medium">Idle Enclave Auto-Lock Timeout</label>
            <select
              value={autoLockTimeout}
              onChange={e => setAutoLockTimeout(e.target.value)}
              className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface focus:outline-none"
            >
              <option value="1">1 Minute (Strict Airgap)</option>
              <option value="5">5 Minutes</option>
              <option value="15">15 Minutes (Default)</option>
              <option value="30">30 Minutes</option>
              <option value="60">60 Minutes</option>
            </select>
          </div>

          {/* Clipboard Sanitization */}
          <div className="flex flex-col gap-1.5 text-xs">
            <label className="text-on-surface font-medium">Purge Clipboard Memory</label>
            <select
              value={clipboardClearSeconds}
              onChange={e => setClipboardClearSeconds(e.target.value)}
              className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface focus:outline-none"
            >
              <option value="10">After 10 Seconds</option>
              <option value="30">After 30 Seconds (Default)</option>
              <option value="60">After 60 Seconds</option>
            </select>
          </div>

          {/* Biometric Toggle */}
          <div className="flex items-center justify-between p-3 rounded-lg bg-surface-container border border-cyber-border/40 text-xs">
            <div className="flex flex-col">
              <span className="font-semibold text-on-surface">FIDO2 Hardware Passkeys</span>
              <span className="text-on-surface-variant text-[11px]">Require WebAuthn step-up verification</span>
            </div>
            <input
              type="checkbox"
              checked={biometricEnabled}
              onChange={e => setBiometricEnabled(e.target.checked)}
              className="accent-primary w-4 h-4 cursor-pointer"
            />
          </div>

          {/* Argon2id Key Derivation Cost */}
          <div className="flex flex-col gap-3 pt-2 border-t border-cyber-border/40">
            <span className="font-mono-key text-[11px] text-secondary uppercase font-semibold">
              Argon2id Memory-Hard Parameters
            </span>
            <div className="grid grid-cols-2 gap-3 text-xs">
              <div className="flex flex-col gap-1">
                <label className="text-on-surface-variant">Memory Block (MB)</label>
                <input
                  type="number"
                  value={argonMemory}
                  onChange={e => setArgonMemory(e.target.value)}
                  className="px-3 py-1.5 bg-surface-container rounded border border-cyber-border text-on-surface font-mono-data"
                />
              </div>
              <div className="flex flex-col gap-1">
                <label className="text-on-surface-variant">Time Cost (Iterations)</label>
                <input
                  type="number"
                  value={argonIterations}
                  onChange={e => setArgonIterations(e.target.value)}
                  className="px-3 py-1.5 bg-surface-container rounded border border-cyber-border text-on-surface font-mono-data"
                />
              </div>
            </div>
          </div>

          <button
            type="submit"
            className="w-full py-2.5 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs shadow-[0_0_12px_rgba(124,92,255,0.35)] transition-all cursor-pointer mt-2"
          >
            Save Enclave Configuration
          </button>
        </form>

        {/* Emergency Recovery Kit & Phrase */}
        <div className="bg-surface-container-low rounded-xl p-6 border border-cyber-border shadow-md flex flex-col justify-between gap-5">
          <div className="flex flex-col gap-4">
            <div className="flex items-center justify-between pb-2 border-b border-cyber-border/40">
              <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
                Emergency 24-Word Recovery Kit
              </h3>
              <span className="font-mono-key text-[10px] text-amber-400">DISASTER ESCROW</span>
            </div>

            <p className="text-xs text-on-surface-variant leading-relaxed">
              If you lose your Master Password, this 24-word deterministic mnemonic can reconstruct your private enclave encryption keys. Store offline in an air-gapped safe.
            </p>

            {/* Revealed words or masked */}
            {showRecoveryWords ? (
              <div className="grid grid-cols-3 sm:grid-cols-4 gap-2 p-3 bg-surface-container rounded-lg border border-secondary/30 font-mono-data text-xs max-h-48 overflow-y-auto">
                {recoveryPhrase.map((word, idx) => (
                  <div key={idx} className="p-1.5 rounded bg-surface-container-lowest text-on-surface text-[11px] flex items-center gap-1.5 border border-cyber-border/40">
                    <span className="text-on-surface-variant text-[10px]">{idx + 1}.</span>
                    <span className="text-secondary font-semibold">{word}</span>
                  </div>
                ))}
              </div>
            ) : (
              <div className="p-6 text-center rounded-lg bg-surface-container border border-cyber-border/60 flex flex-col items-center gap-2">
                <span className="material-symbols-outlined text-secondary text-2xl">visibility_off</span>
                <span className="text-xs text-on-surface font-medium">Recovery Mnemonic Masked</span>
                <span className="text-[11px] text-on-surface-variant">Click reveal to inspect phrases</span>
              </div>
            )}
          </div>

          <div className="flex flex-wrap items-center gap-2.5">
            <button
              onClick={() => setShowRecoveryWords(!showRecoveryWords)}
              className="flex-1 py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface text-xs font-medium border border-cyber-border transition-colors cursor-pointer"
            >
              {showRecoveryWords ? 'Mask Recovery Kit' : 'Reveal 24-Word Phrase'}
            </button>
            <button
              onClick={() => {
                navigator.clipboard.writeText(recoveryPhrase.join(' '));
                onShowToast('Copied 24-word recovery phrase to clipboard.', 'success');
              }}
              className="px-3.5 py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-secondary border border-secondary/30 text-xs font-mono-key transition-colors cursor-pointer"
            >
              Copy Words
            </button>
            <button
              onClick={handleDownloadKit}
              className="px-3.5 py-2 rounded-lg bg-secondary text-on-secondary font-bold text-xs hover:bg-secondary-fixed transition-colors shadow-sm cursor-pointer"
            >
              Download .TXT
            </button>
          </div>
        </div>
      </div>

      {/* Backup & Disaster Management Section */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <div className="p-5 rounded-xl bg-surface-container-low border border-cyber-border shadow-md space-y-3">
          <div className="flex items-center gap-2">
            <span className="material-symbols-outlined text-secondary text-xl">backup</span>
            <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
              Offline Vault Snapshots
            </h3>
          </div>
          <p className="text-xs text-on-surface-variant leading-relaxed">
            Create or restore encrypted snapshot files (<code className="text-secondary font-mono">.enc.json</code>). All secrets remain sealed with client-derived keys.
          </p>
          <div className="flex items-center gap-3 pt-2">
            <button
              onClick={onOpenExport}
              className="px-4 py-2 rounded-lg bg-primary hover:bg-primary-hover text-on-primary text-xs font-semibold shadow-md transition-all cursor-pointer flex items-center gap-1.5"
            >
              <span className="material-symbols-outlined text-sm">settings_backup_restore</span>
              <span>Export / Restore Center</span>
            </button>
          </div>
        </div>

        <div className="p-5 rounded-xl bg-surface-container-low border border-error/30 shadow-md space-y-3">
          <div className="flex items-center gap-2">
            <span className="material-symbols-outlined text-error text-xl">dangerous</span>
            <h3 className="font-headline-sm text-sm font-semibold text-error">
              Emergency Keystore Reset
            </h3>
          </div>
          <p className="text-xs text-on-surface-variant leading-relaxed">
            Purge all local credentials and reset the enclave back to genesis state. This action cannot be reversed.
          </p>
          <div className="flex items-center gap-3 pt-2">
            <button
              onClick={() => setShowPurgeModal(true)}
              className="px-4 py-2 rounded-lg bg-error/15 hover:bg-error/25 text-error border border-error/40 text-xs font-semibold transition-all cursor-pointer flex items-center gap-1.5"
            >
              <span className="material-symbols-outlined text-sm">delete_forever</span>
              <span>Purge Local Enclave</span>
            </button>
          </div>
        </div>
      </div>

      {/* Confirmation Modal */}
      <ConfirmModal
        isOpen={showPurgeModal}
        onClose={() => setShowPurgeModal(false)}
        onConfirm={handlePurgeKeystore}
        title="Purge Entire Enclave Keystore"
        message="This operation will destroy all local encrypted credentials, SHA-256 Merkle records, and ephemeral keys stored in browser storage."
        confirmText="Confirm Permanent Purge"
        danger={true}
      />
    </div>
  );
}
