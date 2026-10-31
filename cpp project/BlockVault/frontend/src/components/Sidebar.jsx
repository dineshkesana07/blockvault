import React, { useState, useEffect } from 'react';

export default function Sidebar({
  activePage,
  setActivePage,
  onLockVault,
  isMobileOpen = false,
  setIsMobileOpen
}) {
  const [lockTimer, setLockTimer] = useState(898); // 14:58 seconds

  useEffect(() => {
    const timer = setInterval(() => {
      setLockTimer(prev => (prev > 0 ? prev - 1 : 0));
    }, 1000);
    return () => clearInterval(timer);
  }, []);

  const formatTimer = (seconds) => {
    const m = Math.floor(seconds / 60).toString().padStart(2, '0');
    const s = (seconds % 60).toString().padStart(2, '0');
    return `[${m}:${s}]`;
  };

  const navItems = [
    { id: 'dashboard', label: 'Dashboard', icon: 'dashboard' },
    { id: 'password-vault', label: 'Password Vault', icon: 'key' },
    { id: 'password-generator', label: 'Password Generator', icon: 'casino' },
    { id: 'security-center', label: 'Security Health', icon: 'security_update_good' },
    { id: 'audit-ledger', label: 'Audit Ledger', icon: 'receipt_long' },
    { id: 'settings', label: 'Settings & Recovery', icon: 'settings_suggest' },
    { id: 'landing', label: 'Public Showcase', icon: 'public' }
  ];

  const handleNavClick = (id) => {
    setActivePage(id);
    if (setIsMobileOpen) {
      setIsMobileOpen(false);
    }
  };

  return (
    <>
      {/* Mobile Backdrop */}
      {isMobileOpen && (
        <div
          className="fixed inset-0 bg-black/70 backdrop-blur-sm z-40 lg:hidden transition-opacity"
          onClick={() => setIsMobileOpen(false)}
        />
      )}

      {/* Sidebar Rail / Drawer */}
      <aside
        className={`fixed left-0 top-0 h-full w-72 bg-surface-container-lowest border-r border-cyber-border/60 z-50 flex flex-col justify-between p-4 select-none shadow-[0_1px_8px_rgba(0,0,0,0.5)] transition-transform duration-300 ease-in-out ${
          isMobileOpen ? 'translate-x-0' : '-translate-x-full lg:translate-x-0'
        }`}
      >
        {/* Top Branding & Nav */}
        <div className="flex flex-col gap-4">
          {/* Emblem & Title */}
          <div className="flex items-center justify-between px-1 py-1">
            <div
              className="flex items-center gap-3 cursor-pointer group"
              onClick={() => handleNavClick('dashboard')}
            >
              <div className="w-9 h-9 rounded-lg bg-surface-container flex items-center justify-center border border-primary/40 shadow-[0_0_12px_rgba(124,92,255,0.25)] text-primary group-hover:border-primary transition-colors">
                <span className="material-symbols-outlined text-xl">enhanced_encryption</span>
              </div>
              <div className="flex flex-col text-left">
                <span className="font-headline-sm uppercase tracking-tight text-on-surface font-bold text-base">
                  BlockVault
                </span>
                <span className="font-mono-key text-[10px] text-secondary tracking-widest uppercase">
                  Zero-Knowledge OS
                </span>
              </div>
            </div>

            {/* Mobile close button */}
            <button
              onClick={() => setIsMobileOpen(false)}
              className="lg:hidden p-1.5 rounded-lg text-on-surface-variant hover:text-on-surface hover:bg-surface-container transition-colors cursor-pointer"
            >
              <span className="material-symbols-outlined text-lg">close</span>
            </button>
          </div>

          {/* Vault Switcher */}
          <div className="bg-surface-container px-3.5 py-2.5 rounded-lg flex items-center justify-between border border-cyber-border/70 text-left cursor-pointer hover:bg-surface-container-high transition-colors">
            <div className="flex items-center gap-2.5">
              <span className="material-symbols-outlined text-secondary text-base">shield</span>
              <div className="flex flex-col">
                <span className="font-body-sm font-semibold text-on-surface text-xs">
                  Personal Vault
                </span>
                <span className="font-mono-key text-[10px] text-on-surface-variant">
                  256-bit AES-GCM
                </span>
              </div>
            </div>
            <span className="material-symbols-outlined text-on-surface-variant text-sm">unfold_more</span>
          </div>

          {/* Navigation Items */}
          <nav className="flex flex-col gap-1 text-xs font-medium">
            {navItems.map((item) => {
              const isActive = activePage === item.id;
              return (
                <button
                  key={item.id}
                  onClick={() => handleNavClick(item.id)}
                  className={`flex items-center gap-2.5 px-3 py-2.5 rounded-lg text-left transition-all cursor-pointer ${
                    isActive
                      ? 'bg-primary-container text-on-primary-container font-semibold shadow-[0_0_12px_rgba(124,92,255,0.3)]'
                      : 'text-on-surface-variant hover:bg-surface-container hover:text-on-surface'
                  }`}
                >
                  <span className={`material-symbols-outlined text-base ${isActive ? 'text-on-primary-container' : 'text-on-surface-variant'}`}>
                    {item.icon}
                  </span>
                  <span>{item.label}</span>
                </button>
              );
            })}
          </nav>
        </div>

        {/* Bottom Section: Storage & Enclave Seal */}
        <div className="flex flex-col gap-3">
          {/* Storage Meter */}
          <div className="bg-surface-container-low p-3 rounded-lg border border-cyber-border/60 text-left text-xs">
            <div className="flex justify-between font-mono-key text-[10px] text-on-surface-variant mb-1">
              <span>ENCRYPTED STORAGE</span>
              <span className="text-secondary font-semibold">42% USED</span>
            </div>
            <div className="w-full h-1.5 bg-surface-container-highest rounded-full overflow-hidden">
              <div className="w-[42%] h-full bg-secondary rounded-full shadow-[0_0_8px_rgba(34,211,238,0.4)]" />
            </div>
            <span className="font-mono-key text-[10px] text-on-surface-variant mt-1 block">
              1.28 GB / 3.0 GB Allocated
            </span>
          </div>

          {/* Lock Vault Button */}
          <button
            onClick={onLockVault}
            className="w-full flex items-center justify-between px-3 py-2 rounded-lg bg-surface-container hover:bg-error-container/20 hover:text-error text-xs border border-cyber-border text-on-surface cursor-pointer transition-colors"
          >
            <div className="flex items-center gap-2">
              <span className="material-symbols-outlined text-sm">lock</span>
              <span>Lock Vault Now</span>
            </div>
            <span className="font-mono-key text-[10px] text-on-surface-variant font-mono-numbers">
              {formatTimer(lockTimer)}
            </span>
          </button>

          {/* Status footer */}
          <div className="flex items-center justify-between px-1 text-[10px] font-mono-key text-on-surface-variant">
            <span>STATUS: SEALED</span>
            <span className="text-secondary bg-surface-container px-1.5 py-0.5 rounded border border-secondary/20">
              CYBER NOIR
            </span>
          </div>
        </div>
      </aside>
    </>
  );
}
