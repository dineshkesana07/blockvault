import { calculateEntropy, quickHash } from './cryptoService';

const VAULT_STORAGE_KEY = 'blockvault_credentials_v1';
const LEDGER_STORAGE_KEY = 'blockvault_merkle_ledger_v1';
const LOCK_STORAGE_KEY = 'blockvault_lock_state_v1';

// Seed initial enterprise credentials
const INITIAL_CREDENTIALS = [
  {
    id: 'bv-cred-101',
    title: 'GitHub Enterprise // DevOps',
    category: 'logins',
    username: 'elena.vance@sovereign-sec.io',
    password: 'p9$V#mK9!zX@2Lq8_9aT4#eW',
    url: 'https://github.com/sovereign-sec',
    tags: ['Production', 'MFA', 'Enclave'],
    favorite: true,
    totpSecret: 'JBSWY3DPEHPK3PXP',
    notes: 'Primary CI/CD pipeline deployment key and repository administrator access.',
    breached: false,
    lastModified: '2026-10-28T14:20:00Z',
    reused: false
  },
  {
    id: 'bv-cred-102',
    title: 'AWS Root IAM // GovCloud',
    category: 'logins',
    username: 'root-iam-0914@sovereign-sec.io',
    password: 'k3$T#9Qx!vL1@mP8_Z4w7#eR',
    url: 'https://signin.aws.amazon.com/console',
    tags: ['Cloud', 'Production', 'High-Risk'],
    favorite: true,
    totpSecret: 'HXDMVJECJJWSRB3H',
    notes: 'Hardware YubiKey required for physical token step-up verification.',
    breached: false,
    lastModified: '2026-10-25T09:12:00Z',
    reused: false
  },
  {
    id: 'bv-cred-103',
    title: 'Binance Cold Storage API',
    category: 'keys',
    username: 'api-key-trader-01',
    password: '89bf24de90c2a517e3381a7d65c40e11894a',
    url: 'https://binance.com',
    tags: ['Crypto', 'Finance', 'Withdrawal-Disabled'],
    favorite: false,
    totpSecret: 'NBSWY3DPEHPK3PXP',
    notes: 'IP whitelisted strictly to node SV-EUS-0914 (198.51.100.24).',
    breached: false,
    lastModified: '2026-10-18T18:40:00Z',
    reused: false
  },
  {
    id: 'bv-cred-104',
    title: 'ProtonMail Encrypted Enclave',
    category: 'logins',
    username: 'elena.vance@pm.me',
    password: 'SpringSummer2025!', // intentionally marked weak for security posture demo
    url: 'https://mail.proton.me',
    tags: ['Communications', 'PGP'],
    favorite: false,
    totpSecret: 'GEZDGNBVGY3TQOJQ',
    notes: 'Dual-factor PGP key derivation used for offline messaging.',
    breached: false,
    lastModified: '2026-08-11T11:00:00Z',
    reused: true
  },
  {
    id: 'bv-cred-105',
    title: 'Legacy Internal GitLab',
    category: 'logins',
    username: 'elena.vance@internal.local',
    password: 'SpringSummer2025!', // reused credential demonstration
    url: 'https://gitlab.internal.local',
    tags: ['Legacy', 'Internal'],
    favorite: false,
    totpSecret: '',
    notes: 'Scheduled for decommission next fiscal quarter.',
    breached: true,
    lastModified: '2026-05-10T12:00:00Z',
    reused: true
  },
  {
    id: 'bv-cred-106',
    title: 'Cloudflare Zero Trust Portal',
    category: 'logins',
    username: 'elena.vance@cloudflare-access',
    password: 'z8!L@0P#4vQ9$tM7_1xW3%kY',
    url: 'https://dash.cloudflare.com',
    tags: ['Edge', 'WAF', 'DNS'],
    favorite: true,
    totpSecret: 'KRSXG5CTMVRXEZLU',
    notes: 'Access management for DNSSEC zones and API gateway shields.',
    breached: false,
    lastModified: '2026-10-29T16:15:00Z',
    reused: false
  },
  {
    id: 'bv-cred-107',
    title: 'Hardware Security Module (HSM-Ed25519)',
    category: 'ssh',
    username: 'root@enclave-master-01',
    password: 'ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIG5fXhYv4T8xY1... [client-isolated]',
    url: 'ssh://198.51.100.24:2202',
    tags: ['SSH', 'Infra', 'Ed25519'],
    favorite: false,
    totpSecret: '',
    notes: 'Hardware L3 HSM direct master terminal access.',
    breached: false,
    lastModified: '2026-10-30T03:30:00Z',
    reused: false
  },
  {
    id: 'bv-cred-108',
    title: 'Emergency Master Seed Escrow',
    category: 'notes',
    username: 'shamir-shard-vault-01',
    password: 'lattice orbit beacon quantum zenith matrix phantom sentinel obsidian cascade',
    url: '',
    tags: ['Escrow', 'Recovery', 'Zero-Knowledge'],
    favorite: true,
    totpSecret: '',
    notes: 'Shard 1 of 3 for multi-signature disaster recovery quorum.',
    breached: false,
    lastModified: '2026-10-01T00:00:00Z',
    reused: false
  }
];

