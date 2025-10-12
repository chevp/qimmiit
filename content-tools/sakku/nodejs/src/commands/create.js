/**
 * Create Command
 * Scaffold a new game project
 */

const chalk = require('chalk');
const inquirer = require('inquirer');
const fs = require('fs').promises;
const path = require('path');
const ora = require('ora');

async function createCommand(projectName, options) {
  console.log(chalk.blue.bold(`\n🎮 Creating new game project: ${projectName}\n`));

  const projectPath = path.join(process.cwd(), projectName);

  // Check if directory already exists
  try {
    await fs.access(projectPath);
    console.error(chalk.red(`❌ Directory ${projectName} already exists!`));
    process.exit(1);
  } catch {
    // Directory doesn't exist, continue
  }

  const spinner = ora('Creating project structure...').start();

  try {
    // Create project directory
    await fs.mkdir(projectPath, { recursive: true });

    // Create package.json
    const packageJson = {
      name: projectName,
      version: '1.0.0',
      description: `Game project created with Sakku`,
      main: 'index.js',
      scripts: {
        build: 'sakku build',
        'build:optimize': 'sakku build --optimize --compress',
      },
      dependencies: {},
      devDependencies: {
        '@qimmiit/sakku': '^1.0.0',
      },
    };

    await fs.writeFile(
      path.join(projectPath, 'package.json'),
      JSON.stringify(packageJson, null, 2)
    );

    // Create directory structure
    await fs.mkdir(path.join(projectPath, 'assets'), { recursive: true });
    await fs.mkdir(path.join(projectPath, 'assets', 'models'), { recursive: true });
    await fs.mkdir(path.join(projectPath, 'assets', 'textures'), { recursive: true });
    await fs.mkdir(path.join(projectPath, 'assets', 'materials'), { recursive: true });

    // Create qimmiit.json config
    const qimmiitConfig = {
      type: 'game-project',
      engine: '^1.0.0',
      template: options.template,
      assets: {
        models: 'assets/models',
        textures: 'assets/textures',
        materials: 'assets/materials',
      },
      build: {
        output: 'dist',
        optimize: true,
      },
    };

    await fs.writeFile(
      path.join(projectPath, 'qimmiit.json'),
      JSON.stringify(qimmiitConfig, null, 2)
    );

    // Create README
    const readme = `# ${projectName}

Game project created with Sakku.

## Getting Started

\`\`\`bash
# Install dependencies
npm install

# Add content packages
sakku add @qimmiit-content/starter-pack

# Build assets
npm run build

# Build with optimization
npm run build:optimize
\`\`\`

## Project Structure

- \`assets/\` - Game assets
  - \`models/\` - 3D models
  - \`textures/\` - Textures
  - \`materials/\` - Materials
- \`qimmiit.json\` - Qimmiit configuration
`;

    await fs.writeFile(path.join(projectPath, 'README.md'), readme);

    spinner.succeed('Project created successfully!');

    console.log(chalk.green(`\n✅ Project ${chalk.bold(projectName)} created!\n`));
    console.log(chalk.cyan('Next steps:'));
    console.log(chalk.gray(`  cd ${projectName}`));
    console.log(chalk.gray(`  npm install`));
    console.log(chalk.gray(`  sakku add @qimmiit-content/starter-pack`));
    console.log(chalk.gray(`  npm run build\n`));
  } catch (error) {
    spinner.fail('Failed to create project!');
    console.error(chalk.red('Error:'), error.message);
    process.exit(1);
  }
}

module.exports = createCommand;
