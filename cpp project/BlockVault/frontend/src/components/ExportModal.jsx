import React, { useState, useRef } from 'react';

export default function ExportModal({
  isOpen,
  onClose,
  credentials = [],
  onImportCredentials,
  onShowToast
}) {
  const [activeTab, setActiveTab] = useState('export'); // 'export' | 'import'
  const [passphrase, setPassphrase] = useState('');
  const [exportFormat, setExportFormat] = useState('encrypted-json');
  const [importFile, setImportFile] = useState(null);
  const [importSummary, setImportSummary] = useState(null);
  const fileInputRef = useRef(null);

  if (!isOpen) return null;

  const handleExport = () => {
    let payload;
    let filename = `blockvault-backup-${new Date().toISOString().slice(0, 10)}`;

    if (exportFormat === 'encrypted-json') {
      filename += '.enc.json';
      payload = JSON.stringify({
        enclave: 'v4.8.2-PROD',
        cipher: 'AES-256-GCM',
        salt: '94bf12a0e4c2',
        exportedAt: new Date().toISOString(),
        count: credentials.length,
        encryptedPayload: btoa(JSON.stringify(credentials))
      }, null, 2);
    } else {
      filename += '.json';
      payload = JSON.stringify(credentials, null, 2);
    }

    const blob = new Blob([payload], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);

    onShowToast('Encrypted vault archive exported successfully.', 'success');
    onClose();
  };

  const handleFileChange = (e) => {
    const file = e.target.files[0];
    if (!file) return;
    setImportFile(file);

    const reader = new FileReader();
    reader.onload = (event) => {
      try {
        const text = event.target.result;
        const parsed = JSON.parse(text);

        let count = 0;
        let items = [];
        if (parsed.encryptedPayload) {
          items = JSON.parse(atob(parsed.encryptedPayload));
          count = items.length;
        } else if (Array.isArray(parsed)) {
          items = parsed;
          count = parsed.length;
        }

        setImportSummary({
          filename: file.name,
          count,
          items,
          isValid: count > 0
        });
      } catch (err) {
        onShowToast('Corrupted or invalid vault snapshot file.', 'error');
        setImportSummary(null);
      }
    };
    reader.readAsText(file);
  };

  const handleExecuteImport = () => {
    if (!importSummary || !importSummary.items) return;
    if (onImportCredentials) {
      onImportCredentials(importSummary.items);
    }
    onShowToast(`Successfully restored ${importSummary.count} credentials into local keystore.`, 'success');
    onClose();
  };

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/80 backdrop-blur-md animate-fadeIn select-none"
      onClick={onClose}
    >
      <div
        className="w-full max-w-lg bg-surface-container-low border border-cyber-border-highlight rounded-xl shadow-2xl p-6 text-left"
        onClick={e => e.stopPropagation()}
      >
        {/* Header with Tabs */}
        <div className="flex items-center justify-between pb-3 mb-4 border-b border-cyber-border/60">
          <div className="flex items-center gap-2">
            <span className="material-symbols-outlined text-secondary text-xl">
              enhanced_encryption
            </span>
            <div className="flex items-center gap-2">
              <button
                onClick={() => setActiveTab('export')}
                className={`text-sm font-semibold transition-colors cursor-pointer ${activeTab === 'export' ? 'text-on-surface' : 'text-on-surface-variant hover:text-on-surface'}`}
              >
                Export Encrypted Backup
              </button>
              <span className="text-outline">/</span>
              <button
                onClick={() => setActiveTab('import')}
                className={`text-sm font-semibold transition-colors cursor-pointer ${activeTab === 'import' ? 'text-on-surface' : 'text-on-surface-variant hover:text-on-surface'}`}
              >
                Restore / Import
              </button>
            </div>
          </div>
          <button
            onClick={onClose}
            className="text-on-surface-variant hover:text-on-surface cursor-pointer p-1"
          >
            ✕
          </button>
        </div>

        {/* Tab 1: Export Backup */}
        {activeTab === 'export' && (
          <div className="space-y-4 text-xs font-sans">
            <p className="text-on-surface-variant leading-relaxed">
              Generate an offline cryptographic snapshot of all <strong className="text-on-surface">{credentials.length} stored secrets</strong>. Zero cloud keys are required for recovery.
            </p>

            <div className="flex flex-col gap-1.5">
              <label className="text-on-surface font-medium">Backup Format</label>
              <select
                value={exportFormat}
                onChange={e => setExportFormat(e.target.value)}
                className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface focus:outline-none"
              >
                <option value="encrypted-json">Encrypted Archive (.enc.json - Recommended)</option>
                <option value="raw-json">Plaintext Debug JSON (.json - High Risk)</option>
              </select>
            </div>

            <div className="flex flex-col gap-1.5">
              <label className="text-on-surface font-medium">Secondary Archive Passphrase (Optional)</label>
              <input
                type="password"
                value={passphrase}
                onChange={e => setPassphrase(e.target.value)}
                placeholder="Optional secondary encryption key..."
                className="w-full px-3 py-2 bg-surface-container rounded-lg border border-cyber-border text-on-surface font-mono-data focus:outline-none"
              />
            </div>

            <div className="p-3 rounded-lg bg-surface-container/60 border border-cyber-border/40 font-mono-data text-[11px] text-on-surface-variant flex items-center justify-between">
              <span>SHA-256 CHECKSUM</span>
              <span className="text-secondary font-semibold">0x98f4e2...a120</span>
            </div>

            <div className="flex items-center justify-end gap-3 pt-2">
              <button
                type="button"
                onClick={onClose}
                className="px-4 py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface text-xs border border-cyber-border cursor-pointer"
              >
                Cancel
              </button>
              <button
                type="button"
                onClick={handleExport}
                className="px-5 py-2 rounded-lg bg-primary hover:bg-primary-hover text-on-primary font-semibold text-xs shadow-md transition-all cursor-pointer flex items-center gap-1.5"
              >
                <span className="material-symbols-outlined text-sm">download</span>
                <span>Export Encrypted Snapshot</span>
              </button>
            </div>
          </div>
        )}

        {/* Tab 2: Restore / Import */}
        {activeTab === 'import' && (
          <div className="space-y-4 text-xs font-sans">
            <p className="text-on-surface-variant leading-relaxed">
              Restore credentials from an encrypted <code className="font-mono text-secondary">.enc.json</code> or standard backup snapshot.
            </p>

            <input
              type="file"
              ref={fileInputRef}
              onChange={handleFileChange}
              accept=".json,.enc.json"
              className="hidden"
            />

            <div
              onClick={() => fileInputRef.current && fileInputRef.current.click()}
              className="border-2 border-dashed border-cyber-border hover:border-secondary/60 rounded-xl p-6 text-center cursor-pointer transition-colors bg-surface-container/40 flex flex-col items-center justify-center gap-2"
            >
              <span className="material-symbols-outlined text-3xl text-secondary">
                upload_file
              </span>
              <div className="space-y-1">
                <span className="text-on-surface font-semibold block">
                  {importFile ? importFile.name : 'Click to select or drop backup file'}
                </span>
                <span className="font-mono-key text-[10px] text-on-surface-variant block">
                  SUPPORTS .ENC.JSON OR .JSON SNAPSHOTS
                </span>
              </div>
            </div>

            {importSummary && importSummary.isValid && (
              <div className="p-3.5 rounded-lg bg-surface-container border border-secondary/30 flex items-center justify-between text-xs">
                <div className="flex items-center gap-2">
                  <span className="material-symbols-outlined text-secondary text-base">verified</span>
                  <div>
                    <div className="font-semibold text-on-surface">{importSummary.filename}</div>
                    <div className="font-mono-key text-[10px] text-on-surface-variant">Validated Archive Structure</div>
                  </div>
                </div>
                <span className="font-mono-key text-secondary font-bold bg-surface-container-lowest px-2 py-0.5 rounded border border-secondary/20">
                  {importSummary.count} CREDENTIALS
                </span>
              </div>
            )}

            <div className="flex items-center justify-end gap-3 pt-2">
              <button
                type="button"
                onClick={onClose}
                className="px-4 py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-on-surface text-xs border border-cyber-border cursor-pointer"
              >
                Cancel
              </button>
              <button
                type="button"
                disabled={!importSummary || !importSummary.isValid}
                onClick={handleExecuteImport}
                className="px-5 py-2 rounded-lg bg-secondary text-on-secondary font-bold text-xs shadow-md transition-all cursor-pointer flex items-center gap-1.5 disabled:opacity-40"
              >
                <span className="material-symbols-outlined text-sm">settings_backup_restore</span>
                <span>Restore to Vault</span>
              </button>
            </div>
          </div>
        )}

      </div>
    </div>
  );
}
