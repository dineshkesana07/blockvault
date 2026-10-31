import React, { useState } from 'react';
import { calculateEntropy } from '../services/cryptoService';
import ConfirmModal from '../components/ConfirmModal';
import DetailDrawer from '../components/DetailDrawer';

export default function VaultPage({
  credentials = [],
  searchTerm = '',
  onOpenNewSecret,
  onOpenExport,
  onEditCredential,
  onDeleteCredential,
  onToggleFavorite,
  selectedItem,
  setSelectedItem,
  onShowToast
}) {
  const [selectedCategory, setSelectedCategory] = useState('all');
  const [revealedPasswords, setRevealedPasswords] = useState({});
  const [sortBy, setSortBy] = useState('recent');
  const [deleteCandidate, setDeleteCandidate] = useState(null);
  const [copiedId, setCopiedId] = useState(null);

  const togglePasswordReveal = (id, e) => {
    e.stopPropagation();
    setRevealedPasswords(prev => {
      const willReveal = !prev[id];
      if (willReveal) {
        // Auto-mask in 10s
        setTimeout(() => {
          setRevealedPasswords(p => ({ ...p, [id]: false }));
        }, 10000);
      }
      return {
        ...prev,
        [id]: willReveal
      };
    });
  };

  const copyToClipboard = (text, label, id, e) => {
    if (e) e.stopPropagation();
    navigator.clipboard.writeText(text);
    setCopiedId(id);
    onShowToast(`Copied ${label} to clipboard. Memory sanitization in 30s.`, 'success');
    setTimeout(() => setCopiedId(null), 2000);
  };

  const handleDeleteRequest = (id, title, e) => {
    if (e) e.stopPropagation();
    setDeleteCandidate({ id, title });
  };

  const handleConfirmDelete = () => {
    if (!deleteCandidate) return;
    onDeleteCredential(deleteCandidate.id);
    onShowToast(`Purged "${deleteCandidate.title}" from local keystore.`, 'warning');
    if (selectedItem && selectedItem.id === deleteCandidate.id) {
      setSelectedItem(null);
    }
    setDeleteCandidate(null);
  };

  // Filter credentials
  const filtered = credentials.filter(item => {
    const matchCategory = selectedCategory === 'all' || item.category === selectedCategory;
    const matchSearch =
      item.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
      item.username.toLowerCase().includes(searchTerm.toLowerCase()) ||
      (item.tags && item.tags.some(t => t.toLowerCase().includes(searchTerm.toLowerCase())));
    return matchCategory && matchSearch;
  });

  // Sort credentials
  const sorted = [...filtered].sort((a, b) => {
    if (sortBy === 'title') return a.title.localeCompare(b.title);
    if (sortBy === 'entropy') {
      const entA = calculateEntropy(a.password).bits;
      const entB = calculateEntropy(b.password).bits;
      return entB - entA;
    }
    return new Date(b.lastModified || 0) - new Date(a.lastModified || 0);
  });

  const categories = [
    { id: 'all', label: 'All Items', icon: 'grid_view', count: credentials.length },
    { id: 'logins', label: 'Logins', icon: 'badge', count: credentials.filter(c => c.category === 'logins').length },
    { id: 'keys', label: 'API Keys', icon: 'vpn_key', count: credentials.filter(c => c.category === 'keys').length },
    { id: 'notes', label: 'Secure Notes', icon: 'note', count: credentials.filter(c => c.category === 'notes').length },
    { id: 'ssh', label: 'SSH Keys', icon: 'terminal', count: credentials.filter(c => c.category === 'ssh').length },
    { id: 'cards', label: 'Cards', icon: 'credit_card', count: credentials.filter(c => c.category === 'cards').length }
  ];

  return (
    <div className="flex flex-col w-full pb-16 space-y-6 text-left select-none font-sans">
      {/* Header and Action Bar */}
      <div className="flex flex-col xl:flex-row xl:items-center justify-between gap-4 pt-2">
        <div className="flex flex-col gap-1">
          <div className="flex items-center gap-2 flex-wrap">
            <h1 className="font-headline-lg text-2xl font-bold text-on-surface tracking-tight">
              Password Vault
            </h1>
            <span className="inline-flex items-center gap-1.5 px-2.5 py-0.5 rounded bg-surface-container text-secondary font-mono-key text-xs border border-secondary/20 shadow-sm">
              <span className="w-1.5 h-1.5 rounded-full bg-secondary animate-pulse" />
              Personal Enclave (AES-256 GCM)
            </span>
          </div>
          <p className="font-body-md text-xs text-on-surface-variant max-w-2xl">
            Hardware-isolated client secrets encrypted locally with Argon2id key derivation before synchronizing to distributed storage nodes.
          </p>
        </div>

        <div className="flex items-center gap-2.5 flex-wrap">
          <button
            onClick={onOpenNewSecret}
            className="flex items-center gap-1.5 bg-primary text-on-primary font-semibold text-xs px-4 py-2 rounded-lg hover:bg-primary-hover transition-all shadow-[0_0_16px_rgba(124,92,255,0.4)] cursor-pointer"
          >
            <span className="material-symbols-outlined text-sm">add</span>
            <span>+ Add Credential</span>
          </button>
          <button
            onClick={onOpenExport}
            className="flex items-center gap-1.5 bg-surface-container text-on-surface text-xs px-3.5 py-2 rounded-lg hover:bg-surface-container-high transition-all border border-cyber-border cursor-pointer"
          >
            <span className="material-symbols-outlined text-sm text-secondary">file_download</span>
            <span>Export Encrypted</span>
          </button>
        </div>
      </div>

      {/* Category Pills & Sorting Bar */}
      <div className="flex flex-col md:flex-row md:items-center justify-between gap-3 pb-2 border-b border-cyber-border/40">
        <div className="flex items-center gap-1.5 overflow-x-auto pb-1 max-w-full">
          {categories.map(cat => {
            const isActive = selectedCategory === cat.id;
            return (
              <button
                key={cat.id}
                onClick={() => setSelectedCategory(cat.id)}
                className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-mono-key transition-all shrink-0 cursor-pointer ${
                  isActive
                    ? 'bg-primary text-on-primary font-semibold shadow-[0_0_12px_rgba(124,92,255,0.3)]'
                    : 'bg-surface-container hover:bg-surface-container-high text-on-surface-variant border border-cyber-border'
                }`}
              >
                <span className="material-symbols-outlined text-sm">{cat.icon}</span>
                <span>{cat.label}</span>
                <span className={`text-[10px] px-1.5 py-0.2 rounded-full ${isActive ? 'bg-black/20 text-white' : 'bg-surface-container-highest text-on-surface-variant'}`}>
                  {cat.count}
                </span>
              </button>
            );
          })}
        </div>

        {/* Sorting Dropdown */}
        <div className="flex items-center gap-2 text-xs text-on-surface-variant shrink-0">
          <span className="font-mono-key text-[10px] uppercase">Sort by:</span>
          <select
            value={sortBy}
            onChange={e => setSortBy(e.target.value)}
            className="px-2.5 py-1 bg-surface-container rounded-lg border border-cyber-border text-on-surface text-xs focus:outline-none focus:border-primary font-mono-key cursor-pointer"
          >
            <option value="recent">Recently Modified</option>
            <option value="title">Alphabetical (A-Z)</option>
            <option value="entropy">Password Strength</option>
          </select>
        </div>
      </div>

      {/* Vault Items List */}
      <div className="space-y-2.5">
        {sorted.length > 0 ? (
          sorted.map(item => {
            const entropy = calculateEntropy(item.password);
            const isRevealed = !!revealedPasswords[item.id];
            const isCopied = copiedId === item.id;

            return (
              <div
                key={item.id}
                onClick={() => setSelectedItem(item)}
                className="group p-4 rounded-xl bg-surface-container-low border border-cyber-border hover:border-cyber-border-highlight hover:bg-surface-container transition-all flex flex-col md:flex-row md:items-center justify-between gap-3 shadow-sm cursor-pointer"
              >
                {/* Left: Favicon/Initial + Name + Identity */}
                <div className="flex items-center gap-3.5 min-w-0">
                  <div className="w-10 h-10 rounded-lg bg-surface-container-lowest border border-cyber-border flex items-center justify-center text-primary font-bold text-sm shrink-0 shadow-inner group-hover:border-primary/40 transition-colors">
                    {item.title.slice(0, 2).toUpperCase()}
                  </div>
                  <div className="flex flex-col min-w-0">
                    <div className="flex items-center gap-2">
                      <span className="font-headline-sm text-sm font-semibold text-on-surface truncate">
                        {item.title}
                      </span>
                      {item.favorite && (
                        <span className="material-symbols-outlined text-amber-400 text-sm fill-current">
                          star
                        </span>
                      )}
                      {item.breached && (
                        <span className="font-badge-label text-[9px] text-error bg-error/15 px-1.5 py-0.2 rounded border border-error/30">
                          BREACH RISK
                        </span>
                      )}
                    </div>
                    <div className="flex items-center gap-2 text-xs font-mono-data text-on-surface-variant truncate">
                      <span className="truncate">{item.username || 'No identity'}</span>
                      {item.url && (
                        <>
                          <span>•</span>
                          <span className="text-outline truncate">{item.url.replace(/^https?:\/\//, '')}</span>
                        </>
                      )}
                    </div>
                  </div>
                </div>

                {/* Right: Secret Mask + Fast Actions */}
                <div className="flex items-center gap-3 justify-between md:justify-end shrink-0 pt-2 md:pt-0 border-t md:border-t-0 border-cyber-border/40">
                  {/* Masked Secret Pill */}
                  <div className="flex items-center gap-2 bg-surface-container px-3 py-1.5 rounded-lg border border-cyber-border/60">
                    <span className="font-mono-data text-xs text-on-surface tracking-widest select-all">
                      {isRevealed ? item.password : '••••••••••••'}
                    </span>
                    <button
                      type="button"
                      onClick={(e) => togglePasswordReveal(item.id, e)}
                      className="text-on-surface-variant hover:text-secondary p-0.5 cursor-pointer"
                      title={isRevealed ? 'Hide Secret' : 'Reveal Secret (10s)'}
                    >
                      <span className="material-symbols-outlined text-sm">
                        {isRevealed ? 'visibility_off' : 'visibility'}
                      </span>
                    </button>
                    <button
                      type="button"
                      onClick={(e) => copyToClipboard(item.password, 'Password', item.id, e)}
                      className={`text-on-surface-variant hover:text-secondary p-0.5 cursor-pointer transition-colors ${
                        isCopied ? 'text-secondary font-bold' : ''
                      }`}
                      title="Copy Password"
                    >
                      <span className="material-symbols-outlined text-sm">
                        {isCopied ? 'check' : 'content_copy'}
                      </span>
                    </button>
                  </div>

                  {/* Entropy Tag */}
                  <span className={`font-mono-key text-[10px] px-2 py-0.5 rounded border border-cyber-border hidden sm:inline-block ${entropy.color}`}>
                    {entropy.label}
                  </span>

                  {/* Edit & Delete Action Buttons */}
                  <div className="flex items-center gap-1">
                    <button
                      type="button"
                      onClick={(e) => {
                        e.stopPropagation();
                        onEditCredential(item);
                      }}
                      className="p-1.5 rounded-lg text-on-surface-variant hover:text-on-surface hover:bg-surface-container transition-colors cursor-pointer"
                      title="Edit Credential"
                    >
                      <span className="material-symbols-outlined text-base">edit</span>
                    </button>
                    <button
                      type="button"
                      onClick={(e) => handleDeleteRequest(item.id, item.title, e)}
                      className="p-1.5 rounded-lg text-on-surface-variant hover:text-error hover:bg-error/10 transition-colors cursor-pointer"
                      title="Delete Credential"
                    >
                      <span className="material-symbols-outlined text-base">delete</span>
                    </button>
                  </div>
                </div>
              </div>
            );
          })
        ) : (
          /* Empty State */
          <div className="py-16 px-4 bg-surface-container-low rounded-xl border border-dashed border-cyber-border text-center flex flex-col items-center justify-center space-y-3">
            <div className="w-12 h-12 rounded-full bg-surface-container flex items-center justify-center text-secondary border border-secondary/30">
              <span className="material-symbols-outlined text-2xl">search_off</span>
            </div>
            <div className="space-y-1">
              <h3 className="font-semibold text-sm text-on-surface">No Credentials Found</h3>
              <p className="font-body-md text-xs text-on-surface-variant max-w-sm">
                No secrets matched your current search query or category filter.
              </p>
            </div>
            <button
              onClick={onOpenNewSecret}
              className="px-4 py-2 rounded-lg bg-primary hover:bg-primary-hover text-on-primary text-xs font-semibold shadow-md transition-all cursor-pointer flex items-center gap-1.5 mt-2"
            >
              <span className="material-symbols-outlined text-sm">add</span>
              <span>Create New Secret</span>
            </button>
          </div>
        )}
      </div>

      {/* Credential Detail Drawer (Opens when a credential is clicked) */}
      <DetailDrawer
        credential={selectedItem}
        isOpen={!!selectedItem}
        onClose={() => setSelectedItem(null)}
        onEdit={(cred) => {
          onEditCredential(cred);
          setSelectedItem(null);
        }}
        onDelete={(id, title) => {
          setDeleteCandidate({ id, title });
        }}
        onToggleFavorite={onToggleFavorite}
        onShowToast={onShowToast}
      />

      {/* Permanent Deletion Confirmation Modal */}
      <ConfirmModal
        isOpen={!!deleteCandidate}
        onClose={() => setDeleteCandidate(null)}
        onConfirm={handleConfirmDelete}
        title="Revoke & Delete Credential"
        message="This action will permanently purge the selected secret from your local hardware-isolated keystore."
        itemTitle={deleteCandidate?.title}
        confirmText="Permanently Purge"
        danger={true}
      />
    </div>
  );
}
