/**
 * Build Command
 * Build and optimize game assets using native binary
 */

const chalk = require('chalk');
const NativeBridge = require('../native-bridge');

async function buildCommand(options) {
  console.log(chalk.blue.bold('\n🛠️  Building game assets...\n'));

  if (options.optimize) {
    console.log(chalk.cyan('  ⚡ Optimization: enabled'));
  }
  if (options.compress) {
    console.log(chalk.cyan('  🗜️  Compression: enabled'));
  }
  if (options.watch) {
    console.log(chalk.cyan('  👀 Watch mode: enabled'));
  }

  console.log();

  const bridge = new NativeBridge();

  try {
    // Call native binary
    const result = await bridge.build({
      optimize: options.optimize,
      compress: options.compress,
      watch: options.watch,
    });

    if (result.success) {
      console.log(chalk.green('\n✅ Build completed successfully!\n'));

      if (result.stdout) {
        console.log(chalk.gray('Output:'));
        console.log(result.stdout);
      }
    } else {
      console.error(chalk.red('\n❌ Build failed!\n'));

      if (result.stderr) {
        console.error(chalk.red('Error:'));
        console.error(result.stderr);
      }

      process.exit(1);
    }
  } catch (error) {
    console.error(chalk.red('\n❌ Build error:'), error.message);
    process.exit(1);
  }
}

module.exports = buildCommand;
