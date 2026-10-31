import React, { useState, useEffect } from 'react';
import { generatePassword, calculateEntropy } from '../services/cryptoService';

export default function GeneratorPage({ onSaveToVault, onShowToast }) {
  const [length, setLength] = useState(24);
  const [useUpper, setUseUpper] = useState(true);
  const [useLower, setUseLower] = useState(true);
  const [useNumbers, setUseNumbers] = useState(true);
  const [useSymbols, setUseSymbols] = useState(true);
  const [avoidAmbiguous, setAvoidAmbiguous] = useState(true);
  const [mode, setMode] = useState('random'); // 'random' | 'passphrase'
  const [wordCount, setWordCount] = useState(5);
  const [separator, setSeparator] = useState('-');

  const [generatedSecret, setGeneratedSecret] = useState('');
  const [history, setHistory] = useState([]);
  const [batchList, setBatchList] = useState([]);

  // Synthesize key
  const handleSynthesize = () => {
    const key = generatePassword({
      length,
      useUpper,
      useLower,
      useNumbers,
      useSymbols,
      avoidAmbiguous,
      mode,
      wordCount,
      separator
    });
    setGeneratedSecret(key);
    setHistory(prev => [{ key, timestamp: new Date().toLocaleTimeString(), entropy: calculateEntropy(key) }, ...prev.slice(0, 9)]);
  };

  useEffect(() => {
    handleSynthesize();
  }, [length, useUpper, useLower, useNumbers, useSymbols, avoidAmbiguous, mode, wordCount, separator]);

  const handleBatchGenerate = () => {
    const list = [];
    for (let i = 0; i < 5; i++) {
      list.push(generatePassword({
        length,
        useUpper,
        useLower,
        useNumbers,
        useSymbols,
        avoidAmbiguous,
        mode,
        wordCount,
        separator
      }));
    }
    setBatchList(list);
    onShowToast('Synthesized 5x isolated cryptographic tokens.', 'success');
  };

  const copySecret = (text) => {
    navigator.clipboard.writeText(text);
    onShowToast('Copied synthesized secret to clipboard.', 'success');
  };

  const entropy = calculateEntropy(generatedSecret);

  return (
    <div className="flex flex-col w-full pb-16 space-y-6 text-left font-sans">
      {/* Workspace Header */}
      <div className="flex flex-col md:flex-row md:items-end justify-between gap-4 pt-2">
        <div className="flex flex-col gap-1 max-w-2xl">
          <div className="flex items-center gap-2">
            <span className="font-mono-key text-[11px] text-secondary px-2 py-0.5 rounded bg-surface-container-high border border-secondary/20">
              SYS::CRYPT_RNG_V4
            </span>
            <span className="w-1.5 h-1.5 rounded-full bg-secondary animate-pulse"></span>
            <span className="font-mono-key text-[11px] text-on-surface-variant">
              SHA-256 SEEDED
            </span>
          </div>
          <h1 className="font-headline-lg text-2xl font-bold text-on-surface tracking-tight">
            Entropy Generator & Key Synthesizer
          </h1>
          <p className="font-body-md text-xs text-on-surface-variant">
            Client-side pseudo-random credential generator for zero-knowledge vaults. Mathematical randomness executed strictly inside isolated browser threads.
          </p>
        </div>

        <div className="flex items-center gap-2.5">
          <button
            onClick={handleBatchGenerate}
            className="flex items-center gap-1.5 px-3.5 py-2 bg-surface-container hover:bg-surface-container-high text-on-surface rounded-lg transition-colors text-xs border border-cyber-border cursor-pointer"
          >
            <span className="material-symbols-outlined text-sm text-primary">splitscreen</span>
            <span>Batch Generate (5x)</span>
          </button>
        </div>
      </div>

      {/* Main Generator Display Card */}
      <div className="bg-surface-container-low rounded-xl p-6 border border-cyber-border-highlight shadow-2xl relative overflow-hidden">
        {/* Glow backdrop */}
        <div className="absolute -top-24 -right-24 w-80 h-80 rounded-full bg-primary/10 blur-3xl pointer-events-none"></div>

        <div className="relative z-10 flex flex-col gap-4">
          <div className="flex items-center justify-between text-xs font-mono-key text-on-surface-variant">
            <span>GENERATED SECRET PAYLOAD</span>
            <span className="text-secondary">WEBCRYPTO API CS-PRNG</span>
          </div>

          {/* Large Secret Display */}
          <div className="flex items-center justify-between gap-3 p-4 bg-surface-container-lowest rounded-xl border border-cyber-border shadow-inner">
            <span className="font-mono-data text-base sm:text-xl text-on-surface select-all break-all tracking-wide">
              {generatedSecret}
            </span>
            <div className="flex items-center gap-2">
              <button
                onClick={handleSynthesize}
                className="p-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface-variant hover:text-secondary transition-colors cursor-pointer"
                title="Re-synthesize"
              >
                <span className="material-symbols-outlined text-base">refresh</span>
              </button>
              <button
                onClick={() => copySecret(generatedSecret)}
                className="flex items-center gap-1 px-3 py-2 rounded-lg bg-secondary text-on-secondary font-semibold text-xs hover:bg-secondary-fixed transition-colors shadow-[0_0_12px_rgba(93,230,255,0.3)] cursor-pointer"
              >
                <span className="material-symbols-outlined text-sm">content_copy</span>
                <span>Copy</span>
              </button>
            </div>
          </div>

          {/* Entropy & Crack Resistance Diagnostic */}
          <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 p-3 bg-surface-container rounded-lg border border-cyber-border/40 text-xs font-mono-data">
            <div>
              <span className="text-[10px] text-on-surface-variant uppercase font-mono-key">Entropy Bits:</span>
              <div className={`text-base font-bold ${entropy.color}`}>{entropy.bits} bits</div>
            </div>
            <div>
              <span className="text-[10px] text-on-surface-variant uppercase font-mono-key">Classification:</span>
              <div className="text-sm font-semibold text-on-surface">{entropy.label}</div>
            </div>
            <div>
              <span className="text-[10px] text-on-surface-variant uppercase font-mono-key">Time to Brute Force:</span>
              <div className="text-sm font-semibold text-secondary">{entropy.crackTime}</div>
            </div>
          </div>
        </div>
      </div>

      {/* Generator Configuration Controls */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Left: Mode & Length Sliders */}
        <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border flex flex-col gap-5">
          <div className="flex items-center justify-between pb-2 border-b border-cyber-border/40">
            <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
              Synthesizer Architecture
            </h3>
            {/* Mode Toggle */}
            <div className="flex rounded-lg bg-surface-container p-0.5 border border-cyber-border text-xs">
              <button
                onClick={() => setMode('random')}
                className={`px-3 py-1 rounded-md transition-all cursor-pointer font-medium ${
                  mode === 'random'
                    ? 'bg-primary text-on-primary font-semibold shadow-sm'
                    : 'text-on-surface-variant hover:text-on-surface'
                }`}
              >
                Random String
              </button>
              <button
                onClick={() => setMode('passphrase')}
                className={`px-3 py-1 rounded-md transition-all cursor-pointer font-medium ${
                  mode === 'passphrase'
                    ? 'bg-primary text-on-primary font-semibold shadow-sm'
                    : 'text-on-surface-variant hover:text-on-surface'
                }`}
              >
                Diceware Passphrase
              </button>
            </div>
          </div>

          {mode === 'random' ? (
            <div className="flex flex-col gap-4">
              <div className="flex flex-col gap-2">
                <div className="flex items-center justify-between text-xs">
                  <span className="text-on-surface font-medium">Secret Length:</span>
                  <span className="font-mono-data text-secondary font-bold text-sm">{length} characters</span>
                </div>
                <input
                  type="range"
                  min="8"
                  max="64"
                  value={length}
                  onChange={e => setLength(parseInt(e.target.value, 10))}
                  className="w-full accent-primary cursor-pointer"
                />
                <div className="flex justify-between text-[10px] font-mono-key text-on-surface-variant">
                  <span>8 (Basic)</span>
                  <span>24 (Recommended)</span>
                  <span>64 (Quantum Max)</span>
                </div>
              </div>

              {/* Toggles */}
              <div className="grid grid-cols-2 gap-3 text-xs pt-2">
                <label className="flex items-center gap-2 cursor-pointer">
                  <input
                    type="checkbox"
                    checked={useUpper}
                    onChange={e => setUseUpper(e.target.checked)}
                    className="accent-primary rounded"
                  />
                  <span>Uppercase (A-Z)</span>
                </label>
                <label className="flex items-center gap-2 cursor-pointer">
                  <input
                    type="checkbox"
                    checked={useLower}
                    onChange={e => setUseLower(e.target.checked)}
                    className="accent-primary rounded"
                  />
                  <span>Lowercase (a-z)</span>
                </label>
                <label className="flex items-center gap-2 cursor-pointer">
                  <input
                    type="checkbox"
                    checked={useNumbers}
                    onChange={e => setUseNumbers(e.target.checked)}
                    className="accent-primary rounded"
                  />
                  <span>Numbers (0-9)</span>
                </label>
                <label className="flex items-center gap-2 cursor-pointer">
                  <input
                    type="checkbox"
                    checked={useSymbols}
                    onChange={e => setUseSymbols(e.target.checked)}
                    className="accent-primary rounded"
                  />
                  <span>Symbols (!@#$%)</span>
                </label>
                <label className="flex items-center gap-2 cursor-pointer col-span-2">
                  <input
                    type="checkbox"
                    checked={avoidAmbiguous}
                    onChange={e => setAvoidAmbiguous(e.target.checked)}
                    className="accent-primary rounded"
                  />
                  <span className="text-on-surface-variant">Avoid Ambiguous Characters (0, O, 1, l, I)</span>
                </label>
              </div>
            </div>
          ) : (
            <div className="flex flex-col gap-4">
              <div className="flex flex-col gap-2">
                <div className="flex items-center justify-between text-xs">
                  <span className="text-on-surface font-medium">Word Count:</span>
                  <span className="font-mono-data text-secondary font-bold text-sm">{wordCount} words</span>
                </div>
                <input
                  type="range"
                  min="3"
                  max="8"
                  value={wordCount}
                  onChange={e => setWordCount(parseInt(e.target.value, 10))}
                  className="w-full accent-primary cursor-pointer"
                />
              </div>

              <div className="flex flex-col gap-1 text-xs">
                <label className="text-on-surface-variant">Word Separator:</label>
                <div className="flex gap-2">
                  {['-', '_', '.', ' ', '#'].map(sep => (
                    <button
                      key={sep}
                      onClick={() => setSeparator(sep)}
                      className={`px-3 py-1.5 rounded-lg border font-mono-data cursor-pointer ${
                        separator === sep
                          ? 'border-secondary text-secondary bg-surface-container'
                          : 'border-cyber-border text-on-surface-variant'
                      }`}
                    >
                      {sep === ' ' ? 'space' : sep}
                    </button>
                  ))}
                </div>
              </div>
            </div>
          )}
        </div>

        {/* Right: Synthesis History & Batch Output */}
        <div className="bg-surface-container-low rounded-xl p-5 border border-cyber-border flex flex-col gap-4">
          <div className="flex items-center justify-between pb-2 border-b border-cyber-border/40">
            <div className="flex items-center gap-2">
              <span className="material-symbols-outlined text-secondary text-base">history</span>
              <h3 className="font-headline-sm text-sm font-semibold text-on-surface">
                Session Generation Log
              </h3>
            </div>
            <span className="font-mono-key text-[10px] text-on-surface-variant">
              LOCAL MEMORY ONLY
            </span>
          </div>

          {batchList.length > 0 && (
            <div className="flex flex-col gap-2 p-3 bg-surface-container rounded-lg border border-secondary/20">
              <span className="font-mono-key text-[10px] text-secondary uppercase">
                Batch Synthesized 5x Tokens
              </span>
              {batchList.map((token, i) => (
                <div key={i} className="flex items-center justify-between text-xs font-mono-data bg-surface-container-lowest p-2 rounded">
                  <span className="truncate max-w-[240px] text-on-surface">{token}</span>
                  <button
                    onClick={() => copySecret(token)}
                    className="text-secondary text-[10px] font-mono-key hover:underline cursor-pointer"
                  >
                    COPY
                  </button>
                </div>
              ))}
            </div>
          )}

          <div className="flex flex-col gap-2 max-h-56 overflow-y-auto">
            {history.slice(0, 5).map((item, i) => (
              <div
                key={i}
                className="flex items-center justify-between p-2.5 rounded-lg bg-surface-container/60 hover:bg-surface-container transition-colors text-xs font-mono-data border border-cyber-border/30"
              >
                <div className="flex flex-col truncate max-w-[220px]">
                  <span className="text-on-surface truncate">{item.key}</span>
                  <span className="text-[10px] text-on-surface-variant font-mono-key">
                    {item.timestamp} • {item.entropy.bits} bits
                  </span>
                </div>
                <button
                  onClick={() => copySecret(item.key)}
                  className="p-1.5 rounded hover:bg-surface-container-high text-on-surface-variant hover:text-secondary cursor-pointer"
                  title="Copy"
                >
                  <span className="material-symbols-outlined text-sm">content_copy</span>
                </button>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}
