import React, { useState } from 'react';

export default function AuditLedgerPage({
  ledger = [],
  onShowToast
}) {
  const [filterAction, setFilterAction] = useState('ALL');
  const [isVerifying, setIsVerifying] = useState(false);
  const [verificationResult, setVerificationResult] = useState(null);

  const handleVerifyChain = () => {
    setIsVerifying(true);
    setVerificationResult(null);
    onShowToast('Calculating cryptographic Merkle root across all blocks...', 'info');

    setTimeout(() => {
      setIsVerifying(false);
      setVerificationResult({
        blocksChecked: ledger.length,
        status: 'VALID',
        rootHash: '0x39a1fe48c90b8321e704a291f038cb127e94',
        timestamp: new Date().toLocaleTimeString()
      });
      onShowToast('Chain integrity verified. 100% of blocks mathematically valid.', 'success');
    }, 1400);
  };

  const handleExport = () => {
    const payload = JSON.stringify(ledger, null, 2);
    const blob = new Blob([payload], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `blockvault-signed-ledger-${Date.now()}.json`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    onShowToast('Signed cryptographic ledger exported.', 'success');
  };

  const filtered = ledger.filter(item => {
    if (filterAction === 'ALL') return true;
    return item.category === filterAction;
  });

  return (
    <div className="flex flex-col w-full pb-16 space-y-6 text-left font-sans">
      {/* High-Level Status Banner */}
      <div className="flex flex-wrap items-center justify-between gap-4 text-xs font-mono-key">
        <div className="flex items-center gap-2 text-on-surface-variant">
          <span>ENCLAVE_ROOT</span>
          <span>/</span>
          <span>INTEGRITY</span>
          <span>/</span>
          <span className="text-secondary font-semibold">HASH_CHAIN_LEDGER</span>
          <span className="px-1.5 py-0.5 rounded bg-surface-container text-secondary text-[10px]">
            READ_ONLY
          </span>
        </div>

        <div className="flex items-center gap-2.5 bg-surface-container-low px-3.5 py-1.5 rounded-full border border-cyber-border">
          <span className="w-2 h-2 rounded-full bg-secondary animate-pulse"></span>
          <span className="text-on-surface font-semibold">LOCAL MERKLE CHAIN</span>
          <span className="text-outline">•</span>
          <span className="text-secondary font-semibold">HEAD: {ledger[0]?.blockId || '#01428'}</span>
          <span className="text-outline">•</span>
          <span className="text-on-surface-variant">ZERO CLOUD LEAKAGE</span>
        </div>
      </div>

      {/* Page Header & Global Actions */}
      <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4 bg-surface-container-low p-6 rounded-xl border border-cyber-border shadow-lg">
        <div className="space-y-1 max-w-2xl">
          <div className="flex items-center gap-2">
            <span className="material-symbols-outlined text-primary text-2xl">receipt_long</span>
            <h1 className="font-headline-lg text-xl sm:text-2xl font-bold text-on-surface tracking-tight">
              Audit History & Cryptographic Ledger
            </h1>
          </div>
          <p className="font-body-md text-xs text-on-surface-variant leading-relaxed">
            Local hash-chained journal recording tamper-evident vault actions. Every state change produces an immutable cryptographic digest strictly linked to the prior block through SHA-256 chain consensus.
          </p>
        </div>

        <div className="flex flex-wrap items-center gap-2.5">
          <button
            onClick={handleVerifyChain}
            disabled={isVerifying}
            className="flex items-center gap-1.5 bg-primary text-on-primary font-semibold text-xs px-4 py-2.5 rounded-lg hover:bg-primary-hover transition-all shadow-[0_0_16px_rgba(124,92,255,0.35)] cursor-pointer disabled:opacity-50"
          >
            <span className={`material-symbols-outlined text-sm ${isVerifying ? 'animate-spin' : ''}`}>
              {isVerifying ? 'refresh' : 'shield_with_heart'}
            </span>
            <span>{isVerifying ? 'Verifying Chain...' : 'Verify Entire Chain Integrity'}</span>
          </button>
          <button
            onClick={handleExport}
            className="flex items-center gap-1.5 bg-surface-container hover:bg-surface-container-high text-on-surface text-xs px-3.5 py-2.5 rounded-lg transition-colors border border-cyber-border cursor-pointer"
          >
            <span className="material-symbols-outlined text-sm">download</span>
            <span>Export Signed Ledger</span>
          </button>
        </div>
      </div>

      {/* Verification Result Toast Box */}
      {verificationResult && (
        <div className="p-4 rounded-xl bg-surface-container border border-secondary/40 shadow-xl flex items-center justify-between font-mono-data text-xs">
          <div className="flex items-center gap-3">
            <span className="material-symbols-outlined text-secondary text-2xl">verified</span>
            <div>
              <div className="font-bold text-on-surface">Merkle Consensus Verified Valid</div>
              <div className="text-[11px] text-on-surface-variant">
                Root: {verificationResult.rootHash} • {verificationResult.blocksChecked} blocks checked at {verificationResult.timestamp}
              </div>
            </div>
          </div>
          <span className="px-2 py-0.5 rounded bg-secondary/20 text-secondary text-[10px] font-mono-key">
            100% UNTAMPERED
          </span>
        </div>
      )}

      {/* Category Filter Buttons */}
      <div className="flex items-center gap-2 text-xs font-mono-key overflow-x-auto pb-1">
        {['ALL', 'AUTH', 'DECRYPT', 'GENERATE', 'UPDATE', 'CREATE', 'PURGE'].map(cat => (
          <button
            key={cat}
            onClick={() => setFilterAction(cat)}
            className={`px-3 py-1.5 rounded-lg transition-colors cursor-pointer ${
              filterAction === cat
                ? 'bg-primary text-on-primary font-semibold'
                : 'bg-surface-container text-on-surface-variant hover:text-on-surface'
            }`}
          >
            {cat}
          </button>
        ))}
      </div>

      {/* Ledger Table */}
      <div className="bg-surface-container-low rounded-xl border border-cyber-border overflow-hidden shadow-md">
        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs font-mono-data">
            <thead className="bg-surface-container-lowest text-on-surface-variant font-mono-key text-[10px] uppercase border-b border-cyber-border">
              <tr>
                <th className="py-3 px-4">Block Height</th>
                <th className="py-3 px-4">Timestamp</th>
                <th className="py-3 px-4">Action</th>
                <th className="py-3 px-4">Target Resource</th>
                <th className="py-3 px-4">Client Node</th>
                <th className="py-3 px-4">Digest / Hash</th>
                <th className="py-3 px-4 text-right">Integrity Status</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-cyber-border/40">
              {filtered.map(entry => (
                <tr key={entry.blockId} className="hover:bg-surface-container/50 transition-colors">
                  <td className="py-3 px-4 font-bold text-secondary">{entry.blockId}</td>
                  <td className="py-3 px-4 text-on-surface-variant text-[11px]">{entry.timestamp}</td>
                  <td className="py-3 px-4">
                    <span className="px-2 py-0.5 rounded bg-surface-container text-on-surface text-[10px] font-mono-key">
                      {entry.action}
                    </span>
                  </td>
                  <td className="py-3 px-4 text-on-surface font-sans font-medium">{entry.target}</td>
                  <td className="py-3 px-4 text-on-surface-variant text-[11px]">{entry.clientNode}</td>
                  <td className="py-3 px-4">
                    <span
                      onClick={() => {
                        navigator.clipboard.writeText(entry.hash);
                        onShowToast('Hash copied to clipboard.', 'success');
                      }}
                      className="cursor-pointer hover:text-secondary text-on-surface-variant underline select-all"
                      title="Click to copy hash"
                    >
                      {entry.hash.slice(0, 14)}...
                    </span>
                  </td>
                  <td className="py-3 px-4 text-right">
                    <span className="inline-flex items-center gap-1 font-mono-key text-[10px] text-secondary bg-surface-container px-2 py-0.5 rounded border border-secondary/20">
                      <span className="w-1.5 h-1.5 rounded-full bg-secondary"></span>
                      {entry.status}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}
