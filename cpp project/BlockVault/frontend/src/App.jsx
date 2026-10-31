import React, { useState, useEffect } from 'react';
import { vaultService } from './services/vaultService';
import Sidebar from './components/Sidebar';
import Header from './components/Header';
import CommandPalette from './components/CommandPalette';
import Toast from './components/Toast';
import CredentialModal from './components/CredentialModal';
import ExportModal from './components/ExportModal';

import LandingPage from './pages/LandingPage';
import RegisterPage from './pages/RegisterPage';
import LoginPage from './pages/LoginPage';
import UnlockScreen from './pages/UnlockScreen';
import DashboardPage from './pages/DashboardPage';
import VaultPage from './pages/VaultPage';
import GeneratorPage from './pages/GeneratorPage';
import SecurityCenterPage from './pages/SecurityCenterPage';
import AuditLedgerPage from './pages/AuditLedgerPage';
import SettingsPage from './pages/SettingsPage';

export default function App() {
  const [isLocked, setIsLocked] = useState(false);
  const [activePage, setActivePage] = useState('dashboard');
  const [credentials, setCredentials] = useState([]);
  const [ledger, setLedger] = useState([]);
  const [searchTerm, setSearchTerm] = useState('');
  const [selectedCredential, setSelectedCredential] = useState(null);
  const [isMobileOpen, setIsMobileOpen] = useState(false);

  // Modals & Popovers
  const [isCredentialModalOpen, setIsCredentialModalOpen] = useState(false);
  const [editingCredential, setEditingCredential] = useState(null);
  const [isExportModalOpen, setIsExportModalOpen] = useState(false);
  const [isCommandPaletteOpen, setIsCommandPaletteOpen] = useState(false);
  const [toasts, setToasts] = useState([]);

  // Load initial data
  useEffect(() => {
    setCredentials(vaultService.getCredentials());
    setLedger(vaultService.getLedger());
    if (vaultService.isVaultLocked()) {
      setIsLocked(true);
    }
  }, []);

  // Global keyboard shortcuts (Cmd+K / Ctrl+K)
  useEffect(() => {
    const handleKeyDown = (e) => {
      if ((e.metaKey || e.ctrlKey) && e.key === 'k') {
        e.preventDefault();
        setIsCommandPaletteOpen(prev => !prev);
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);

  // Toast Helper
  const addToast = (message, type = 'info', details = '') => {
    const id = Date.now() + Math.random();
    setToasts(prev => [...prev, { id, message, type, details }]);
    setTimeout(() => {
      setToasts(prev => prev.filter(t => t.id !== id));
    }, 4000);
  };

  const removeToast = (id) => {
    setToasts(prev => prev.filter(t => t.id !== id));
  };

  // Lock / Unlock Handlers
  const handleLockVault = () => {
    vaultService.lockVault();
    setIsLocked(true);
    addToast('Vault locked. Ephemeral keys purged from volatile memory.', 'warning');
  };

  const handleUnlockSuccess = () => {
    vaultService.unlockVault();
    setIsLocked(false);
    setLedger(vaultService.getLedger());
  };

  // Credential CRUD Handlers
  const handleSaveCredential = (data) => {
    if (editingCredential) {
      const updated = vaultService.updateCredential(editingCredential.id, data);
      setCredentials(vaultService.getCredentials());
      setLedger(vaultService.getLedger());
      if (selectedCredential && selectedCredential.id === editingCredential.id) {
        setSelectedCredential(updated);
      }
      addToast(`Updated secret "${data.title}".`, 'success');
    } else {
      const added = vaultService.addCredential(data);
      setCredentials(vaultService.getCredentials());
      setLedger(vaultService.getLedger());
      addToast(`Encrypted and stored "${data.title}".`, 'success');
    }
    setEditingCredential(null);
  };

  const handleDeleteCredential = (id) => {
    const updated = vaultService.deleteCredential(id);
    setCredentials(updated);
    setLedger(vaultService.getLedger());
    if (selectedCredential && selectedCredential.id === id) {
      setSelectedCredential(null);
    }
  };

  const handleToggleFavorite = (id) => {
    const updated = vaultService.toggleFavorite(id);
    setCredentials([...updated]);
  };

  const handleOpenEdit = (cred) => {
    setEditingCredential(cred);
    setIsCredentialModalOpen(true);
  };

  const handleOpenNew = () => {
    setEditingCredential(null);
    setIsCredentialModalOpen(true);
  };

  const handleImportCredentials = (importedItems) => {
    const existing = vaultService.getCredentials();
    const merged = [...importedItems, ...existing];
    // Deduplicate by ID
    const uniqueMap = new Map();
    merged.forEach(item => uniqueMap.set(item.id, item));
    const finalList = Array.from(uniqueMap.values());
    localStorage.setItem('blockvault_credentials_v1', JSON.stringify(finalList));
    setCredentials(finalList);
    setLedger(vaultService.getLedger());
  };

  const metrics = vaultService.getMetrics();

  // If locked, render the UnlockScreen
  if (isLocked) {
    return (
      <UnlockScreen
        onUnlockSuccess={handleUnlockSuccess}
        onShowToast={addToast}
      />
    );
  }

  // If user navigated to Registration
  if (activePage === 'register') {
    return (
      <RegisterPage
        onRegisterSuccess={() => setActivePage('dashboard')}
        onNavigateToLogin={() => setActivePage('login')}
        onShowToast={addToast}
      />
    );
  }

  // If user navigated to Login
  if (activePage === 'login') {
    return (
      <LoginPage
        onLoginSuccess={() => setActivePage('dashboard')}
        onNavigateToRegister={() => setActivePage('register')}
        onShowToast={addToast}
      />
    );
  }

  // If public landing page requested
  if (activePage === 'landing') {
    return (
      <LandingPage
        onLaunchVault={() => setActivePage('dashboard')}
        onOpenUnlock={() => setIsLocked(true)}
        onNavigateToRegister={() => setActivePage('register')}
        onNavigateToLogin={() => setActivePage('login')}
      />
    );
  }

  return (
    <div className="w-full min-h-screen bg-background text-on-surface antialiased font-sans select-none">
      {/* Responsive Cyber Noir Sidebar */}
      <Sidebar
        activePage={activePage}
        setActivePage={setActivePage}
        onLockVault={handleLockVault}
        isMobileOpen={isMobileOpen}
        setIsMobileOpen={setIsMobileOpen}
      />

      {/* Main Workspace Frame */}
      <div className="lg:pl-72 pl-0 flex flex-col min-h-screen transition-all duration-300">
        {/* Top Header */}
        <Header
          searchTerm={searchTerm}
          setSearchTerm={setSearchTerm}
          onOpenNewSecret={handleOpenNew}
          onLockVault={handleLockVault}
          onOpenCommandPalette={() => setIsCommandPaletteOpen(true)}
          isMobileOpen={isMobileOpen}
          setIsMobileOpen={setIsMobileOpen}
          onNavigate={setActivePage}
        />

        {/* Dynamic Page Container */}
        <main className="w-full pt-20 px-4 sm:px-6 lg:px-8 flex-1 bg-background ambient-mesh">
          {activePage === 'dashboard' && (
            <DashboardPage
              credentials={credentials}
              metrics={metrics}
              onOpenNewSecret={handleOpenNew}
              onOpenExport={() => setIsExportModalOpen(true)}
              onNavigate={setActivePage}
              onSelectCredential={(cred) => {
                setSelectedCredential(cred);
                setActivePage('password-vault');
              }}
              onShowToast={addToast}
            />
          )}

          {activePage === 'password-vault' && (
            <VaultPage
              credentials={credentials}
              searchTerm={searchTerm}
              onOpenNewSecret={handleOpenNew}
              onOpenExport={() => setIsExportModalOpen(true)}
              onEditCredential={handleOpenEdit}
              onDeleteCredential={handleDeleteCredential}
              onToggleFavorite={handleToggleFavorite}
              selectedItem={selectedCredential}
              setSelectedItem={setSelectedCredential}
              onShowToast={addToast}
            />
          )}

          {activePage === 'password-generator' && (
            <GeneratorPage
              onSaveToVault={handleOpenNew}
              onShowToast={addToast}
            />
          )}

          {activePage === 'security-center' && (
            <SecurityCenterPage
              credentials={credentials}
              metrics={metrics}
              onEditCredential={handleOpenEdit}
              onShowToast={addToast}
            />
          )}

          {activePage === 'audit-ledger' && (
            <AuditLedgerPage
              ledger={ledger}
              onShowToast={addToast}
            />
          )}

          {activePage === 'settings' && (
            <SettingsPage
              onShowToast={addToast}
              onOpenExport={() => setIsExportModalOpen(true)}
            />
          )}
        </main>
      </div>

      {/* Global Modals & Notifications */}
      <CredentialModal
        isOpen={isCredentialModalOpen}
        onClose={() => {
          setIsCredentialModalOpen(false);
          setEditingCredential(null);
        }}
        onSave={handleSaveCredential}
        initialData={editingCredential}
      />

      <ExportModal
        isOpen={isExportModalOpen}
        onClose={() => setIsExportModalOpen(false)}
        credentials={credentials}
        onImportCredentials={handleImportCredentials}
        onShowToast={addToast}
      />

      <CommandPalette
        isOpen={isCommandPaletteOpen}
        onClose={() => setIsCommandPaletteOpen(false)}
        credentials={credentials}
        onSelectCredential={(cred) => {
          setSelectedCredential(cred);
          setActivePage('password-vault');
        }}
        onNavigate={setActivePage}
        onLockVault={handleLockVault}
        onOpenNewSecret={handleOpenNew}
      />

      <Toast toasts={toasts} onCloseToast={removeToast} />
    </div>
  );
}
