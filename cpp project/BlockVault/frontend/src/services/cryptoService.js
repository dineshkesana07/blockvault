// Cryptographic & Entropy Utilities for BlockVault

// Character Sets
export const CHARSETS = {
  uppercase: 'ABCDEFGHJKLMNPQRSTUVWXYZ',
  uppercaseWithAmbiguous: 'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
  lowercase: 'abcdefghijkmnopqrstuvwxyz',
  lowercaseWithAmbiguous: 'abcdefghijklmnopqrstuvwxyz',
  numbers: '23456789',
  numbersWithAmbiguous: '0123456789',
  symbols: '!@#$%^&*()_+-=[]{}|;:,.<>?'
};

// Wordlist for Diceware Passphrases
const WORDLIST = [
  'cipher', 'quantum', 'argon', 'enclave', 'shannon', 'entropy', 'merkle', 'ledger',
  'vertex', 'sovereign', 'nebula', 'protocol', 'poly1305', 'lattice', 'synapse',
  'orbital', 'strata', 'plasma', 'glacier', 'shield', 'obsidian', 'zero', 'vault',
  'cascade', 'monolith', 'matrix', 'vector', 'neutron', 'stellar', 'tactical',
  'sentinel', 'phantom', 'beacon', 'spectrum', 'zenith', 'pulse', 'circuit'
];

/**
 * Generate a cryptographically secure random string
 */
export function generatePassword({
  length = 24,
  useUpper = true,
  useLower = true,
  useNumbers = true,
  useSymbols = true,
  avoidAmbiguous = true,
  mode = 'random', // 'random' | 'passphrase'
  wordCount = 5,
  separator = '-'
}) {
  if (mode === 'passphrase') {
    const words = [];
    const array = new Uint32Array(wordCount);
    window.crypto.getRandomValues(array);
    for (let i = 0; i < wordCount; i++) {
      const index = array[i] % WORDLIST.length;
      words.push(WORDLIST[index]);
    }
    return words.join(separator);
  }

  let pool = '';
  if (useUpper) pool += avoidAmbiguous ? CHARSETS.uppercase : CHARSETS.uppercaseWithAmbiguous;
  if (useLower) pool += avoidAmbiguous ? CHARSETS.lowercase : CHARSETS.lowercaseWithAmbiguous;
  if (useNumbers) pool += avoidAmbiguous ? CHARSETS.numbers : CHARSETS.numbersWithAmbiguous;
  if (useSymbols) pool += CHARSETS.symbols;

  if (!pool) pool = CHARSETS.lowercase;

  const array = new Uint32Array(length);
  window.crypto.getRandomValues(array);

  let result = '';
  for (let i = 0; i < length; i++) {
    result += pool[array[i] % pool.length];
  }

  return result;
}

/**
 * Calculates Shannon Entropy and strength score
 */
export function calculateEntropy(password = '') {
  if (!password) {
    return {
      bits: 0,
      score: 0,
      label: 'EMPTY',
      color: 'text-outline',
      crackTime: 'Instantaneous'
    };
  }

  let poolSize = 0;
  if (/[a-z]/.test(password)) poolSize += 26;
  if (/[A-Z]/.test(password)) poolSize += 26;
  if (/[0-9]/.test(password)) poolSize += 10;
  if (/[^a-zA-Z0-9]/.test(password)) poolSize += 33;

  if (poolSize === 0) poolSize = 1;

  // Bits of Entropy = Length * log2(Pool Size)
  const bits = Math.round(password.length * Math.log2(poolSize) * 10) / 10;

  if (bits < 40) {
    return {
      bits,
      score: 20,
      label: 'VERY WEAK',
      color: 'text-error',
      bgColor: 'bg-error',
      crackTime: 'Seconds'
    };
  } else if (bits < 60) {
    return {
      bits,
      score: 45,
      label: 'MODERATE',
      color: 'text-tertiary',
      bgColor: 'bg-tertiary',
      crackTime: '3 Months'
    };
  } else if (bits < 90) {
    return {
      bits,
      score: 80,
      label: 'STRONG',
      color: 'text-secondary',
      bgColor: 'bg-secondary',
      crackTime: '1,400 Centuries'
    };
  } else {
    return {
      bits,
      score: 100,
      label: 'QUANTUM-RESISTANT',
      color: 'text-primary',
      bgColor: 'bg-primary',
      crackTime: '8.4 Billion Years'
    };
  }
}

/**
 * Generates SHA-256 Hex Digest using Web Crypto API
 */
export async function sha256Hex(message) {
  const msgUint8 = new TextEncoder().encode(message);
  const hashBuffer = await window.crypto.subtle.digest('SHA-256', msgUint8);
  const hashArray = Array.from(new Uint8Array(hashBuffer));
  return hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
}

/**
 * Generates synchronous simulated SHA-256 / hash for quick rendering
 */
export function quickHash(input) {
  let hash = 0x811c9dc5;
  for (let i = 0; i < input.length; i++) {
    hash ^= input.charCodeAt(i);
    hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
  }
  const hex = (hash >>> 0).toString(16).padStart(8, '0');
  return `0x${hex}7f9b2d${hex.slice(0, 4)}e148`;
}

/**
 * Generates pseudo TOTP (Time-Based One-Time Password)
 */
export function getSimulatedTOTP(secretKey, step = 30) {
  const epoch = Math.floor(Date.now() / 1000);
  const counter = Math.floor(epoch / step);
  const remaining = step - (epoch % step);
  
  // Seeded hash based on counter + secret
  let hashVal = 0;
  const str = `${secretKey}:${counter}`;
  for (let i = 0; i < str.length; i++) {
    hashVal = (hashVal << 5) - hashVal + str.charCodeAt(i);
    hashVal |= 0;
  }
  const code = Math.abs(hashVal % 1000000).toString().padStart(6, '0');

  return { code, remaining };
}
