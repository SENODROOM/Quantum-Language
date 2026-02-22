# GitHub Language Support for Quantum

This document explains how to add Quantum language support to GitHub's language detection system.

## Current Status

✅ **Repository-level configuration completed**
- `.gitattributes` file created to associate `.sa` files with Quantum language
- `.github/linguist.yml` configuration file created with dark blue color (#0033CC)

## What's Been Done

### 1. Repository Configuration
- **`.gitattributes`**: Tells GitHub to treat `.sa` files as Quantum language
- **`.github/linguist.yml`**: Defines Quantum language properties including the dark blue color

### 2. Language Definition
```yaml
Quantum:
  type: programming
  color: "#0033CC"  # Dark blue as requested
  extensions:
    - ".sa"
  tm_scope: source.quantum
  ace_mode: text
  language_id: 999999
```

## Next Steps for Full GitHub Support

To get Quantum language officially recognized by GitHub across all repositories, you need to contribute to the [github-linguist/linguist](https://github.com/github-linguist/linguist) repository.

### Requirements for Official Support

1. **Real-world samples** (2-3 files)
   - Must be substantial, non-trivial Quantum code
   - Should be from real projects with MIT/BSD/Apache licenses
   - Place in `samples/quantum/` directory in linguist repo

2. **Syntax highlighter** (.tmLanguage.json)
   - TextMate grammar file for syntax highlighting
   - Can be adapted from VSCode extension if available
   - Must have MIT-compatible license

3. **Prove usage**
   - Search GitHub for existing `.sa` files: `PATH:*.sa -IS:FORK`
   - If extension conflicts, provide regex heuristic

4. **Language ID**
   - Get assigned unique ID from linguist maintainers

### Contribution Process

1. Fork github-linguist/linguist
2. Add Quantum to `lib/linguist/languages.yml`
3. Add samples to `samples/quantum/`
4. Add syntax grammar if available
5. Run tests: `bundle exec rake`
6. Submit Pull Request

## Testing Repository Configuration

To test if the configuration works:

1. Push these changes to your GitHub repository
2. Wait for GitHub to reprocess (can take few minutes)
3. Check repository language statistics

## Alternative: Override Language Name

If you want to customize how the language appears in your repository specifically:

```gitattributes
*.sa linguist-language=Quantum
*.sa linguist-detectable
```

## Color Reference

The dark blue color `#0033CC` is now configured for:
- Repository-level display (immediate)
- Official GitHub support (when accepted into linguist)

## Resources

- [GitHub Linguist Repository](https://github.com/github-linguist/linguist)
- [Contributing Guidelines](https://github.com/github-linguist/linguist/blob/master/CONTRIBUTING.md)
- [Language Addition Tutorial](https://blog.jakelee.co.uk/adding-github-language-with-linguist/)
