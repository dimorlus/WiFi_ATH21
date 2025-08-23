# Contributing to WiFi_ATH21

Thank you for your interest in contributing to the WiFi_ATH21 project! This document provides guidelines for contributing to this IoT temperature and humidity monitoring system.

## How to Contribute

### Reporting Issues

1. **Search existing issues** first to avoid duplicates
2. **Use issue templates** when available
3. **Provide detailed information**:
   - Hardware setup (ESP8266 model, wiring)
   - Software versions (SDK, compiler, OS)
   - Steps to reproduce the issue
   - Expected vs actual behavior
   - Error messages or logs

### Submitting Changes

1. **Fork the repository**
2. **Create a feature branch** from `main`
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes** following the coding standards
4. **Test thoroughly** on both ESP8266 and PC components
5. **Commit with clear messages**
   ```bash
   git commit -m "Add feature: brief description of changes"
   ```
6. **Submit a pull request**

## Development Setup

### ESP8266 Development

1. **Install ESP8266 SDK** v0.9.4 or higher
2. **Set up toolchain**:
   - Windows: MinGW with Xtensa tools
   - Linux/Mac: GCC with Xtensa cross-compiler
3. **Build environment**:
   ```bash
   cd ESP/mqtt_aht21
   make SDK_BASE="/path/to/esp8266/sdk" all
   ```

### PC Application Development

1. **Install dependencies**:
   - CMake 3.15+
   - C++17 compatible compiler
   - vcpkg (for Windows)
2. **Build the application**:
   ```bash
   cd PC/mqtt_logger
   mkdir build && cd build
   cmake ..
   make
   ```

## Coding Standards

### C Code (ESP8266)

- Use K&R brace style
- 4-space indentation
- Meaningful variable and function names
- Comment complex logic
- Avoid magic numbers - use #define constants

Example:
```c
#define SENSOR_READ_INTERVAL_MS 30000

static void ICACHE_FLASH_ATTR sensor_timer_cb(void *arg) {
    // Read sensor data every 30 seconds
    aht21_read_data();
}
```

### C++ Code (PC Application)

- Use modern C++17 features
- 4-space indentation
- Follow standard naming conventions
- Use RAII principles
- Add unit tests for new functionality

Example:
```cpp
class DataLogger {
public:
    explicit DataLogger(const std::string& dataDir);
    bool logData(const std::string& deviceId, const std::string& data);
    
private:
    std::string dataDirectory_;
    std::mutex fileMutex_;
};
```

### Documentation

- Update README.md for new features
- Add inline comments for complex algorithms
- Document configuration parameters
- Include usage examples

## Testing Guidelines

### ESP8266 Testing

1. **Hardware testing** on real devices
2. **WiFi connectivity** in different environments
3. **MQTT reliability** over extended periods
4. **Power consumption** measurements
5. **Temperature range** testing (-10°C to +50°C)

### PC Application Testing

1. **Unit tests** for core functionality
2. **Integration tests** with real MQTT broker
3. **File I/O testing** with various permissions
4. **Multi-device scenarios**
5. **Configuration validation**

## Pull Request Guidelines

### PR Title and Description

- **Clear, descriptive title** 
- **Detailed description** of changes
- **Reference issues** being fixed
- **Include screenshots** for UI changes
- **List breaking changes** if any

### PR Checklist

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] Documentation updated
- [ ] Follows coding standards
- [ ] No sensitive data (passwords, keys) committed
- [ ] Build artifacts excluded (.gitignore updated)

## Code Review Process

1. **Automated checks** must pass (if configured)
2. **Manual review** by maintainers
3. **Testing verification** on hardware
4. **Documentation review**
5. **Approval and merge**

## Release Process

### Version Numbering

We use [Semantic Versioning](https://semver.org/):
- **MAJOR**: Incompatible API changes
- **MINOR**: Backwards-compatible functionality
- **PATCH**: Backwards-compatible bug fixes

### Release Checklist

1. Update version numbers
2. Update CHANGELOG.md
3. Test on target hardware
4. Create GitHub release
5. Update documentation

## Getting Help

- **GitHub Discussions**: For general questions
- **GitHub Issues**: For bugs and feature requests
- **Pull Request Comments**: For code review discussions

## Code of Conduct

Please be respectful and constructive in all interactions. We aim to maintain a welcoming environment for all contributors regardless of experience level.

## License

By contributing to this project, you agree that your contributions will be licensed under the MIT License.