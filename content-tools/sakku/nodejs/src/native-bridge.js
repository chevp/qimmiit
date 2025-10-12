/**
 * Native Bridge
 * Spawns and communicates with the native sakku-cli.exe binary
 */

const { execa } = require('execa');
const chalk = require('chalk');
const ora = require('ora');
const { getNativeBinaryPath } = require('./utils/platform');

class NativeBridge {
  constructor() {
    this.binaryPath = getNativeBinaryPath();
  }

  /**
   * Execute native binary with arguments
   * @param {string[]} args - Command line arguments
   * @param {Object} options - Execution options
   * @returns {Promise<Object>} - Execution result
   */
  async execute(args, options = {}) {
    const { showSpinner = true, spinnerText = 'Processing...' } = options;

    const spinner = showSpinner ? ora(spinnerText).start() : null;

    try {
      const result = await execa(this.binaryPath, args, {
        stdio: options.stdio || 'pipe',
        cwd: options.cwd || process.cwd(),
      });

      if (spinner) {
        spinner.succeed('Done!');
      }

      return {
        success: true,
        stdout: result.stdout,
        stderr: result.stderr,
        exitCode: result.exitCode,
      };
    } catch (error) {
      if (spinner) {
        spinner.fail('Failed!');
      }

      console.error(chalk.red('Native binary error:'), error.message);

      return {
        success: false,
        error: error.message,
        stdout: error.stdout,
        stderr: error.stderr,
        exitCode: error.exitCode,
      };
    }
  }

  /**
   * Build assets using native binary
   */
  async build(options = {}) {
    const args = ['build'];

    if (options.optimize) args.push('--optimize');
    if (options.compress) args.push('--compress');
    if (options.output) args.push('--output', options.output);

    return this.execute(args, {
      spinnerText: 'Building assets...',
      ...options,
    });
  }

  /**
   * Process GLTF model
   */
  async processGLTF(inputPath, outputPath, options = {}) {
    const args = ['process-gltf', inputPath, outputPath];

    if (options.optimize) args.push('--optimize');

    return this.execute(args, {
      spinnerText: `Processing ${inputPath}...`,
      ...options,
    });
  }

  /**
   * Compress texture
   */
  async compressTexture(inputPath, outputPath, options = {}) {
    const args = ['compress-texture', inputPath, outputPath];

    if (options.format) args.push('--format', options.format);
    if (options.quality) args.push('--quality', options.quality);

    return this.execute(args, {
      spinnerText: `Compressing ${inputPath}...`,
      ...options,
    });
  }

  /**
   * Get native binary version
   */
  async getVersion() {
    const result = await this.execute(['--version'], { showSpinner: false });
    return result.stdout?.trim();
  }
}

module.exports = NativeBridge;