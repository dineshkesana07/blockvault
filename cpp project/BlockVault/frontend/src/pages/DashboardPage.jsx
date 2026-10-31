import React, { useState } from 'react';

export default function DashboardPage({
  credentials = [],
  metrics,
  onOpenNewSecret,
  onOpenExport,
  onNavigate,
  onSelectCredential,
  onShowToast
}) {
  const [isScanning, setIsScanning] = useState(false);

  const handleRunScan = () => {
    setIsScanning(true);
    onShowToast('Initiating zero-knowledge enclave vulnerability scan...', 'info');
    setTimeout(() => {
      setIsScanning(false);
      onShowToast('Scan complete. 0 critical zero-day exposures detected. 6 weak passwords flagged.', 'success');
    }, 1800);
  };

  const handleQuickCopy = (e, text, label) => {
    e.stopPropagation();
    navigator.clipboard.writeText(text);
    onShowToast(`Copied ${label} to clipboard.`, 'success');
  };

  const favorites = credentials.filter(c => c.favorite);

  return (
    <div className="flex flex-col w-full pb-16 space-y-6 text-left">
      {/* Telemetry Header Banner */}
      <div className="relative overflow-hidden bg-surface-container rounded-xl p-6 shadow-xl border border-cyber-border">
        <div className="absolute -right-16 -top-16 w-80 h-80 rounded-full bg-primary/10 blur-3xl pointer-events-none"></div>
        <div className="absolute right-64 -bottom-10 w-64 h-64 rounded-full bg-secondary/5 blur-2xl pointer-events-none"></div>

        <div className="relative z-10 flex flex-col lg:flex-row lg:items-center justify-between gap-4">
          <div className="flex flex-col gap-1.5">
            <div className="flex items-center gap-2 flex-wrap text-[11px] font-mono-key">
              <span className="text-secondary bg-surface-container-lowest px-2 py-0.5 rounded border border-secondary/20">
                ENCLAVE: v4.8.2-PROD
              </span>
              <span className="text-on-surface-variant flex items-center gap-1">
                <span className="w-1.5 h-1.5 rounded-full bg-secondary animate-ping"></span>
                LATENCY: 14ms
              </span>
              <span className="text-secondary bg-surface-container-lowest px-2 py-0.5 rounded border border-secondary/20">
                HARDWARE BIOMETRIC: ACTIVE
              </span>
              <span className="text-outline">ZERO-KNOWLEDGE VERIFIED</span>
            </div>
            <h1 className="font-headline-lg text-2xl font-bold text-on-surface tracking-tight mt-1">
              Welcome back, Dr. Elena Vance — Enclave Sealed
            </h1>
            <p className="font-body-md text-xs text-on-surface-variant">
              Hardware-isolated cryptographic keyspace protecting {credentials.length} credentials across multi-region services.
            </p>
          </div>

          <div className="flex items-center flex-wrap gap-2.5">
            <button
              onClick={onOpenExport}
              className="flex items-center gap-1.5 bg-surface-container-high hover:bg-surface-bright text-on-surface text-xs px-3.5 py-2 rounded-lg transition-all border border-cyber-border cursor-pointer"
            >
              <span className="material-symbols-outlined text-sm text-on-surface-variant">file_download</span>
              <span>Export Backup</span>
            </button>
            <button
              onClick={handleRunScan}
              disabled={isScanning}
              className="flex items-center gap-1.5 bg-surface-container-high hover:bg-surface-bright text-secondary text-xs px-3.5 py-2 rounded-lg transition-all border border-secondary/30 cursor-pointer disabled:opacity-50"
            >
              <span className={`material-symbols-outlined text-sm ${isScanning ? 'animate-spin' : ''}`}>
                {isScanning ? 'refresh' : 'radar'}
              </span>
              <span>{isScanning ? 'Scanning Keystore...' : 'Run Vulnerability Scan'}</span>
            </button>
            <button
              onClick={onOpenNewSecret}
              className="flex items-center gap-1.5 bg-primary hover:bg-primary-hover text-on-primary text-xs font-semibold px-4 py-2 rounded-lg transition-all shadow-[0_0_16px_rgba(124,92,255,0.35)] cursor-pointer"
            >
              <span className="material-symbols-outlined text-sm">add</span>
              <span>+ Add Credential</span>
            </button>
          </div>
        </div>
      </div>

      {/* Key Metric Stat Cards (4-Column Grid) */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        {/* Card 1: Total Credentials */}
        <div 
          onClick={() => onNavigate('password-vault')}
          className="group bg-surface-container hover:bg-surface-container-high transition-colors p-4 rounded-xl flex flex-col justify-between shadow-md border border-cyber-border cursor-pointer"
        >
          <div className="flex items-start justify-between mb-2">
            <div className="flex flex-col">
              <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                Total Credentials
              </span>
              <span className="text-3xl font-bold text-on-surface font-mono-numbers mt-1 tracking-tight">
                {metrics.total}
              </span>
            </div>
            <div className="w-9 h-9 rounded-lg bg-surface-container-lowest flex items-center justify-center text-primary border border-primary/20">
              <span className="material-symbols-outlined text-lg">token</span>
            </div>
          </div>
          <div className="flex flex-col gap-1 mt-2">
            <div className="flex items-center gap-1.5 text-xs">
              <span className="font-mono-key text-[10px] text-secondary bg-surface-container-lowest px-1.5 py-0.5 rounded font-semibold">
                +8 this month
              </span>
              <span className="text-outline text-[11px]">Synchronized</span>
            </div>
            <span className="font-mono-data text-on-surface-variant text-[10px]">
              Active AES-256 GCM client keystore
            </span>
          </div>
        </div>

        {/* Card 2: Strong Passwords */}
        <div 
          onClick={() => onNavigate('security-center')}
          className="group bg-surface-container hover:bg-surface-container-high transition-colors p-4 rounded-xl flex flex-col justify-between shadow-md border border-cyber-border cursor-pointer"
        >
          <div className="flex items-start justify-between mb-2">
            <div className="flex flex-col">
              <span className="font-mono-key text-[10px] uppercase text-on-surface-variant tracking-wider">
                Strong Passwords
              </span>
              <div className="flex items-baseline gap-2 mt-1">
                <span className="text-3xl font-bold text-on-surface font-mono-numbers tracking-tight">
                  {metrics.strongCount}
                </span>
                <span className="font-mono-key text-xs text-secondary bg-surface-container-lowest px-1.5 py-0.5 rounded font-semibold">
                  {metrics.strongPercent}%
                </span>
              </div>
            </div>
            <div className="w-9 h-9 rounded-lg bg-surface-container-lowest flex items-center justify-center text-secondary border border-secondary/20">
              <span className="material-symbols-outlined text-lg">verified_user</span>
            </div>
          </div>
          <div className="flex flex-col gap-1 mt-2">
            <div className="w-full bg-surface-container-lowest h-1.5 rounded-full overflow-hidden">
              <div
                className="bg-secondary h-full rounded-full transition-all duration-500"
                style={{ width: `${metrics.strongPercent}%` }}
              ></div>
            </div>
            <span className="font-mono-data text-on-surface-variant text-[10px]">
              Entropy &gt; 80 bits (Argon2id + AES)
            </span>
          </div>
        </div>

        {/* Card 3: Weak Passwords */}
        <div 
          onClick={() => onNavigate('security-center')}
          className="group bg-surface-container hover:bg-surface-container-high transition-colors p-4 rounded-xl flex flex-col justify-between shadow-md border border-cyber-border cursor-pointer"
        >
          <div className="flex items-start justify-between mb-2">
            <div className="flex flex-col">
              <span className="font-mono-key text-[10px] uppercase text-error tracking-wider font-semibold">
                Weak Passwords
              </span>
              <div className="flex items-baseline gap-2 mt-1">
                <span className="text-3xl font-bold text-error font-mono-numbers tracking-tight">
                  {metrics.weakCount}
                </span>
                <span className="font-mono-key text-xs text-error bg-error-container/30 px-1.5 py-0.5 rounded font-semibold">
                  {metrics.weakPercent}%
                </span>
              </div>
            </div>
            <div className="w-9 h-9 rounded-lg bg-surface-container-lowest flex items-center justify-center text-error border border-error/20">
              <span className="material-symbols-outlined text-lg">gpp_maybe</span>
            </div>
          </div>
          <div className="flex flex-col gap-1 mt-2">
            <div className="w-full bg-surface-container-lowest h-1.5 rounded-full overflow-hidden">
              <div
                className="bg-error h-full rounded-full transition-all duration-500"
                style={{ width: `${metrics.weakPercent * 3}%` }}
              ></div>
            </div>
            <span className="font-mono-data text-error text-[10px] flex items-center gap-1 font-semibold">
              <span className="material-symbols-outlined text-xs">warning</span> Requires key rotation
            </span>
          </div>
        </div>

        {/* Card 4: Reused Passwords */}
        <div 
          onClick={() => onNavigate('security-center')}
          className="group bg-surface-container hover:bg-surface-container-high transition-colors p-4 rounded-xl flex flex-col justify-between shadow-md border border-cyber-border cursor-pointer"
        >
          <div className="flex items-start justify-between mb-2">
            <div className="flex flex-col">
              <span className="font-mono-key text-[10px] uppercase text-tertiary tracking-wider font-semibold">
                Reused Passwords
              </span>
              <div className="flex items-baseline gap-2 mt-1">
                <span className="text-3xl font-bold text-tertiary font-mono-numbers tracking-tight">
                  {metrics.reusedCount}
                </span>
                <span className="font-mono-key text-xs text-tertiary bg-tertiary-container/30 px-1.5 py-0.5 rounded font-semibold">
                  {metrics.reusedPercent}%
                </span>
              </div>
            </div>
            <div className="w-9 h-9 rounded-lg bg-surface-container-lowest flex items-center justify-center text-tertiary border border-tertiary/20">
              <span className="material-symbols-outlined text-lg">repeat</span>
            </div>
          </div>
          <div className="flex flex-col gap-1 mt-2">
            <div className="w-full bg-surface-container-lowest h-1.5 rounded-full overflow-hidden">
              <div
                className="bg-tertiary h-full rounded-full transition-all duration-500"
                style={{ width: `${metrics.reusedPercent * 2}%` }}
              ></div>
            </div>
            <span className="font-mono-data text-tertiary text-[10px]">
              Cross-service exposure risk
            </span>
          </div>
        </div>
      </div>

      {/* Two Column Layout: Posture & Favorites */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Column (2 Cols): Quick Access Vault & Telemetry */}
        <div className="lg:col-span-2 flex flex-col gap-6">
          {/* Favorite Credentials Card */}
          <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border shadow-md">
            <div className="flex items-center justify-between pb-3 mb-3 border-b border-cyber-border/40">
              <div className="flex items-center gap-2">
                <span className="material-symbols-outlined text-primary text-xl">star</span>
                <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
                  Priority Enclave Credentials
                </h3>
              </div>
              <button
                onClick={() => onNavigate('password-vault')}
                className="text-xs text-secondary hover:underline font-mono-key cursor-pointer"
              >
                View Full Vault ({credentials.length}) →
              </button>
            </div>

            <div className="flex flex-col gap-2">
              {favorites.slice(0, 4).map(item => (
                <div
                  key={item.id}
                  onClick={() => onSelectCredential(item)}
                  className="flex items-center justify-between p-3 rounded-lg bg-surface-container/70 hover:bg-surface-container border border-cyber-border/40 transition-colors cursor-pointer group"
                >
                  <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-surface-container-lowest flex items-center justify-center text-secondary border border-cyber-border">
                      <span className="material-symbols-outlined text-base">
                        {item.category === 'keys' ? 'vpn_key' : item.category === 'ssh' ? 'terminal' : 'shield'}
                      </span>
                    </div>
                    <div className="flex flex-col">
                      <span className="text-xs font-semibold text-on-surface group-hover:text-primary transition-colors">
                        {item.title}
                      </span>
                      <span className="font-mono-data text-[10px] text-on-surface-variant">
                        {item.username}
                      </span>
                    </div>
                  </div>

                  <div className="flex items-center gap-2">
                    {item.tags.slice(0, 2).map((tag, idx) => (
                      <span
                        key={idx}
                        className="hidden sm:inline-block font-mono-key text-[10px] text-on-surface-variant bg-surface-container-lowest px-2 py-0.5 rounded border border-cyber-border/30"
                      >
                        {tag}
                      </span>
                    ))}
                    <button
                      onClick={(e) => handleQuickCopy(e, item.username, 'Username')}
                      className="p-1.5 rounded hover:bg-surface-container-high text-on-surface-variant hover:text-secondary text-xs cursor-pointer"
                      title="Copy Username"
                    >
                      <span className="material-symbols-outlined text-sm">person</span>
                    </button>
                    <button
                      onClick={(e) => handleQuickCopy(e, item.password, 'Password')}
                      className="p-1.5 rounded hover:bg-surface-container-high text-on-surface-variant hover:text-secondary text-xs cursor-pointer"
                      title="Copy Password"
                    >
                      <span className="material-symbols-outlined text-sm">content_copy</span>
                    </button>
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* Quick Enclave Synthesizer Card */}
          <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border shadow-md flex items-center justify-between">
            <div className="flex items-center gap-4">
              <div className="w-12 h-12 rounded-xl bg-surface-container flex items-center justify-center text-secondary border border-secondary/20 shadow-inner">
                <span className="material-symbols-outlined text-2xl">casino</span>
              </div>
              <div className="flex flex-col">
                <h4 className="font-headline-sm text-sm font-semibold text-on-surface">
                  Need a Quantum-Resistant Key?
                </h4>
                <p className="text-xs text-on-surface-variant">
                  Generate 256-bit entropy passphrases with zero cloud leakage.
                </p>
              </div>
            </div>
            <button
              onClick={() => onNavigate('password-generator')}
              className="px-4 py-2 rounded-lg bg-surface-container-high hover:bg-surface-bright text-secondary font-semibold text-xs border border-secondary/30 transition-all cursor-pointer"
            >
              Open Synthesizer →
            </button>
          </div>
        </div>

        {/* Right Column (1 Col): Security Posture & Hardware Status */}
        <div className="flex flex-col gap-6">
          {/* Posture Gauge Card */}
          <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border shadow-md flex flex-col justify-between">
            <div>
              <div className="flex items-center justify-between pb-3 mb-4 border-b border-cyber-border/40">
                <span className="font-mono-key text-[10px] uppercase text-on-surface-variant font-semibold tracking-wider">
                  Posture Index
                </span>
                <span className="font-badge-label text-[10px] text-secondary bg-surface-container px-2 py-0.5 rounded border border-secondary/20">
                  OPTIMAL
                </span>
              </div>

              {/* Gauge visual */}
              <div className="flex flex-col items-center justify-center py-4">
                <div className="relative w-36 h-36 flex items-center justify-center">
                  <svg className="w-full h-full -rotate-90" viewBox="0 0 100 100">
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      className="text-surface-container-lowest stroke-current"
                      strokeWidth="10"
                      fill="transparent"
                    />
                    <circle
                      cx="50"
                      cy="50"
                      r="40"
                      className="text-secondary stroke-current transition-all duration-1000 ease-out"
                      strokeWidth="10"
                      strokeDasharray="251.2"
                      strokeDashoffset={251.2 - (251.2 * metrics.postureScore) / 100}
                      strokeLinecap="round"
                      fill="transparent"
                    />
                  </svg>
                  <div className="absolute flex flex-col items-center">
                    <span className="text-3xl font-bold text-on-surface font-mono-numbers">
                      {metrics.postureScore}
                    </span>
                    <span className="font-mono-key text-[10px] text-on-surface-variant">
                      / 100
                    </span>
                  </div>
                </div>

                <div className="text-center mt-3">
                  <p className="text-xs text-on-surface font-medium">
                    Zero-Knowledge Enclave Verified
                  </p>
                  <p className="text-[11px] text-on-surface-variant font-mono-data mt-0.5">
                    Continuous local audit active
                  </p>
                </div>
              </div>
            </div>

            <button
              onClick={() => onNavigate('security-center')}
              className="w-full py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-xs text-on-surface font-medium border border-cyber-border transition-colors cursor-pointer mt-2"
            >
              Analyze Detailed Health Report →
            </button>
          </div>

          {/* Enclave Verification Spec */}
          <div className="bg-surface-container-low rounded-xl p-4 border border-cyber-border text-xs font-mono-data text-on-surface-variant flex flex-col gap-2">
            <div className="flex items-center justify-between text-[11px] font-mono-key pb-2 border-b border-cyber-border/30 text-on-surface">
              <span>HARDWARE ENCLAVE</span>
              <span className="text-secondary">ARM TrustZone</span>
            </div>
            <div className="flex items-center justify-between text-[11px]">
              <span>Key Derivation:</span>
              <span className="text-on-surface">Argon2id (m=64M, t=4)</span>
            </div>
            <div className="flex items-center justify-between text-[11px]">
              <span>Cipher Spec:</span>
              <span className="text-on-surface">AES-256-GCM / Poly1305</span>
            </div>
            <div className="flex items-center justify-between text-[11px]">
              <span>Merkle Height:</span>
              <span className="text-secondary font-mono-numbers">#01428</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