// Seed initial Cryptographic Merkle Hash Chain Ledger
const INITIAL_LEDGER = [
  {
    blockId: '#01428',
    timestamp: '2026-10-31 00:22:14',
    action: 'ROTATION_KEY_VERIFIED',
    category: 'ENCLAVE',
    target: 'GitHub Enterprise // DevOps',
    clientNode: 'SV-EUS-0914 (198.51.100.24)',
    hash: '0x8f4c91a02194b6d39e83ca1209b02a9e144',
    prevHash: '0x17b3c4892e01b3e9a740f912e84c98762a1',
    status: 'VERIFIED MERKLE PROOF'
  },
  {
    blockId: '#01427',
    timestamp: '2026-10-30 23:45:02',
    action: 'CREDENTIAL_DECRYPTED',
    category: 'DECRYPT',
    target: 'Cloudflare Zero Trust Portal',
    clientNode: 'SV-EUS-0914 (198.51.100.24)',
    hash: '0x17b3c4892e01b3e9a740f912e84c98762a1',
    prevHash: '0x44c9b207f6e3a981c2d0e7a5b3901a884e2',
    status: 'VERIFIED MERKLE PROOF'
  },
  {
    blockId: '#01426',
    timestamp: '2026-10-30 21:10:48',
    action: 'ENTROPY_SYNTHESIZED',
    category: 'GENERATE',
    target: '256-bit Random Keyspace [Argon2id]',
    clientNode: 'LOCAL_SANDBOX_THREAD',
    hash: '0x44c9b207f6e3a981c2d0e7a5b3901a884e2',
    prevHash: '0x99a0e14bf392d470a1c6e489b03f7a14921',
    status: 'VERIFIED MERKLE PROOF'
  },
  {
    blockId: '#01425',
    timestamp: '2026-10-30 19:04:15',
    action: 'ENCLAVE_SCAN_EXECUTED',
    category: 'SECURITY',
    target: 'Audit Engine v4.18 (Zero-Knowledge)',
    clientNode: 'HSM_L3_ACTIVE',
    hash: '0x99a0e14bf392d470a1c6e489b03f7a14921',
    prevHash: '0x22f1c8490a3e8174b091c6e5a40b938167d',
    status: 'VERIFIED MERKLE PROOF'
  },
  {
    blockId: '#01424',
    timestamp: '2026-10-30 14:15:30',
    action: 'FIDO2_BIOMETRIC_AUTH',
    category: 'AUTH',
    target: 'Dr. Elena Vance (0x89F4...D12A)',
    clientNode: 'SV-EUS-0914 (VERIFIED MTLS)',
    hash: '0x22f1c8490a3e8174b091c6e5a40b938167d',
    prevHash: '0x00000000000000000000000000000000000',
    status: 'GENESIS BLOCK'
  }
];

