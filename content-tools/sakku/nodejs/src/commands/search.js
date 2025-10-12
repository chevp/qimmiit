/**
 * Search Command
 * Search for content packages on npm
 */

const chalk = require('chalk');
const { execa } = require('execa');
const ora = require('ora');

async function searchCommand(query, options) {
  console.log(chalk.blue.bold(`\n🔍 Searching for: ${query}\n`));

  const spinner = ora('Searching npm registry...').start();

  try {
    // Use npm search
    const result = await execa('npm', ['search', query, '--json'], {
      stdio: 'pipe',
    });

    spinner.succeed('Search completed!');

    const packages = JSON.parse(result.stdout);

    if (packages.length === 0) {
      console.log(chalk.yellow('\n⚠️  No packages found.\n'));
      return;
    }

    // Filter by tag if specified
    let filteredPackages = packages;
    if (options.tag) {
      filteredPackages = packages.filter(pkg =>
        pkg.keywords && pkg.keywords.includes(options.tag)
      );
    }

    // Apply pagination
    const page = parseInt(options.page) || 1;
    const limit = parseInt(options.limit) || 10;
    const start = (page - 1) * limit;
    const end = start + limit;
    const paginatedPackages = filteredPackages.slice(start, end);

    console.log(chalk.green(`\n✅ Found ${filteredPackages.length} packages:\n`));

    paginatedPackages.forEach((pkg, index) => {
      const number = start + index + 1;
      console.log(chalk.bold(`${number}. ${pkg.name}`) + chalk.gray(` v${pkg.version}`));
      if (pkg.description) {
        console.log(chalk.gray(`   ${pkg.description}`));
      }
      if (pkg.keywords && pkg.keywords.length > 0) {
        console.log(chalk.cyan(`   Tags: ${pkg.keywords.join(', ')}`));
      }
      console.log();
    });

    // Show pagination info
    const totalPages = Math.ceil(filteredPackages.length / limit);
    if (totalPages > 1) {
      console.log(chalk.gray(`Page ${page} of ${totalPages}`));
      console.log(chalk.gray(`Use --page <n> to view other pages\n`));
    }

    console.log(chalk.cyan('To install a package:'));
    console.log(chalk.gray(`  sakku add <package-name>\n`));
  } catch (error) {
    spinner.fail('Search failed!');
    console.error(chalk.red('Error:'), error.message);
    process.exit(1);
  }
}

module.exports = searchCommand;
