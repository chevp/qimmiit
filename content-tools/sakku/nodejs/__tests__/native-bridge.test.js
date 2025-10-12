/**
 * Native Bridge Tests
 */

const NativeBridge = require('../src/native-bridge');
const { execa } = require('execa');

// Mock execa
jest.mock('execa');

describe('NativeBridge', () => {
  let bridge;

  beforeEach(() => {
    bridge = new NativeBridge();
    jest.clearAllMocks();
  });

  describe('execute', () => {
    it('should execute native binary successfully', async () => {
      execa.mockResolvedValue({
        stdout: 'Success',
        stderr: '',
        exitCode: 0,
      });

      const result = await bridge.execute(['--version'], { showSpinner: false });

      expect(result.success).toBe(true);
      expect(result.stdout).toBe('Success');
      expect(execa).toHaveBeenCalledWith(
        expect.stringContaining('sakku-cli'),
        ['--version'],
        expect.any(Object)
      );
    });

    it('should handle execution errors', async () => {
      execa.mockRejectedValue({
        message: 'Binary not found',
        stdout: '',
        stderr: 'Error',
        exitCode: 1,
      });

      const result = await bridge.execute(['--version'], { showSpinner: false });

      expect(result.success).toBe(false);
      expect(result.error).toBe('Binary not found');
    });
  });

  describe('build', () => {
    it('should call build with correct arguments', async () => {
      execa.mockResolvedValue({
        stdout: 'Build successful',
        stderr: '',
        exitCode: 0,
      });

      await bridge.build({ optimize: true, compress: true });

      expect(execa).toHaveBeenCalledWith(
        expect.stringContaining('sakku-cli'),
        ['build', '--optimize', '--compress'],
        expect.any(Object)
      );
    });
  });

  describe('processGLTF', () => {
    it('should process GLTF with correct arguments', async () => {
      execa.mockResolvedValue({
        stdout: 'GLTF processed',
        stderr: '',
        exitCode: 0,
      });

      await bridge.processGLTF('input.gltf', 'output.gltf', { optimize: true });

      expect(execa).toHaveBeenCalledWith(
        expect.stringContaining('sakku-cli'),
        ['process-gltf', 'input.gltf', 'output.gltf', '--optimize'],
        expect.any(Object)
      );
    });
  });

  describe('compressTexture', () => {
    it('should compress texture with correct arguments', async () => {
      execa.mockResolvedValue({
        stdout: 'Texture compressed',
        stderr: '',
        exitCode: 0,
      });

      await bridge.compressTexture('input.png', 'output.bc7', {
        format: 'bc7',
        quality: 'high',
      });

      expect(execa).toHaveBeenCalledWith(
        expect.stringContaining('sakku-cli'),
        ['compress-texture', 'input.png', 'output.bc7', '--format', 'bc7', '--quality', 'high'],
        expect.any(Object)
      );
    });
  });

  describe('getVersion', () => {
    it('should return binary version', async () => {
      execa.mockResolvedValue({
        stdout: 'sakku-cli v1.0.0\n',
        stderr: '',
        exitCode: 0,
      });

      const version = await bridge.getVersion();

      expect(version).toBe('sakku-cli v1.0.0');
    });
  });
});