export const vaultService = {
  // Credentials Management
  getCredentials() {
    const raw = localStorage.getItem(VAULT_STORAGE_KEY);
    if (!raw) {
      localStorage.setItem(VAULT_STORAGE_KEY, JSON.stringify(INITIAL_CREDENTIALS));
      return INITIAL_CREDENTIALS;
    }
    try {
      return JSON.parse(raw);
    } catch {
      return INITIAL_CREDENTIALS;
    }
  },

  saveCredentials(items) {
    localStorage.setItem(VAULT_STORAGE_KEY, JSON.stringify(items));
  },

  addCredential(newItem) {
    const items = this.getCredentials();
    const id = `bv-cred-${Date.now()}`;
    const credential = {
      ...newItem,
      id,
      favorite: newItem.favorite || false,
      lastModified: new Date().toISOString()
    };
    const updated = [credential, ...items];
    this.saveCredentials(updated);

    // Append cryptographic ledger entry
    this.addLedgerEntry({
      action: 'CREDENTIAL_CREATED',
      category: 'CREATE',
      target: credential.title,
      clientNode: 'SV-EUS-0914 (198.51.100.24)'
    });

    return credential;
  },

  updateCredential(id, updates) {
    const items = this.getCredentials();
    const index = items.findIndex(c => c.id === id);
    if (index === -1) return null;

    items[index] = {
      ...items[index],
      ...updates,
      lastModified: new Date().toISOString()
    };
    this.saveCredentials(items);

    this.addLedgerEntry({
      action: 'CREDENTIAL_UPDATED',
      category: 'UPDATE',
      target: items[index].title,
      clientNode: 'SV-EUS-0914 (198.51.100.24)'
    });

    return items[index];
  },

  deleteCredential(id) {
    const items = this.getCredentials();
    const target = items.find(c => c.id === id);
    const filtered = items.filter(c => c.id !== id);
    this.saveCredentials(filtered);

    if (target) {
      this.addLedgerEntry({
        action: 'CREDENTIAL_PURGED',
        category: 'PURGE',
        target: target.title,
        clientNode: 'SV-EUS-0914 (198.51.100.24)'
      });
    }
    return filtered;
  },

  toggleFavorite(id) {
    const items = this.getCredentials();
    const index = items.findIndex(c => c.id === id);
    if (index === -1) return items;
    items[index].favorite = !items[index].favorite;
    this.saveCredentials(items);
    return items;
  },

  // Telemetry & Security Metrics
  getMetrics() {
    const credentials = this.getCredentials();
    const total = credentials.length;
    let strongCount = 0;
    let weakCount = 0;
    let reusedCount = 0;
    let breachedCount = 0;

    // Track password frequencies to detect reused
    const passFreq = {};
    credentials.forEach(c => {
      passFreq[c.password] = (passFreq[c.password] || 0) + 1;
      if (c.breached) breachedCount++;
    });

    credentials.forEach(c => {
      const entropy = calculateEntropy(c.password);
      if (entropy.bits >= 80) {
        strongCount++;
      } else if (entropy.bits < 50) {
        weakCount++;
      }

      if (passFreq[c.password] > 1) {
        reusedCount++;
      }
    });

    // Score computation: scale out of 100
    let postureScore = 100;
    postureScore -= weakCount * 10;
    postureScore -= reusedCount * 5;
    postureScore -= breachedCount * 15;
    if (postureScore < 20) postureScore = 20;

    return {
      total,
      strongCount,
      strongPercent: total > 0 ? Math.round((strongCount / total) * 100) : 0,
      weakCount,
      weakPercent: total > 0 ? Math.round((weakCount / total) * 100) : 0,
      reusedCount,
      reusedPercent: total > 0 ? Math.round((reusedCount / total) * 100) : 0,
      breachedCount,
      postureScore
    };
  },

  // Cryptographic Ledger
  getLedger() {
    const raw = localStorage.getItem(LEDGER_STORAGE_KEY);
    if (!raw) {
      localStorage.setItem(LEDGER_STORAGE_KEY, JSON.stringify(INITIAL_LEDGER));
      return INITIAL_LEDGER;
    }
    try {
      return JSON.parse(raw);
    } catch {
      return INITIAL_LEDGER;
    }
  },

  addLedgerEntry({ action, category, target, clientNode }) {
    const ledger = this.getLedger();
    const topBlock = ledger[0] || { blockId: '#01400', hash: '0x000000000000' };
    const num = parseInt(topBlock.blockId.replace('#', ''), 10) + 1;
    const blockId = `#0${num}`;
    const timestamp = new Date().toISOString().replace('T', ' ').slice(0, 19);
    const hash = quickHash(`${blockId}:${action}:${target}:${Date.now()}`);

    const newEntry = {
      blockId,
      timestamp,
      action,
      category: category || 'SYS',
      target: target || 'Enclave Memory',
      clientNode: clientNode || 'SV-EUS-0914 (198.51.100.24)',
      hash,
      prevHash: topBlock.hash,
      status: 'VERIFIED MERKLE PROOF'
    };

    const updated = [newEntry, ...ledger];
    localStorage.setItem(LEDGER_STORAGE_KEY, JSON.stringify(updated.slice(0, 50)));
    return updated;
  },

  // Vault Lock State
  isVaultLocked() {
    return localStorage.getItem(LOCK_STORAGE_KEY) === 'locked';
  },

  lockVault() {
    localStorage.setItem(LOCK_STORAGE_KEY, 'locked');
  },

  unlockVault() {
    localStorage.setItem(LOCK_STORAGE_KEY, 'unlocked');
    this.addLedgerEntry({
      action: 'VAULT_UNLOCKED_ARGON2ID',
      category: 'AUTH',
      target: 'Master Key Enclave Decryption',
      clientNode: 'SV-EUS-0914 (198.51.100.24)'
    });
  }
};
