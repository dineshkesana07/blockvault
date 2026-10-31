import React from 'react';

export default function ConfirmModal({
  isOpen,
  onClose,
  onConfirm,
  title = 'Confirm Sensitive Action',
  message = 'Are you sure you want to permanently execute this action? This change cannot be undone.',
  itemTitle = '',
  confirmText = 'Permanently Delete',
  danger = true
}) {
  if (!isOpen) return null;

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/80 backdrop-blur-md animate-fadeIn"
      onClick={onClose}
    >
      <div
        className="w-full max-w-md bg-surface-container-low border border-cyber-border-highlight rounded-xl shadow-2xl p-6 relative overflow-hidden"
        onClick={e => e.stopPropagation()}
      >
        {/* Top Emissive Danger Bar */}
        <div className={`absolute top-0 left-0 right-0 h-1 ${danger ? 'bg-error shadow-[0_0_12px_rgba(239,68,68,0.5)]' : 'bg-primary'}`} />

        <div className="flex items-start gap-3.5 mb-4">
          <div className={`w-10 h-10 rounded-lg shrink-0 flex items-center justify-center ${danger ? 'bg-error/15 text-error border border-error/30' : 'bg-primary/15 text-primary border border-primary/30'}`}>
            <span className="material-symbols-outlined text-2xl">
              {danger ? 'warning' : 'shield_question'}
            </span>
          </div>

          <div className="flex flex-col text-left">
            <h3 className="font-headline-sm text-base font-bold text-on-surface">
              {title}
            </h3>
            <span className="font-mono-key text-[10px] text-on-surface-variant uppercase tracking-wider">
              Enclave Integrity Verification
            </span>
          </div>
        </div>

        <div className="text-left text-xs text-on-surface-variant space-y-2 mb-6">
          <p>{message}</p>
          {itemTitle && (
            <div className="p-2.5 rounded bg-surface-container border border-cyber-border font-mono-data text-on-surface text-xs break-all">
              Target: <span className="text-secondary font-semibold">{itemTitle}</span>
            </div>
          )}
          {danger && (
            <div className="flex items-center gap-1.5 text-[11px] text-error bg-error/10 p-2 rounded border border-error/20">
              <span className="material-symbols-outlined text-sm">shield</span>
              <span>Hardware keys and AES-256-GCM ciphertexts will be purged from volatile memory.</span>
            </div>
          )}
        </div>

        <div className="flex items-center justify-end gap-3">
          <button
            type="button"
            onClick={onClose}
            className="px-4 py-2 rounded-lg bg-surface-container hover:bg-surface-container-high text-xs text-on-surface border border-cyber-border transition-colors cursor-pointer"
          >
            Cancel
          </button>
          <button
            type="button"
            onClick={() => {
              onConfirm();
              onClose();
            }}
            className={`px-4 py-2 rounded-lg text-xs font-semibold text-white transition-all shadow-md cursor-pointer ${
              danger
                ? 'bg-error hover:bg-error/80 shadow-[0_0_14px_rgba(239,68,68,0.4)]'
                : 'bg-primary hover:bg-primary-hover shadow-[0_0_14px_rgba(124,92,255,0.4)]'
            }`}
          >
            {confirmText}
          </button>
        </div>
      </div>
    </div>
  );
}

