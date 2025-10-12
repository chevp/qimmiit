/**
 * Publish Command
 * Publish content package to npm
 */

const chalk = require('chalk');
const { execa } = require('execa');
const ora = require('ora');
const fs = require('fs').promises;
const path = require('path');

async function publishCommand(options) {
  console.log(chalk.blue.bold('\n🚀 Publishing content package...\n'));

  // Validate package.json exists
  const packageJsonPath = path.join(process.cwd(), 'package.json');

  try {
    await fs.access(packageJsonPath);
  } catch {
    console.error(chalk.red('❌ No package.json found in current directory!'));
    process.exit(1);
  }

  // Read and validate package.json
  const packageJson = JSON.parse(await fs.readFile(packageJsonPath, 'utf-8'));

  if (!packageJson.name) {
    console.error(chalk.red('❌ package.json must have a "name" field!'));
    process.exit(1);
  }

  if (!packageJson.version) {
    console.error(chalk.red('❌ package.json must have a "version" field!'));
    process.exit(1);
  }

  console.log(chalk.cyan(`  📦 Package: ${packageJson.name}`));
  console.log(chalk.cyan(`  📌 Version: ${packageJson.version}\n`));

  if (options.dryRun) {
    console.log(chalk.yellow('  🧪 Dry run mode - no actual publish\n'));
  }

  const spinner = ora('Publishing to npm...').start();

  try {
    const args = ['publish'];

    if (options.dryRun) {
      args.push('--dry-run');
    }

    await execa('npm', args, {
      stdio: 'inherit',
    });

    spinner.succeed('Published successfully!');

    console.log(chalk.green(`\n✅ Package ${chalk.bold(packageJson.name)}@${packageJson.version} published!\n`));
  } catch (error) {
    spinner.fail('Publish failed!');
    console.error(chalk.red('Error:'), error.message);
    process.exit(1);
  }
}

module.exports = publishCommand;
