/**
 * Add Command
 * Install content package from npm
 */

const chalk = require('chalk');
const { execa } = require('execa');
const ora = require('ora');

async function addCommand(packageName, options) {
  console.log(chalk.blue.bold(`\n📦 Installing content package: ${packageName}\n`));

  const spinner = ora(`Installing ${packageName}...`).start();

  try {
    // Use npm install to add the package
    await execa('npm', ['install', packageName], {
      stdio: 'inherit',
    });

    spinner.succeed(`Installed ${packageName}!`);

    console.log(chalk.green(`\n✅ Package ${chalk.bold(packageName)} added successfully!\n`));
    console.log(chalk.cyan('Next steps:'));
    console.log(chalk.gray(`  Check node_modules/${packageName}/assets/`));
    console.log(chalk.gray(`  Run 'sakku build' to process assets\n`));
  } catch (error) {
    spinner.fail('Installation failed!');
    console.error(chalk.red('Error:'), error.message);
    process.exit(1);
  }
}

module.exports = addCommand;
