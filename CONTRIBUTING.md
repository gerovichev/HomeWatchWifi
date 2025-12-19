# Contributing to HomeWatchWifi

Thank you for your interest in contributing to HomeWatchWifi! This document provides guidelines and instructions for contributing.

## Code of Conduct

- Be respectful and considerate
- Welcome newcomers and help them learn
- Focus on constructive feedback
- Respect different viewpoints and experiences

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in the Issues section
2. If not, create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Hardware/software versions
   - Relevant logs or error messages

### Suggesting Features

1. Check if the feature has already been suggested
2. Create a new issue with:
   - Clear description of the feature
   - Use case and benefits
   - Possible implementation approach (if you have ideas)

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature-name`)
3. Make your changes following the coding standards
4. Test your changes thoroughly
5. Commit with clear, descriptive messages
6. Push to your fork (`git push origin feature/your-feature-name`)
7. Create a Pull Request with:
   - Clear description of changes
   - Reference to related issues (if any)
   - Testing notes

## Coding Standards

### Code Style

- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and small
- Follow existing code patterns
- Use consistent indentation (2 spaces)

### Code Comments

- Write comments in English
- Explain "why" not "what" (code should be self-documenting)
- Update comments when code changes
- Use `//` for single-line comments
- Use `/* */` for multi-line comments

### Memory Management

- Minimize String object creation
- Use Flash strings (F()) for static strings
- Prefer char buffers over String where possible
- Be mindful of ESP8266 memory constraints

### Error Handling

- Use the centralized ErrorHandler where appropriate
- Provide meaningful error messages
- Handle edge cases gracefully
- Log errors appropriately

## Testing

Before submitting a PR:

- [ ] Code compiles without errors
- [ ] Tested on actual hardware (ESP8266)
- [ ] All features work as expected
- [ ] No memory leaks or crashes
- [ ] Logs are appropriate (not too verbose)
- [ ] Documentation updated if needed

## Documentation

- Update README.md if adding new features
- Add comments to new functions
- Update CHANGELOG.md for significant changes
- Keep code examples up to date

## Commit Messages

Use clear, descriptive commit messages:

```
Good: "Add support for BME280 sensor"
Good: "Fix WiFi reconnection timeout issue"
Bad: "fix"
Bad: "updates"
```

## Review Process

1. All PRs will be reviewed
2. Feedback will be provided within a reasonable time
3. Changes may be requested before merging
4. Be open to suggestions and improvements

## Questions?

If you have questions about contributing, feel free to:
- Open an issue with the "question" label
- Check existing issues and discussions
- Review the code and documentation

Thank you for contributing to HomeWatchWifi! 🎉





