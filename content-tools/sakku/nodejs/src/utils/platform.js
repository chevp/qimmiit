/**
 * Platform Utilities
 * Detect platform and return correct native binary path
 */

const path = require('path');
const fs = require('fs');

/**
 * Get the platform-specific binary name
 * @returns {string} - Binary filename
 */
function getBinaryName() {
  return process.platform === 'win32' ? 'sakku-cli.exe' : 'sakku-cli';
}

/**
 * Get the platform-specific directory name
 * @returns {string} - Platform directory (e.g., 'win32-x64')
 */
function getPlatformDir() {
  const platform = process.platform;
  const arch = process.arch;
  return `${platform}-${arch}`;
}

/**
 * Get the full path to the native binary
 * @returns {string} - Absolute path to binary
 */
function getNativeBinaryPath() {
  const binaryName = getBinaryName();
  const platformDir = getPlatformDir();

  // Path relative to this file: ../../binaries/<platform>/<binary>
  const binaryPath = path.join(
    __dirname,
    '..',
    '..',
    'binaries',
    platformDir,
    binaryName
  );

  // Verify binary exists
  if (!fs.existsSync(binaryPath)) {
    throw new Error(
      `Native binary not found for ${platformDir}.\n` +
      `Expected at: ${binaryPath}\n` +
      `Please ensure the correct binaries are bundled with this package.`
    );
  }

  // Verify binary is executable (Unix-like systems)
  if (process.platform !== 'win32') {
    try {
      fs.accessSync(binaryPath, fs.constants.X_OK);
    } catch (err) {
      // Make executable if not already
      fs.chmodSync(binaryPath, 0o755);
    }
  }

  return binaryPath;
}

/**
 * Check if current platform is supported
 * @returns {boolean}
 */
function isPlatformSupported() {
  const supported = [
    'win32-x64',
    'linux-x64',
    'darwin-x64',
    'darwin-arm64',
  ];

  return supported.includes(getPlatformDir());
}

module.exports = {
  getBinaryName,
  getPlatformDir,
  getNativeBinaryPath,
  isPlatformSupported,
};