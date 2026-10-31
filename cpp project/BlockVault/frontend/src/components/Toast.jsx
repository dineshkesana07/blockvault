import React from 'react';

export default function Toast({ toasts = [], onCloseToast }) {
  if (toasts.length === 0) return null;

  return (
    <div className="fixed bottom-6 right-6 z-50 flex flex-col gap-2 pointer-events-auto">
      {toasts.map(toast => {
        let borderClass = 'border-cyber-border-highlight';
        let iconColor = 'text-secondary';
        let iconName = 'verified_user';

        if (toast.type === 'error') {
          borderClass = 'border-error/40 bg-error-container/20';
          iconColor = 'text-error';
          iconName = 'error';
        } else if (toast.type === 'warning') {
          borderClass = 'border-tertiary/40 bg-tertiary-container/20';
          iconColor = 'text-tertiary';
          iconName = 'warning';
        } else if (toast.type === 'success') {
          borderClass = 'border-secondary/40 shadow-[0_0_12px_rgba(93,230,255,0.2)]';
          iconColor = 'text-secondary';
          iconName = 'check_circle';
        }

        return (
          <div
            key={toast.id}
            className={`flex items-center gap-3 px-4 py-3 bg-surface-container-high text-on-surface rounded-lg border ${borderClass} shadow-xl animate-slideUp text-xs font-sans max-w-sm`}
          >
            <span className={`material-symbols-outlined text-lg ${iconColor}`}>
              {toast.icon || iconName}
            </span>
            <div className="flex flex-col flex-1">
              <span className="font-medium text-on-surface">{toast.message}</span>
              {toast.details && (
                <span className="font-mono-data text-[10px] text-on-surface-variant">
                  {toast.details}
                </span>
              )}
            </div>
            <button
              onClick={() => onCloseToast(toast.id)}
              className="text-on-surface-variant hover:text-on-surface text-sm cursor-pointer ml-1"
            >
              ✕
            </button>
          </div>
        );
      })}
    </div>
  );
}
