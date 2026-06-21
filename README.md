# GLYPH

**G**enerate **L**ettering **Y**ielding **P**ixel-art & **H**ues

A pure C, zero-dependency ASCII art generator for the terminal — text rendering with custom fonts, true-color styles, and image-to-ASCII conversion.

## Features

- 🔤 Custom font format (`.nfont`) with full FIGlet (`.flf`) support
- 🎨 24-bit true color styles: `plain`, `gradient`, `rainbow`, `gay`, `border`
- 🖼️ Image-to-ASCII conversion (PNG/JPG) via [stb_image](https://github.com/nothings/stb)
- 🌐 UTF-8 safe rendering
- 📦 Zero external dependencies (pure C + libc + bundled stb_image)
- ⚡ Auto-resolves fonts from local `fonts/`, `$PREFIX/share/glyph/fonts/`, and system FIGlet fonts

## Build

```bash
make
make install   # installs to $PREFIX/bin as `glyph` (alias: `gl`)
```

## Usage

```bash
glyph text -f <font> "your text" [options]
glyph img <image.png|jpg> [-w width] [-h height] [--no-color]
```

### Font

Fonts can be:
- A direct path (`./fonts/3d.flf`)
- A name only — auto-resolved from `./fonts/`, `$PREFIX/share/glyph/fonts/`, or `$PREFIX/share/figlet/`

### Options

| Flag | Description |
|------|-------------|
| `-f <font>` | Font name or path |
| `-F`, `--style` | `plain`, `gradient`, `rainbow`, `gay`, `border` |
| `--color` | `red`, `green`, `blue`, `yellow`, `cyan`, `magenta`, `white` |
| `-h`, `--help` | Show help |
| `-v`, `--version` | Show version |

### Examples

```bash
glyph text -f 3d -F rainbow "hello world"
glyph text -f standard -F gay "claude"
glyph img photo.jpg -w 80
```

## Project structure

```
glyph/
├── include/    # headers (font, parser, term, utf8, stb_image)
├── src/        # implementation
├── fonts/      # bundled custom + sample fonts
└── Makefile
```

## License

License under the [Apache License 2.0](LICENSE)

## Author

[Natarizki](https://github.com/Natarizki)
```

