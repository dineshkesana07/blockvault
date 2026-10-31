import React, { useState, useEffect } from 'react';
import { generatePassword, calculateEntropy } from '../services/cryptoService';

export default function CredentialModal({
  isOpen,
  onClose,
  onSave,
  initialData = null
}) {
  const [title, setTitle] = useState('');
  const [category, setCategory] = useState('logins');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [url, setUrl] = useState('');
  const [tags, setTags] = useState('');
  const [totpSecret, setTotpSecret] = useState('');
  const [notes, setNotes] = useState('');
  const [showPassword, setShowPassword] = useState(false);

  useEffect(() => {
    if (initialData) {
      setTitle(initialData.title || '');
      setCategory(initialData.category || 'logins');
      setUsername(initialData.username || '');
      setPassword(initialData.password || '');
      setUrl(initialData.url || '');
      setTags(initialData.tags ? initialData.tags.join(', ') : '');
      setTotpSecret(initialData.totpSecret || '');
      setNotes(initialData.notes || '');
    } else {
      setTitle('');
      setCategory('logins');
      setUsername('');
      setPassword(generatePassword({ length: 24 }));
      setUrl('');
      setTags('Production, Zero-Knowledge');
      setTotpSecret('');
      setNotes('');
    }
  }, [initialData, isOpen]);

  if (!isOpen) return null;

  const entropy = calculateEntropy(password);

  const handleSubmit = (e) => {
    e.preventDefault();
    if (!title.trim()) return;

    const parsedTags = tags
      .split(',')
      .map(t => t.trim())
      .filter(Boolean);

    onSave({
      title,
      category,
      username,
      password,
      url,
      tags: parsedTags,
      totpSecret,
      notes,
      favorite: initialData ? initialData.favorite : false
    });
    onClose();
  };

  const handleQuickGenerate = () => {
    const newPass = generatePassword({ length: 26 });
    setPassword(newPass);
  };

  const categories = [
    { id: 'logins', label: 'Login' },
    { id: 'keys', label: 'API Key' },
    { id: 'notes', label: 'Secure Note' },
    { id: 'ssh', label: 'SSH Key' },
    { id: 'cards', label: 'Card' }
  ];

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/80 backdrop-blur-md animate-fadeIn select-none"
      onClick={onClose}
    >
      <div
        className="w-full max-w-xl bg-surface-container-low border border-cyber-border-highlight rounded-xl shadow-2xl p-6 relative max-h-[90vh] overflow-y-auto text-left"
        onClick={e => e.stopPropagation()}
      >
        {/* Top Emissive Accent */}
        <div className="absolute top-0 left-0 right-0 h-1 bg-gradient-to-r from-primary via-secondary to-primary" />

        {/* Header */}
        <div className="flex items-center justify-between pb-4 mb-4 border-b border-cyber-border/60">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-lg bg-surface-container flex items-center justify-center text-primary border border-primary/30 shadow-inner">
              <span className="material-symbols-outlined text-2xl">
                {initialData ? 'edit_note' : 'add_moderator'}
              </span>
            </div>
            <div>
              <h2 className="font-headline-sm text-base text-on-surface font-semibold">
                {initialData ? 'Edit Stored Credential' : 'Create New Vault Credential'}
              </h2>
              <p className="font-mono-key text-[10px] text-on-surface-variant">
                Client-side encrypted with AES-256-GCM before enclave persistence
              </p>
            </div>
          </div>
          <button
            onClick={onClose}
            className="p-1 rounded-lg text-on-surface-variant hover:text-on-surface hover:bg-surface-container transition-colors cursor-pointer"
          >
            <span className="material-symbols-outlined text-lg">close</span>
          </button>
        </div>

        {/* Form Body */}
        <form onSubmit={handleSubmit} className="space-y-4 text-xs font-sans">
          
          {/* Category Selector Pills */}
          <div className="flex flex-col gap-1.5">
            <label className="text-on-surface font-medium">Credential Category</label>
            <div className="flex flex-wrap gap-1.5">
              {categories.map(cat => (
                <button
                  key={cat.id}
                  type="button"
                  onClick={() => setCategory(cat.id)}
                  className={`px-3 py-1 rounded-lg text-xs font-mono-key transition-all cursor-pointer ${
                    category === cat.id
                      ? 'bg-primary text-on-primary font-semibold shadow-[0_0_10px_rgba(124,92,255,0.3)]'
                      : 'bg-surface-container hover:bg-surface-container-high text-on-surface-variant border border-cyber-border'
                  }`}
                >
                  {cat.label}
                </button>
              ))}
            </div>
          </div>

          {/* Title & Target URL */}
          <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
            <div className="flex flex-col gap-1.5">
              <label className="text-on-surface font-medium">Title / Service Name *</label>
              <input
                type="text"
                required
                value={title}
                onChange={e => setTitle(e.target.value)}
                placeholder="e.g. GitHub Enterprise"
                className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface focus:outline-none focus:border-primary"
              />
            </div>
            <div className="flex flex-col gap-1.5">
              <label className="text-on-surface font-medium">Target URL / Host</label>
              <input
                type="text"
                value={url}
                onChange={e => setUrl(e.target.value)}
                placeholder="https://github.com"
                className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface font-mono-data text-xs focus:outline-none focus:border-primary"
              />
            </div>
          </div>

          {/* Username / Identity */}
          <div className="flex flex-col gap-1.5">
            <label className="text-on-surface font-medium">Username / Account Identifier</label>
            <input
              type="text"
              value={username}
              onChange={e => setUsername(e.target.value)}
              placeholder="e.g. elena.vance@org.io"
              className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface font-mono-data focus:outline-none focus:border-primary"
            />
          </div>

          {/* Secret / Password Input with Generator & Entropy */}
          <div className="flex flex-col gap-1.5">
            <div className="flex items-center justify-between">
              <label className="text-on-surface font-medium">Secret / Password *</label>
              <button
                type="button"
                onClick={handleQuickGenerate}
                className="text-[11px] text-secondary hover:underline cursor-pointer flex items-center gap-1 font-mono-key"
              >
                <span className="material-symbols-outlined text-xs">auto_fix_high</span>
                <span>Auto-Generate (26 chars)</span>
              </button>
            </div>
            <div className="relative">
              <input
                type={showPassword ? 'text' : 'password'}
                required
                value={password}
                onChange={e => setPassword(e.target.value)}
                placeholder="Cryptographic key or secret..."
                className="w-full pl-3 pr-20 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface font-mono-data text-xs focus:outline-none focus:border-primary tracking-wider"
              />
              <div className="absolute right-2 top-1/2 -translate-y-1/2 flex items-center gap-1">
                <button
                  type="button"
                  onClick={() => setShowPassword(!showPassword)}
                  className="p-1 text-on-surface-variant hover:text-on-surface cursor-pointer"
                  title={showPassword ? 'Hide Secret' : 'Reveal Secret'}
                >
                  <span className="material-symbols-outlined text-base">
                    {showPassword ? 'visibility_off' : 'visibility'}
                  </span>
                </button>
              </div>
            </div>

            {/* Entropy feedback bar */}
            {password && (
              <div className="space-y-1 pt-1">
                <div className="w-full h-1.5 bg-surface-container-highest rounded-full overflow-hidden">
                  <div
                    className={`h-full ${entropy.bgColor} transition-all`}
                    style={{ width: `${Math.min(100, entropy.score)}%` }}
                  />
                </div>
                <div className="flex items-center justify-between text-[11px] font-mono-key">
                  <span className={entropy.color}>{entropy.label} ({entropy.bits} BITS)</span>
                  <span className="text-on-surface-variant">Estimated crack: {entropy.crackTime}</span>
                </div>
              </div>
            )}
          </div>

          {/* TOTP Secret (Optional) */}
          <div className="flex flex-col gap-1.5">
            <label className="text-on-surface font-medium">TOTP Authenticator Secret (Optional)</label>
            <input
              type="text"
              value={totpSecret}
              onChange={e => setTotpSecret(e.target.value)}
              placeholder="e.g. JBSWY3DPEHPK3PXP (Base32)"
              className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface font-mono-data text-xs uppercase focus:outline-none focus:border-secondary"
            />
          </div>

          {/* Tags */}
          <div className="flex flex-col gap-1.5">
            <label className="text-on-surface font-medium">Tags (comma separated)</label>
            <input
              type="text"
              value={tags}
              onChange={e => setTags(e.target.value)}
              placeholder="Production, High-Risk, Enclave"
              className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-xs focus:outline-none focus:border-primary"
            />
          </div>

          {/* Secure Notes */}
          <div className="flex flex-col gap-1.5">
            <label className="text-on-surface font-medium">Secure Notes (Zero-Knowledge Encrypted)</label>
            <textarea
              rows={3}
              value={notes}
              onChange={e => setNotes(e.target.value)}
              placeholder="Encrypted recovery keys, IP restrictions, or notes..."
              className="w-full p-2.5 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-xs focus:outline-none focus:border-primary"
            />
          </div>

          {/* Modal Actions */}
          <div className="flex items-center justify-end gap-3 pt-3 border-t border-cyber-border/60">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-xs text-on-surface border border-cyber-border transition-colors cursor-pointer"
            >
              Cancel
            </button>
            <button
              type="submit"
              className="px-5 py-2 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs shadow-[0_0_14px_rgba(124,92,255,0.4)] transition-all cursor-pointer flex items-center gap-1.5"
            >
              <span className="material-symbols-outlined text-sm">lock</span>
              <span>{initialData ? 'Commit Changes' : 'Encrypt & Store'}</span>
            </button>
          </div>

        </form>
      </div>
    </div>
  );
}
