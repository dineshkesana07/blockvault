# BlockVault - Zero-Knowledge Credential Orchestration & Security SaaS Frontend

## Overview
BlockVault is a client-side zero-knowledge password manager and cryptographic security orchestration platform engineered according to the **Cyber Noir Vault System** design specification.

### Key Features
- **Client-Side Cryptography**: Argon2id memory-hard key derivation & AES-256-GCM encryption with zero cloud plaintext exposure.
- **Master Password & FIDO2 Biometric Unlock**: Hardware enclave integration with simulated WebAuthn step-up authentication.
- **Real-Time Security Health Center**: Live entropy bit calculations, posture scoring (88/100), reuse cluster detection, and 1-click remediation.
- **Entropy Synthesizer & Generator**: Customizable length (8-64), Diceware passphrase generator, Shannon entropy bit metrics, and batch generator.
- **Cryptographic Merkle Audit Ledger**: Tamper-evident SHA-256 event chaining with local mathematical proof verification.
- **Interactive Omnisearch (`Cmd+K` / `Ctrl+K`)**: Fast command palette for credential searches and quick navigation.
- **Offline Encrypted Backup**: Export client-encrypted `.enc.json` snapshots.

## Project Structure
```
BlockVault/
└── frontend/
    ├── package.json
    ├── vite.config.js
    ├── index.html
    └── src/
        ├── index.css
        ├── main.jsx
        ├── App.jsx
        ├── components/
        │   ├── Sidebar.jsx
        │   ├── Header.jsx
        │   ├── CommandPalette.jsx
        │   ├── Toast.jsx
        │   ├── CredentialModal.jsx
        │   └── ExportModal.jsx
        ├── pages/
        │   ├── LandingPage.jsx
        │   ├── UnlockScreen.jsx
        │   ├── DashboardPage.jsx
        │   ├── VaultPage.jsx
        │   ├── GeneratorPage.jsx
        │   ├── SecurityCenterPage.jsx
        │   ├── AuditLedgerPage.jsx
        │   └── SettingsPage.jsx
        └── services/
            ├── cryptoService.js
            └── vaultService.js
```

## Running the Application

### 1. Install Dependencies
```bash
cd BlockVault/frontend
npm install
```

### 2. Start Vite Dev Server
```bash
npm run dev
```
Open your browser at `http://localhost:3000` to interact with the full Cyber Noir BlockVault interface.

### 3. Production Build
```bash
npm run build
```
