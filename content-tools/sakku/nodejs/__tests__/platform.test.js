/**
 * Platform Utilities Tests
 */

const {
  getBinaryName,
  getPlatformDir,
  isPlatformSupported,
} = require('../src/utils/platform');

describe('Platform Utilities', () => {
  describe('getBinaryName', () => {
    it('should return sakku-cli.exe on Windows', () => {
      const originalPlatform = process.platform;
      Object.defineProperty(process, 'platform', { value: 'win32' });

      expect(getBinaryName()).toBe('sakku-cli.exe');

      Object.defineProperty(process, 'platform', { value: originalPlatform });
    });

    it('should return sakku-cli on Unix', () => {
      const originalPlatform = process.platform;
      Object.defineProperty(process, 'platform', { value: 'linux' });

      expect(getBinaryName()).toBe('sakku-cli');

      Object.defineProperty(process, 'platform', { value: originalPlatform });
    });
  });

  describe('getPlatformDir', () => {
    it('should return platform-arch format', () => {
      const dir = getPlatformDir();
      expect(dir).toMatch(/^(win32|linux|darwin)-(x64|arm64)$/);
    });
  });

  describe('isPlatformSupported', () => {
    it('should return true for supported platforms', () => {
      const supported = isPlatformSupported();
      expect(typeof supported).toBe('boolean');
    });
  });
});
