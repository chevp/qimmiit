/**
 * Sakku - Content Management CLI
 * Programmatic API
 */

const NativeBridge = require('./native-bridge');
const { getNativeBinaryPath } = require('./utils/platform');

module.exports = {
  // Native bridge
  NativeBridge,

  // Platform utilities
  getNativeBinaryPath,

  // Version
  version: require('../package.json').version,
};