import React, { useState } from 'react';

export default function Header({
  searchTerm,
  setSearchTerm,
  onOpenNewSecret,
  onLockVault,
  onOpenCommandPalette,
  isMobileOpen = false,
  setIsMobileOpen,
  onNavigate
}) {
  const [showNotifications, setShowNotifications] = useState(false);
  const [showProfileMenu, setShowProfileMenu] = useState(false);

  const notifications = [
    { id: 1, title: 'Enclave Key Rotation Verified', time: '12m ago', type: 'success', desc: 'SHA-256 chain integrity verified.' },
    { id: 2, title: 'Weak Password Warning', time: '1h ago', type: 'warning', desc: 'Legacy VPN password has sub-40-bit entropy.' },
    { id: 3, title: 'Merkle Ledger Synced with Node', time: '3h ago', type: 'info', desc: 'Block #01428 added to tamper-evident journal.' }
  ];

  return (
    <header className="fixed top-0 left-0 lg:left-72 right-0 h-16 bg-surface-container-lowest/90 backdrop-blur-xl shadow-[0_1px_8px_rgba(0,0,0,0.4)] z-40 flex items-center justify-between px-4 sm:px-6 lg:px-8 border-b border-cyber-border/50 select-none transition-all">
      {/* Left: Mobile Drawer Trigger + Search Bar */}
      <div className="flex items-center gap-3 flex-1 max-w-xl">
        {/* Mobile Hamburger Button */}
        <button
          onClick={() => setIsMobileOpen && setIsMobileOpen(!isMobileOpen)}
          className="lg:hidden p-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface-variant hover:text-on-surface transition-colors cursor-pointer"
          title="Open Navigation"
        >
          <span className="material-symbols-outlined text-xl">menu</span>
        </button>

        {/* Omnisearch Input */}
        <div className="relative w-full">
          <span className="material-symbols-outlined absolute left-3 top-1/2 -translate-y-1/2 text-on-surface-variant text-base">
            search
          </span>
          <input
            type="text"
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
            placeholder="Search credentials, keys, vaults... (Cmd+K)"
            className="w-full pl-9 pr-14 py-1.5 bg-surface-container rounded-lg font-body-sm text-xs text-on-surface placeholder:text-on-surface-variant/70 focus:outline-none focus:ring-1 focus:ring-primary border border-cyber-border/70 shadow-inner"
          />
          <button
            onClick={onOpenCommandPalette}
            className="absolute right-2 top-1/2 -translate-y-1/2 font-mono-key text-[10px] text-outline bg-surface-container-lowest px-1.5 py-0.5 rounded border border-cyber-border hover:text-secondary cursor-pointer"
            title="Open Command Palette (Cmd+K / Ctrl+K)"
          >
            ⌘K
          </button>
        </div>
      </div>

      {/* Right Header Actions */}
      <div className="flex items-center gap-2 sm:gap-3.5">
        {/* Quick New Secret Action */}
        <button
          onClick={onOpenNewSecret}
          className="flex items-center gap-1.5 bg-primary text-on-primary font-body-sm text-xs font-semibold px-3 sm:px-3.5 py-1.5 sm:py-2 rounded-lg hover:bg-primary-hover hover:text-white transition-all shadow-[0_0_12px_rgba(124,92,255,0.35)] cursor-pointer active:scale-95"
        >
          <span className="material-symbols-outlined text-sm">add</span>
          <span className="hidden sm:inline">New Secret</span>
        </button>

        {/* Lock Vault Button */}
        <button
          onClick={onLockVault}
          className="hidden sm:flex items-center gap-1.5 bg-error/10 hover:bg-error/20 text-error font-mono-key text-xs px-2.5 sm:px-3 py-1.5 rounded-lg border border-error/30 transition-all cursor-pointer"
          title="Purge cryptographic keys from browser memory"
        >
          <span className="material-symbols-outlined text-sm">lock</span>
          <span className="hidden md:inline">Lock</span>
        </button>

        {/* Sealed Status Telemetry Badge */}
        <div className="hidden xl:flex items-center gap-1.5 font-mono-key text-[11px] px-2.5 py-1 rounded bg-surface-container text-secondary border border-secondary/20 shadow-[0_0_10px_rgba(34,211,238,0.15)]">
          <span className="w-2 h-2 rounded-full bg-secondary animate-pulse" />
          <span>Enclave Sealed</span>
        </div>

        {/* Notification Bell */}
        <div className="relative">
          <button
            onClick={() => {
              setShowNotifications(!showNotifications);
              setShowProfileMenu(false);
            }}
            className="relative p-2 rounded-lg text-on-surface-variant hover:bg-surface-container hover:text-on-surface transition-colors cursor-pointer"
            title="Notifications"
          >
            <span className="material-symbols-outlined text-xl">notifications</span>
            <span className="absolute top-1.5 right-1.5 w-2 h-2 rounded-full bg-secondary animate-ping" />
            <span className="absolute top-1.5 right-1.5 w-2 h-2 rounded-full bg-secondary" />
          </button>

          {showNotifications && (
            <div className="absolute right-0 mt-2 w-80 bg-surface-container-low border border-cyber-border-highlight rounded-xl shadow-2xl p-4 z-50 animate-fadeIn text-left">
              <div className="flex items-center justify-between pb-2 mb-2 border-b border-cyber-border/60">
                <span className="font-headline-sm text-xs uppercase text-on-surface font-semibold tracking-wider">
                  Enclave Telemetry Alerts
                </span>
                <span className="font-mono-key text-[10px] text-secondary">3 RECENT</span>
              </div>
              <div className="flex flex-col gap-2">
                {notifications.map(n => (
                  <div key={n.id} className="p-2.5 rounded-lg bg-surface-container hover:bg-surface-container-high transition-colors text-left flex flex-col gap-0.5 border border-cyber-border/40">
                    <div className="flex items-center justify-between">
                      <span className="text-xs text-on-surface font-medium">{n.title}</span>
                      <span className="font-mono-key text-[10px] text-on-surface-variant">{n.time}</span>
                    </div>
                    <span className="font-body-md text-[11px] text-on-surface-variant leading-tight">{n.desc}</span>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>

        {/* User Profile / Enclave Identity */}
        <div className="relative">
          <div
            onClick={() => {
              setShowProfileMenu(!showProfileMenu);
              setShowNotifications(false);
            }}
            className="flex items-center gap-2 pl-1 sm:pl-2 cursor-pointer group"
          >
            <div className="w-8 h-8 rounded-full bg-primary/20 text-primary font-bold flex items-center justify-center text-xs border border-primary/40 group-hover:border-primary transition-colors">
              EV
            </div>
            <div className="hidden xl:flex flex-col text-left">
              <span className="font-body-sm font-semibold text-on-surface text-xs leading-tight">
                Dr. Elena Vance
              </span>
              <span className="font-mono-key text-[10px] text-on-surface-variant leading-tight">
                Vault Architect
              </span>
            </div>
            <span className="material-symbols-outlined text-on-surface-variant text-sm hidden sm:inline">
              expand_more
            </span>
          </div>

          {showProfileMenu && (
            <div className="absolute right-0 mt-2 w-64 bg-surface-container-low border border-cyber-border-highlight rounded-xl shadow-2xl p-4 z-50 animate-fadeIn text-left">
              <div className="flex items-center gap-3 pb-3 mb-3 border-b border-cyber-border/60">
                <div className="w-10 h-10 rounded-full bg-primary/20 text-primary font-bold flex items-center justify-center text-sm border border-primary/40">
                  EV
                </div>
                <div className="flex flex-col">
                  <span className="font-semibold text-xs text-on-surface">Dr. Elena Vance</span>
                  <span className="font-mono-key text-[10px] text-on-surface-variant truncate max-w-[140px]">
                    elena.vance@org.io
                  </span>
                </div>
              </div>

              <div className="flex flex-col gap-1 text-xs">
                <button
                  onClick={() => {
                    setShowProfileMenu(false);
                    if (onNavigate) onNavigate('settings');
                  }}
                  className="flex items-center gap-2 px-2.5 py-2 rounded-lg text-on-surface hover:bg-surface-container text-left transition-colors cursor-pointer"
                >
                  <span className="material-symbols-outlined text-base text-secondary">settings</span>
                  <span>Enclave Settings</span>
                </button>
                <button
                  onClick={() => {
                    setShowProfileMenu(false);
                    if (onNavigate) onNavigate('audit-ledger');
                  }}
                  className="flex items-center gap-2 px-2.5 py-2 rounded-lg text-on-surface hover:bg-surface-container text-left transition-colors cursor-pointer"
                >
                  <span className="material-symbols-outlined text-base text-primary">receipt_long</span>
                  <span>Merkle Audit Log</span>
                </button>
                <button
                  onClick={() => {
                    setShowProfileMenu(false);
                    onLockVault();
                  }}
                  className="flex items-center gap-2 px-2.5 py-2 rounded-lg text-error hover:bg-error/10 text-left transition-colors cursor-pointer mt-1 border-t border-cyber-border/40 pt-2"
                >
                  <span className="material-symbols-outlined text-base">lock</span>
                  <span>Purge Keys & Lock</span>
                </button>
              </div>
            </div>
          )}
        </div>

      </div>
    </header>
  );
}
