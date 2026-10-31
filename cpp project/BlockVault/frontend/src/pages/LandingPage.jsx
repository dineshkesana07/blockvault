import React, { useState } from 'react';

export default function LandingPage({ onLaunchVault, onOpenUnlock }) {
  const [copiedCli, setCopiedCli] = useState(false);

  const handleCopyCli = () => {
    navigator.clipboard.writeText('npx @blockvault/cli init');
    setCopiedCli(true);
    setTimeout(() => setCopiedCli(false), 2000);
  };

  return (
    <div className="w-full min-h-screen bg-background text-on-surface antialiased flex flex-col font-sans select-none">
      {/* Navigation Header */}
      <header className="fixed top-0 left-0 w-full z-50 bg-surface-container-lowest/85 backdrop-blur-xl border-b border-cyber-border/40 shadow-[0_1px_8px_rgba(0,0,0,0.4)]">
        <div className="h-20 max-w-7xl mx-auto px-6 flex items-center justify-between">
          <div className="flex items-center gap-3 cursor-pointer" onClick={onLaunchVault}>
            <div className="w-8 h-8 rounded-lg bg-surface-container flex items-center justify-center p-1 border border-primary/40 shadow-[0_0_12px_rgba(124,92,255,0.3)]">
              <span className="material-symbols-outlined text-primary text-xl">enhanced_encryption</span>
            </div>
            <span className="font-headline-sm text-base uppercase tracking-tight text-on-surface font-bold">
              BlockVault
            </span>
            <span className="font-mono-key text-[10px] px-2 py-0.5 rounded bg-surface-container text-secondary uppercase border border-secondary/20">
              v3.4-ZKP
            </span>
          </div>

          <nav className="hidden md:flex items-center gap-8 text-sm">
            <a href="#features" className="text-on-surface-variant hover:text-on-surface transition-colors">
              Features
            </a>
            <a href="#architecture" className="text-on-surface-variant hover:text-on-surface transition-colors">
              Zero-Trust Architecture
            </a>
            <a href="#telemetry" className="text-on-surface-variant hover:text-on-surface transition-colors">
              Telemetry
            </a>
            <a href="#pricing" className="text-on-surface-variant hover:text-on-surface transition-colors">
              Enterprise
            </a>
          </nav>

          <div className="flex items-center gap-4">
            <button
              onClick={onOpenUnlock}
              className="font-body-sm text-xs text-on-surface hover:text-primary transition-colors px-3 py-2 cursor-pointer"
            >
              Unlock Vault
            </button>
            <button
              onClick={onLaunchVault}
              className="font-body-sm text-xs font-semibold bg-primary text-on-primary hover:bg-primary-hover px-4 py-2 rounded-lg shadow-[0_0_14px_rgba(124,92,255,0.4)] transition-all cursor-pointer"
            >
              Launch Enclave
            </button>
          </div>
        </div>
      </header>

      {/* Main Content Container */}
      <main className="w-full pt-20 bg-background flex-1">
        {/* Hero Section */}
        <div className="relative w-full overflow-hidden ambient-mesh">
          <div className="absolute -top-40 left-1/2 -translate-x-1/2 w-[760px] h-[380px] bg-primary/10 rounded-full blur-[120px] pointer-events-none"></div>
          <div className="absolute top-64 right-1/4 w-[420px] h-[240px] bg-secondary/5 rounded-full blur-[100px] pointer-events-none"></div>

          <section className="relative max-w-7xl mx-auto px-6 pt-20 pb-24 flex flex-col items-center text-center">
            {/* Overline Tag */}
            <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-surface-container border border-cyber-border shadow-[0_0_12px_rgba(93,230,255,0.1)] mb-6">
              <span className="w-2 h-2 rounded-full bg-secondary animate-pulse"></span>
              <span className="font-mono-key text-[11px] text-secondary uppercase tracking-widest font-semibold">
                Argon2id + XChaCha20-Poly1305 Engine
              </span>
              <span className="text-on-surface-variant font-mono-key text-xs">/</span>
              <span className="font-mono-key text-[11px] text-on-surface-variant">
                Zero-Trust V3
              </span>
            </div>

            {/* Main Headline */}
            <h1 className="font-headline-lg text-4xl sm:text-5xl md:text-6xl text-on-surface max-w-4xl tracking-tight mb-6 font-bold leading-tight">
              Zero-Knowledge Credential Orchestration for the{' '}
              <span className="text-transparent bg-clip-text bg-gradient-to-r from-primary via-primary-fixed-dim to-secondary">
                Sovereign Web
              </span>
              .
            </h1>

            {/* Subheadline */}
            <p className="font-body-lg text-base sm:text-lg text-on-surface-variant max-w-2xl mb-8 leading-relaxed">
              Store, synthesize, and synchronize cryptographic credentials across enterprise infrastructure with client-side isolation, local Merkle ledgers, and zero cloud plaintext exposure.
            </p>

            {/* Action CTA Row */}
            <div className="flex flex-wrap items-center justify-center gap-4 mb-16 w-full max-w-xl">
              <button
                onClick={onLaunchVault}
                className="flex items-center justify-center gap-2 bg-primary text-on-primary font-semibold text-sm px-6 py-3 rounded-lg shadow-[0_0_20px_rgba(124,92,255,0.4)] hover:bg-primary-hover transition-all cursor-pointer group"
              >
                <span>Launch Your Vault</span>
                <span className="material-symbols-outlined text-lg group-hover:translate-x-1 transition-transform">
                  arrow_forward
                </span>
              </button>
              <button
                onClick={onOpenUnlock}
                className="flex items-center justify-center gap-2 bg-surface-container hover:bg-surface-container-high text-on-surface text-sm px-5 py-3 rounded-lg border border-cyber-border transition-all cursor-pointer"
              >
                <span className="material-symbols-outlined text-secondary text-lg">lock_open</span>
                <span>Unlock Existing Session</span>
              </button>

              {/* Terminal Snippet */}
              <button
                onClick={handleCopyCli}
                className="w-full sm:w-auto flex items-center justify-between gap-4 px-4 py-2.5 rounded-lg bg-surface-container-lowest border border-cyber-border text-left group cursor-pointer hover:border-secondary/40 transition-colors"
              >
                <div className="flex items-center gap-1.5 font-mono-data text-xs text-secondary">
                  <span className="text-outline-variant">$</span>
                  <span>npx @blockvault/cli init</span>
                </div>
                <span className="font-mono-key text-[10px] text-on-surface-variant bg-surface-container px-2 py-0.5 rounded uppercase">
                  {copiedCli ? 'COPIED!' : 'COPY'}
                </span>
              </button>
            </div>

            {/* High-Fidelity Vault UI Preview Box */}
            <div className="w-full max-w-4xl relative">
              <div className="relative rounded-xl p-1 bg-gradient-to-b from-primary/30 via-surface-container to-surface-container-lowest shadow-[0_24px_64px_-12px_rgba(0,0,0,0.8)] border border-cyber-border">
                <div className="rounded-lg bg-surface-container-low p-4 sm:p-6 text-left overflow-hidden">
                  {/* Window Titlebar */}
                  <div className="flex items-center justify-between pb-4 mb-4 border-b border-cyber-border/40">
                    <div className="flex items-center gap-2">
                      <span className="w-3 h-3 rounded-full bg-error/80"></span>
                      <span className="w-3 h-3 rounded-full bg-tertiary/80"></span>
                      <span className="w-3 h-3 rounded-full bg-secondary/80"></span>
                      <span className="font-mono-key text-[11px] text-on-surface-variant ml-2">
                        vault-session-local // authenticated [FIDO2-ECC]
                      </span>
                    </div>
                    <div className="flex items-center gap-2">
                      <span className="inline-flex items-center gap-1 font-mono-key text-[10px] px-2 py-0.5 rounded bg-surface text-secondary border border-secondary/20">
                        <span className="w-1.5 h-1.5 rounded-full bg-secondary animate-pulse"></span>
                        TLS 1.3 PINNED
                      </span>
                      <span className="font-mono-key text-[10px] text-on-surface-variant">
                        NODE: SV-EUS-0914
                      </span>
                    </div>
                  </div>

                  {/* Vault Preview Stats Row */}
                  <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 mb-4">
                    <div className="bg-surface-container p-3 rounded-lg border border-cyber-border/40">
                      <span className="font-mono-key text-[10px] text-on-surface-variant uppercase">
                        Active Keyspace
                      </span>
                      <div className="text-xl font-bold text-on-surface font-mono-numbers mt-0.5">
                        142 Encrypted Secrets
                      </div>
                      <span className="text-[10px] text-secondary font-mono-key">
                        AES-256 GCM Client-Side
                      </span>
                    </div>
                    <div className="bg-surface-container p-3 rounded-lg border border-cyber-border/40">
                      <span className="font-mono-key text-[10px] text-on-surface-variant uppercase">
                        Posture Index
                      </span>
                      <div className="text-xl font-bold text-secondary font-mono-numbers mt-0.5">
                        88 / 100 [Optimal]
                      </div>
                      <span className="text-[10px] text-on-surface-variant font-mono-key">
                        Zero Breach Exposure
                      </span>
                    </div>
                    <div className="bg-surface-container p-3 rounded-lg border border-cyber-border/40">
                      <span className="font-mono-key text-[10px] text-on-surface-variant uppercase">
                        Chain Head
                      </span>
                      <div className="text-xl font-bold text-primary font-mono-numbers mt-0.5">
                        #01428 Merkle Proof
                      </div>
                      <span className="text-[10px] text-on-surface-variant font-mono-key">
                        Tamper-Evident Ledger
                      </span>
                    </div>
                  </div>

                  {/* Interactive Button in preview */}
                  <div className="flex items-center justify-between p-3 rounded-lg bg-surface-container-lowest border border-cyber-border/40">
                    <div className="flex items-center gap-2">
                      <span className="material-symbols-outlined text-secondary text-base">verified</span>
                      <span className="text-xs text-on-surface font-mono-data">
                        Dr. Elena Vance • elena.vance@sovereign-sec.io
                      </span>
                    </div>
                    <button
                      onClick={onLaunchVault}
                      className="px-3 py-1 rounded bg-primary text-on-primary font-semibold text-xs hover:bg-primary-hover transition-colors cursor-pointer"
                    >
                      Enter Interactive Enclave
                    </button>
                  </div>
                </div>
              </div>
            </div>
          </section>
        </div>

        {/* Feature Grid Section */}
        <section id="features" className="max-w-7xl mx-auto px-6 py-20 border-t border-cyber-border/40">
          <div className="text-center max-w-2xl mx-auto mb-16">
            <h2 className="font-headline-lg text-2xl sm:text-3xl font-bold text-on-surface mb-3">
              Engineered for Zero-Trust Environments
            </h2>
            <p className="text-sm text-on-surface-variant">
              Every cryptographic action is mathematically isolated and executed strictly on the client.
            </p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
            <div className="p-6 rounded-xl bg-surface-container-low border border-cyber-border hover:border-primary/40 transition-colors">
              <div className="w-10 h-10 rounded-lg bg-surface-container flex items-center justify-center text-primary mb-4 border border-primary/20">
                <span className="material-symbols-outlined text-2xl">lock</span>
              </div>
              <h3 className="font-headline-sm text-base font-semibold text-on-surface mb-2">
                Client-Side Argon2id & AES-GCM
              </h3>
              <p className="text-xs text-on-surface-variant leading-relaxed">
                Derive encryption keys from your master passphrase using memory-hard Argon2id parameters. Secrets are encrypted locally before any network dispatch.
              </p>
            </div>

            <div className="p-6 rounded-xl bg-surface-container-low border border-cyber-border hover:border-secondary/40 transition-colors">
              <div className="w-10 h-10 rounded-lg bg-surface-container flex items-center justify-center text-secondary mb-4 border border-secondary/20">
                <span className="material-symbols-outlined text-2xl">receipt_long</span>
              </div>
              <h3 className="font-headline-sm text-base font-semibold text-on-surface mb-2">
                Merkle Hash Chain Audit Ledger
              </h3>
              <p className="text-xs text-on-surface-variant leading-relaxed">
                Every read, decrypt, rotation, and generator execution produces an immutable SHA-256 digest linked sequentially to provide mathematical proof of integrity.
              </p>
            </div>

            <div className="p-6 rounded-xl bg-surface-container-low border border-cyber-border hover:border-tertiary/40 transition-colors">
              <div className="w-10 h-10 rounded-lg bg-surface-container flex items-center justify-center text-tertiary mb-4 border border-tertiary/20">
                <span className="material-symbols-outlined text-2xl">casino</span>
              </div>
              <h3 className="font-headline-sm text-base font-semibold text-on-surface mb-2">
                Entropy Synthesizer
              </h3>
              <p className="text-xs text-on-surface-variant leading-relaxed">
                Pseudo-random cryptographic token synthesis with live Shannon entropy evaluation, diceware passphrase models, and brute-force resistance ratings.
              </p>
            </div>
          </div>
        </section>

        {/* Pricing / Tiers Section */}
        <section id="pricing" className="max-w-7xl mx-auto px-6 py-20 border-t border-cyber-border/40">
          <div className="text-center max-w-2xl mx-auto mb-16">
            <h2 className="font-headline-lg text-2xl sm:text-3xl font-bold text-on-surface mb-3">
              Transparent Sovereignty Tiers
            </h2>
            <p className="text-sm text-on-surface-variant">
              Deploy as an isolated personal enclave or configure enterprise multi-signature governance.
            </p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
            {/* Free Tier */}
            <div className="p-6 rounded-xl bg-surface-container-low border border-cyber-border flex flex-col justify-between">
              <div>
                <span className="font-mono-key text-[11px] text-secondary uppercase font-semibold">
                  Personal Enclave
                </span>
                <div className="text-2xl font-bold text-on-surface mt-2 mb-1">Free / OSS</div>
                <p className="text-xs text-on-surface-variant mb-4">
                  For individual security researchers and sovereign developers.
                </p>
                <ul className="text-xs text-on-surface-variant space-y-2 mb-6">
                  <li className="flex items-center gap-2">✓ Unlimited Local Credentials</li>
                  <li className="flex items-center gap-2">✓ Argon2id Key Derivation</li>
                  <li className="flex items-center gap-2">✓ Entropy Synthesizer</li>
                  <li className="flex items-center gap-2">✓ Local Merkle Audit Ledger</li>
                </ul>
              </div>
              <button
                onClick={onLaunchVault}
                className="w-full py-2.5 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface font-medium text-xs border border-cyber-border transition-colors cursor-pointer"
              >
                Launch Personal Vault
              </button>
            </div>

            {/* Team Tier */}
            <div className="p-6 rounded-xl bg-surface-container-low border-2 border-primary shadow-[0_0_24px_rgba(124,92,255,0.25)] flex flex-col justify-between relative">
              <span className="absolute -top-3 right-6 font-mono-key text-[10px] bg-primary text-on-primary font-bold px-2 py-0.5 rounded">
                MOST POPULAR
              </span>
              <div>
                <span className="font-mono-key text-[11px] text-primary uppercase font-semibold">
                  Team Enclave
                </span>
                <div className="text-2xl font-bold text-on-surface mt-2 mb-1">
                  $12 <span className="text-xs text-on-surface-variant">/ user / mo</span>
                </div>
                <p className="text-xs text-on-surface-variant mb-4">
                  For DevOps teams and security engineers managing shared keys.
                </p>
                <ul className="text-xs text-on-surface-variant space-y-2 mb-6">
                  <li className="flex items-center gap-2 text-on-surface">✓ Everything in Personal</li>
                  <li className="flex items-center gap-2 text-on-surface">✓ Multi-Region Node Sync</li>
                  <li className="flex items-center gap-2 text-on-surface">✓ FIDO2 / Passkey Enforcement</li>
                  <li className="flex items-center gap-2 text-on-surface">✓ Real-time Breach Telemetry</li>
                </ul>
              </div>
              <button
                onClick={onLaunchVault}
                className="w-full py-2.5 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs transition-all shadow-[0_0_12px_rgba(124,92,255,0.35)] cursor-pointer"
              >
                Deploy Team Cluster
              </button>
            </div>

            {/* Enterprise Tier */}
            <div className="p-6 rounded-xl bg-surface-container-low border border-cyber-border flex flex-col justify-between">
              <div>
                <span className="font-mono-key text-[11px] text-secondary uppercase font-semibold">
                  Enterprise Zero-Trust
                </span>
                <div className="text-2xl font-bold text-on-surface mt-2 mb-1">Custom</div>
                <p className="text-xs text-on-surface-variant mb-4">
                  For air-gapped sovereign networks and financial institutions.
                </p>
                <ul className="text-xs text-on-surface-variant space-y-2 mb-6">
                  <li className="flex items-center gap-2">✓ Hardware HSM L3 Partitioning</li>
                  <li className="flex items-center gap-2">✓ Shamir Secret Sharing Quorums</li>
                  <li className="flex items-center gap-2">✓ SIEM & Splunk Merkle Connector</li>
                  <li className="flex items-center gap-2">✓ 24/7 Cryptographic Support</li>
                </ul>
              </div>
              <button
                onClick={onLaunchVault}
                className="w-full py-2.5 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface font-medium text-xs border border-cyber-border transition-colors cursor-pointer"
              >
                Contact Architecture Team
              </button>
            </div>
          </div>
        </section>
      </main>

      {/* Footer */}
      <footer className="w-full bg-surface-container-lowest border-t border-cyber-border/40 py-8 text-center text-xs text-on-surface-variant font-mono-data">
        <div className="max-w-7xl mx-auto px-6 flex flex-col sm:flex-row items-center justify-between gap-4">
          <div className="flex items-center gap-2">
            <span className="w-2 h-2 rounded-full bg-secondary"></span>
            <span>BlockVault Cryptographic Enclave • Built on Argon2id + XChaCha20-Poly1305</span>
          </div>
          <div className="text-[11px] text-on-surface-variant">
            ISO-27001 & SOC-2 Type II Zero-Knowledge Compliant
          </div>
        </div>
      </footer>
    </div>
  );
}
