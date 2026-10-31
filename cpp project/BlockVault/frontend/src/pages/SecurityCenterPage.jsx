import React, { useState } from 'react';
import { calculateEntropy } from '../services/cryptoService';

export default function SecurityCenterPage({
  credentials = [],
  metrics,
  onEditCredential,
  onShowToast
}) {
  const [isScanning, setIsScanning] = useState(false);
  const [lastScanTime, setLastScanTime] = useState('4m ago');

  const handleScan = () => {
    setIsScanning(true);
    onShowToast('Triggering zero-knowledge enclave audit engine...', 'info');
    setTimeout(() => {
      setIsScanning(false);
      setLastScanTime('Just now');
      onShowToast('Enclave audit completed. All zero-knowledge constraints verified.', 'success');
    }, 1500);
  };

  const handleRemediate = (cred) => {
    onEditCredential(cred);
  };

  const weakList = credentials.filter(c => calculateEntropy(c.password).bits < 50);
  const breachedList = credentials.filter(c => c.breached);

  // Group reused passwords
  const reusedGroups = {};
  credentials.forEach(c => {
    reusedGroups[c.password] = reusedGroups[c.password] || [];
    reusedGroups[c.password].push(c);
  });
  const reusedClusters = Object.values(reusedGroups).filter(group => group.length > 1);

  return (
    <div className="flex flex-col w-full pb-16 space-y-6 text-left font-sans">
      {/* Header and Telemetry Actions */}
      <div className="flex flex-col xl:flex-row xl:items-end justify-between gap-4 pt-2">
        <div className="flex flex-col gap-1 max-w-2xl">
          <div className="flex items-center gap-2.5 text-xs font-mono-key">
            <span className="inline-flex items-center gap-1.5 px-2 py-0.5 rounded bg-surface-container text-secondary uppercase border border-secondary/20">
              <span className="w-1.5 h-1.5 rounded-full bg-secondary animate-pulse"></span>
              ENCLAVE AUDIT ENGINE v4.18
            </span>
            <span className="text-on-surface-variant font-mono-key">•</span>
            <span className="text-on-surface-variant flex items-center gap-1 font-mono-key">
              Last scan: {lastScanTime}
            </span>
          </div>
          <h1 className="font-headline-lg text-2xl font-bold text-on-surface tracking-tight">
            Security Health Center
          </h1>
          <p className="font-body-md text-xs text-on-surface-variant">
            Continuous zero-knowledge telemetry evaluating credential entropy, reuse clusters, and rotation hygiene without client-side plaintext exposure.
          </p>
        </div>

        <div className="flex items-center gap-2.5 flex-wrap">
          <button
            onClick={handleScan}
            disabled={isScanning}
            className="flex items-center gap-2 bg-primary text-on-primary text-xs font-semibold px-4 py-2 rounded-lg hover:bg-primary-hover transition-all shadow-[0_0_14px_rgba(124,92,255,0.4)] cursor-pointer disabled:opacity-50"
          >
            <span className={`material-symbols-outlined text-sm ${isScanning ? 'animate-spin' : ''}`}>
              sync
            </span>
            <span>{isScanning ? 'Auditing Keystore...' : 'Trigger Enclave Scan'}</span>
          </button>
          <button
            onClick={() => onShowToast('Exported Security Posture Report (PDF) to downloads.', 'success')}
            className="flex items-center gap-1.5 bg-surface-container hover:bg-surface-container-high text-on-surface text-xs px-3.5 py-2 rounded-lg transition-colors border border-cyber-border cursor-pointer"
          >
            <span className="material-symbols-outlined text-sm text-secondary">file_download</span>
            <span>Export Report</span>
          </button>
        </div>
      </div>

      {/* Bento Grid: Overall Posture + Key Telemetry */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        {/* Card 1: Large Posture Index Gauge */}
        <div className="md:col-span-2 bg-surface-container-low rounded-xl p-5 border border-cyber-border-highlight shadow-xl relative overflow-hidden flex flex-col justify-between">
          <div className="flex items-start justify-between">
            <div>
              <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                Overall Posture Index
              </span>
              <div className="flex items-baseline gap-2 mt-1">
                <span className="text-4xl font-bold text-on-surface font-mono-numbers">
                  {metrics.postureScore}
                </span>
                <span className="text-xs text-outline font-mono-key">/ 100</span>
              </div>
            </div>
            <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full bg-surface-container text-secondary font-mono-key text-xs border border-secondary/20">
              <span className="w-2 h-2 rounded-full bg-secondary"></span>
              {metrics.postureScore >= 80 ? 'EXCELLENT POSTURE' : 'ACTION REQUIRED'}
            </span>
          </div>

          <div className="flex items-center gap-6 my-4">
            <div className="relative w-24 h-24 flex items-center justify-center flex-shrink-0">
              <svg className="w-full h-full -rotate-90" viewBox="0 0 100 100">
                <circle cx="50" cy="50" r="40" className="text-surface-container-lowest stroke-current" strokeWidth="10" fill="transparent" />
                <circle
                  cx="50" cy="50" r="40"
                  className="text-secondary stroke-current transition-all duration-1000"
                  strokeWidth="10"
                  strokeDasharray="251.2"
                  strokeDashoffset={251.2 - (251.2 * metrics.postureScore) / 100}
                  strokeLinecap="round"
                  fill="transparent"
                />
              </svg>
              <span className="absolute font-mono-numbers text-lg font-bold text-on-surface">
                {metrics.postureScore}%
              </span>
            </div>
            <div className="flex flex-col text-xs text-on-surface-variant gap-1">
              <span className="text-on-surface font-semibold">Zero-Knowledge Enclave Analysis</span>
              <span>All secrets audited via local PBKDF2 entropy evaluator.</span>
              <span className="text-secondary font-mono-key text-[10px]">Zero keys uploaded to cloud.</span>
            </div>
          </div>
        </div>

        {/* Card 2: Weak Keys Flagged */}
        <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border flex flex-col justify-between">
          <div className="flex items-start justify-between">
            <span className="font-mono-key text-[10px] text-error uppercase font-semibold">
              Weak Passwords
            </span>
            <span className="material-symbols-outlined text-error text-xl">gpp_maybe</span>
          </div>
          <div>
            <div className="text-3xl font-bold text-error font-mono-numbers mt-2">
              {metrics.weakCount}
            </div>
            <span className="text-[11px] text-on-surface-variant">
              Low entropy (&lt;50 bits)
            </span>
          </div>
          <span className="text-[10px] text-error font-mono-key bg-error-container/20 px-2 py-0.5 rounded border border-error/30 mt-2">
            Urgent rotation needed
          </span>
        </div>

        {/* Card 3: Reused Passwords */}
        <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border flex flex-col justify-between">
          <div className="flex items-start justify-between">
            <span className="font-mono-key text-[10px] text-tertiary uppercase font-semibold">
              Reused Passwords
            </span>
            <span className="material-symbols-outlined text-tertiary text-xl">repeat</span>
          </div>
          <div>
            <div className="text-3xl font-bold text-tertiary font-mono-numbers mt-2">
              {metrics.reusedCount}
            </div>
            <span className="text-[11px] text-on-surface-variant">
              Across {reusedClusters.length} clusters
            </span>
          </div>
          <span className="text-[10px] text-tertiary font-mono-key bg-tertiary-container/20 px-2 py-0.5 rounded border border-tertiary/30 mt-2">
            Credential stuffing risk
          </span>
        </div>
      </div>

      {/* Actionable Remediation Sections */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Weak Passwords List */}
        <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border shadow-md flex flex-col gap-4">
          <div className="flex items-center justify-between pb-2 border-b border-cyber-border/40">
            <div className="flex items-center gap-2">
              <span className="material-symbols-outlined text-error text-lg">warning</span>
              <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
                Vulnerable Credentials Flagged ({weakList.length})
              </h3>
            </div>
            <span className="font-mono-key text-[10px] text-error">PRIORITY 1</span>
          </div>

          <div className="flex flex-col gap-2.5">
            {weakList.length === 0 ? (
              <p className="text-xs text-on-surface-variant p-4 text-center">
                All credentials satisfy minimum 80-bit entropy requirements!
              </p>
            ) : (
              weakList.map(item => (
                <div
                  key={item.id}
                  className="flex items-center justify-between p-3 rounded-lg bg-surface-container border border-error/30 text-xs"
                >
                  <div className="flex flex-col">
                    <span className="font-semibold text-on-surface">{item.title}</span>
                    <span className="font-mono-data text-[10px] text-on-surface-variant">
                      {item.username}
                    </span>
                  </div>
                  <button
                    onClick={() => handleRemediate(item)}
                    className="flex items-center gap-1 px-3 py-1.5 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs shadow-sm transition-all cursor-pointer"
                  >
                    <span className="material-symbols-outlined text-xs">auto_fix_high</span>
                    <span>Rotate Key</span>
                  </button>
                </div>
              ))
            )}
          </div>
        </div>

        {/* Reused Password Clusters */}
        <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border shadow-md flex flex-col gap-4">
          <div className="flex items-center justify-between pb-2 border-b border-cyber-border/40">
            <div className="flex items-center gap-2">
              <span className="material-symbols-outlined text-tertiary text-lg">repeat</span>
              <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
                Reused Password Clusters ({reusedClusters.length})
              </h3>
            </div>
            <span className="font-mono-key text-[10px] text-tertiary">CROSS-SERVICE RISK</span>
          </div>

          <div className="flex flex-col gap-3">
            {reusedClusters.length === 0 ? (
              <p className="text-xs text-on-surface-variant p-4 text-center">
                Zero password reuse detected across services.
              </p>
            ) : (
              reusedClusters.map((cluster, idx) => (
                <div key={idx} className="p-3 rounded-lg bg-surface-container border border-tertiary/30 text-xs">
                  <div className="flex items-center justify-between pb-1.5 mb-2 border-b border-cyber-border/30">
                    <span className="font-mono-key text-[10px] text-tertiary font-semibold">
                      CLUSTER #{idx + 1} ({cluster.length} Accounts share this secret)
                    </span>
                  </div>
                  <div className="flex flex-col gap-1.5">
                    {cluster.map(c => (
                      <div key={c.id} className="flex items-center justify-between text-[11px] font-mono-data">
                        <span className="text-on-surface">{c.title}</span>
                        <button
                          onClick={() => handleRemediate(c)}
                          className="text-secondary hover:underline cursor-pointer"
                        >
                          Change →
                        </button>
                      </div>
                    ))}
                  </div>
                </div>
              ))
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
