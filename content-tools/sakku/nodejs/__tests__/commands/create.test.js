/**
 * Create Command Tests
 */

const fs = require('fs').promises;
const path = require('path');
const createCommand = require('../../src/commands/create');

// Mock fs
jest.mock('fs', () => ({
  promises: {
    access: jest.fn(),
    mkdir: jest.fn(),
    writeFile: jest.fn(),
  },
}));

describe('Create Command', () => {
  const projectName = 'test-project';
  const options = { template: 'minimal' };

  beforeEach(() => {
    jest.clearAllMocks();
    // Mock directory doesn't exist
    fs.access.mockRejectedValue(new Error('Directory not found'));
  });

  it('should create project structure', async () => {
    await createCommand(projectName, options);

    // Check if directories were created
    expect(fs.mkdir).toHaveBeenCalledWith(
      expect.stringContaining(projectName),
      expect.any(Object)
    );

    // Check if package.json was created
    expect(fs.writeFile).toHaveBeenCalledWith(
      expect.stringContaining('package.json'),
      expect.stringContaining(projectName)
    );

    // Check if qimmiit.json was created
    expect(fs.writeFile).toHaveBeenCalledWith(
      expect.stringContaining('qimmiit.json'),
      expect.any(String)
    );

    // Check if README.md was created
    expect(fs.writeFile).toHaveBeenCalledWith(
      expect.stringContaining('README.md'),
      expect.any(String)
    );
  });

  it('should fail if directory already exists', async () => {
    // Mock directory exists
    fs.access.mockResolvedValue();

    // Mock process.exit
    const mockExit = jest.spyOn(process, 'exit').mockImplementation();

    await createCommand(projectName, options);

    expect(mockExit).toHaveBeenCalledWith(1);

    mockExit.mockRestore();
  });
});
