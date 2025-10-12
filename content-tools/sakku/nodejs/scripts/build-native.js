#!/usr/bin/env node

/**
 * Build Native Binary
 * Compiles the C++ binary for the current platform and copies it to binaries/
 */

const { execa } = require('execa');
const fs = require('fs').promises;
const path = require('path');
const chalk = require('chalk');
const ora = require('ora');

const NATIVE_DIR = path.join(__dirname, '..', '..', 'native');
const BUILD_DIR = path.join(NATIVE_DIR, 'build');
const BINARIES_DIR = path.join(__dirname, '..', 'binaries');

// Platform detection
const platform = process.platform;
const arch = process.arch;
const platformDir = `${platform}-${arch}`;
const binaryName = platform === 'win32' ? 'sakku-cli.exe' : 'sakku-cli';

async function checkDependencies() {
  console.log(chalk.blue('🔍 Checking dependencies...\n'));

  // Check CMake
  try {
    const { stdout } = await execa('cmake', ['--version']);
    console.log(chalk.green('✓ CMake:'), stdout.split('\n')[0]);
  } catch {
    console.error(chalk.red('✗ CMake not found!'));
    console.error(chalk.yellow('  Install CMake 3.20+ from https://cmake.org'));
    process.exit(1);
  }

  // Check vcpkg (optional but recommended)
  try {
    await execa('vcpkg', ['version']);
    console.log(chalk.green('✓ vcpkg: installed'));
  } catch {
    console.log(chalk.yellow('⚠ vcpkg not found (optional but recommended)'));
    console.log(chalk.gray('  Install from https://vcpkg.io'));
  }

  console.log();
}

async function configureCMake() {
  const spinner = ora('Configuring CMake...').start();

  try {
    const args = ['-B', BUILD_DIR, '-S', NATIVE_DIR];

    // Platform-specific configuration
    if (platform === 'win32') {
      args.push('-G', 'Visual Studio 17 2022');
      args.push('-A', arch === 'x64' ? 'x64' : 'Win32');
    } else {
      args.push('-DCMAKE_BUILD_TYPE=Release');
    }

    // Use vcpkg if available
    const vcpkgRoot = process.env.VCPKG_ROOT;
    if (vcpkgRoot) {
      args.push(`-DCMAKE_TOOLCHAIN_FILE=${vcpkgRoot}/scripts/buildsystems/vcpkg.cmake`);
    }

    await execa('cmake', args, {
      stdio: 'pipe',
      cwd: NATIVE_DIR,
    });

    spinner.succeed('CMake configured');
  } catch (error) {
    spinner.fail('CMake configuration failed');
    console.error(chalk.red('\nError:'), error.message);
    if (error.stderr) {
      console.error(chalk.red(error.stderr));
    }
    process.exit(1);
  }
}

async function buildBinary() {
  const spinner = ora('Building native binary...').start();

  try {
    const args = ['--build', BUILD_DIR, '--config', 'Release'];

    // Parallel build
    if (platform !== 'win32') {
      args.push('--parallel');
    }

    await execa('cmake', args, {
      stdio: 'pipe',
      cwd: NATIVE_DIR,
    });

    spinner.succeed('Binary built successfully');
  } catch (error) {
    spinner.fail('Build failed');
    console.error(chalk.red('\nError:'), error.message);
    if (error.stderr) {
      console.error(chalk.red(error.stderr));
    }
    process.exit(1);
  }
}

async function copyBinary() {
  const spinner = ora('Copying binary to binaries directory...').start();

  try {
    // Find the built binary
    let builtBinaryPath;
    if (platform === 'win32') {
      builtBinaryPath = path.join(BUILD_DIR, 'Release', binaryName);
    } else {
      builtBinaryPath = path.join(BUILD_DIR, binaryName);
    }

    // Check if binary exists
    try {
      await fs.access(builtBinaryPath);
    } catch {
      throw new Error(`Binary not found at ${builtBinaryPath}`);
    }

    // Create platform directory
    const targetDir = path.join(BINARIES_DIR, platformDir);
    await fs.mkdir(targetDir, { recursive: true });

    // Copy binary
    const targetPath = path.join(targetDir, binaryName);
    await fs.copyFile(builtBinaryPath, targetPath);

    // Make executable on Unix
    if (platform !== 'win32') {
      await fs.chmod(targetPath, 0o755);
    }

    spinner.succeed(`Binary copied to ${chalk.cyan(path.relative(process.cwd(), targetPath))}`);
  } catch (error) {
    spinner.fail('Failed to copy binary');
    console.error(chalk.red('\nError:'), error.message);
    process.exit(1);
  }
}

async function verifyBinary() {
  const spinner = ora('Verifying binary...').start();

  try {
    const binaryPath = path.join(BINARIES_DIR, platformDir, binaryName);

    const { stdout } = await execa(binaryPath, ['--version'], {
      stdio: 'pipe',
    });

    spinner.succeed(`Binary verified: ${chalk.cyan(stdout.trim())}`);
  } catch (error) {
    spinner.fail('Binary verification failed');
    console.error(chalk.red('\nError:'), error.message);
    process.exit(1);
  }
}

async function main() {
  console.log(chalk.bold.blue('\n🔨 Building Sakku Native Binary\n'));
  console.log(chalk.gray(`Platform: ${platformDir}`));
  console.log(chalk.gray(`Binary: ${binaryName}\n`));

  await checkDependencies();
  await configureCMake();
  await buildBinary();
  await copyBinary();
  await verifyBinary();

  console.log(chalk.green.bold('\n✅ Build completed successfully!\n'));
  console.log(chalk.cyan('Next steps:'));
  console.log(chalk.gray('  npm test          # Run tests'));
  console.log(chalk.gray('  npm publish       # Publish to npm\n'));
}

main().catch((error) => {
  console.error(chalk.red('\n❌ Build failed:'), error.message);
  process.exit(1);
});
