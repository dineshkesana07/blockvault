import React, { useState, useEffect } from 'react';

export default function CommandPalette({
  isOpen,
  onClose,
  credentials = [],
  onSelectCredential,
  onNavigate,
  onLockVault,
  onOpenNewSecret
}) {
  const [query, setQuery] = useState('');
  const [selectedIndex, setSelectedIndex] = useState(0);

  useEffect(() => {
    setQuery('');
    setSelectedIndex(0);
  }, [isOpen]);

  if (!isOpen) return null;

  // Static commands
  const systemCommands = [
    { id: 'cmd-new', label: '+ Add New Credential', type: 'action', icon: 'add', action: onOpenNewSecret },
    { id: 'cmd-dash', label: 'Go to Dashboard', type: 'nav', icon: 'dashboard', action: () => onNavigate('dashboard') },
    { id: 'cmd-vault', label: 'Go to Password Vault', type: 'nav', icon: 'key', action: () => onNavigate('password-vault') },
    { id: 'cmd-gen', label: 'Go to Entropy Generator', type: 'nav', icon: 'casino', action: () => onNavigate('password-generator') },
    { id: 'cmd-sec', label: 'Go to Security Health Center', type: 'nav', icon: 'security_update_good', action: () => onNavigate('security-center') },
    { id: 'cmd-audit', label: 'Go to Audit Ledger', type: 'nav', icon: 'receipt_long', action: () => onNavigate('audit-ledger') },
    { id: 'cmd-lock', label: 'Lock Vault Enclave', type: 'action', icon: 'lock', action: onLockVault }
  ];

  // Filtered items
  const matchingCreds = credentials.filter(c =>
    c.title.toLowerCase().includes(query.toLowerCase()) ||
    c.username.toLowerCase().includes(query.toLowerCase()) ||
    c.tags.some(t => t.toLowerCase().includes(query.toLowerCase()))
  ).map(c => ({
    id: c.id,
    label: c.title,
    sublabel: c.username,
    type: 'credential',
    icon: 'token',
    action: () => onSelectCredential(c)
  }));

  const matchingSystem = systemCommands.filter(c =>
    c.label.toLowerCase().includes(query.toLowerCase())
  );

  const results = [...matchingCreds, ...matchingSystem];

  const handleKeyDown = (e) => {
    if (e.key === 'Escape') {
      onClose();
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      setSelectedIndex(prev => (prev < results.length - 1 ? prev + 1 : prev));
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      setSelectedIndex(prev => (prev > 0 ? prev - 1 : 0));
    } else if (e.key === 'Enter') {
      e.preventDefault();
      if (results[selectedIndex]) {
        results[selectedIndex].action();
        onClose();
      }
    }
  };

  return (
    <div
      className="fixed inset-0 z-50 flex items-start justify-center pt-24 bg-black/75 backdrop-blur-md p-4 animate-fadeIn"
      onClick={onClose}
    >
      <div
        className="w-full max-w-xl bg-surface-container-low border border-cyber-border-highlight rounded-xl shadow-2xl overflow-hidden flex flex-col"
        onClick={e => e.stopPropagation()}
        onKeyDown={handleKeyDown}
      >
        {/* Search Input Bar */}
        <div className="flex items-center gap-3 px-4 py-3 border-b border-cyber-border bg-surface-container-lowest">
          <span className="material-symbols-outlined text-secondary text-xl">search</span>
          <input
            type="text"
            autoFocus
            value={query}
            onChange={e => {
              setQuery(e.target.value);
              setSelectedIndex(0);
            }}
            placeholder="Type a command or search stored credentials..."
            className="w-full bg-transparent text-sm text-on-surface placeholder:text-on-surface-variant focus:outline-none font-sans"
          />
          <kbd className="font-mono-key text-[10px] text-on-surface-variant bg-surface-container px-2 py-0.5 rounded border border-cyber-border">
            ESC
          </kbd>
        </div>

        {/* Results List */}
        <div className="max-h-80 overflow-y-auto p-2 flex flex-col gap-1">
          {results.length === 0 ? (
            <div className="p-6 text-center text-xs text-on-surface-variant font-mono-data">
              No matching credentials or commands found.
            </div>
          ) : (
            results.map((item, index) => {
              const isSelected = index === selectedIndex;
              return (
                <button
                  key={item.id}
                  onClick={() => {
                    item.action();
                    onClose();
                  }}
                  onMouseEnter={() => setSelectedIndex(index)}
                  className={`flex items-center justify-between px-3 py-2.5 rounded-lg text-left text-xs transition-colors ${
                    isSelected
                      ? 'bg-cyber-card-active text-on-surface border border-secondary/40 shadow-sm'
                      : 'text-on-surface-variant hover:bg-surface-container'
                  }`}
                >
                  <div className="flex items-center gap-2.5">
                    <span className={`material-symbols-outlined text-base ${isSelected ? 'text-secondary' : 'text-on-surface-variant'}`}>
                      {item.icon}
                    </span>
                    <div className="flex flex-col">
                      <span className={`font-medium ${isSelected ? 'text-on-surface' : 'text-on-surface-variant'}`}>
                        {item.label}
                      </span>
                      {item.sublabel && (
                        <span className="font-mono-data text-[10px] text-on-surface-variant">
                          {item.sublabel}
                        </span>
                      )}
                    </div>
                  </div>
                  <span className="font-mono-key text-[10px] uppercase text-outline px-1.5 py-0.5 rounded bg-surface-container-lowest">
                    {item.type}
                  </span>
                </button>
              );
            })
          )}
        </div>

        {/* Footer info */}
        <div className="flex items-center justify-between px-4 py-2 border-t border-cyber-border/40 bg-surface-container-lowest text-[11px] font-mono-key text-on-surface-variant">
          <span>Navigate: ↑ / ↓</span>
          <span>Select: ↵</span>
          <span>Close: ESC</span>
        </div>
      </div>
    </div>
  );
}
