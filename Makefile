CC = clang
ARCH := $(shell uname -m)

CFLAGS = -Wall -O2 -Iinclude
LDFLAGS = -lm

SRC = src/font.c src/flf_parser.c src/nfont_parser.c src/term.c src/image.c src/utf8.c src/color.c src/anim.c src/main.c
OBJ = $(SRC:.c=.o)
BIN = glyph-$(ARCH)

PREFIX ?= $(shell echo $$PREFIX)
INSTALL_BIN = $(PREFIX)/bin/glyph
INSTALL_FONTS = $(PREFIX)/share/glyph/fonts

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)
	@ln -sf $(BIN) glyph
	@echo "Built for architecture: $(ARCH)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	@mkdir -p $(INSTALL_FONTS)
	cp glyph $(INSTALL_BIN)
	chmod +x $(INSTALL_BIN)
	ln -sf $(INSTALL_BIN) $(PREFIX)/bin/gl
	cp -f fonts/*.nfont $(INSTALL_FONTS)/ 2>/dev/null || true
	cp -f fonts/*.flf $(INSTALL_FONTS)/ 2>/dev/null || true
	@echo "Installed glyph to $(INSTALL_BIN)"
	@echo "Alias 'gl' tersedia juga"
	@echo "Installed fonts to $(INSTALL_FONTS)"

uninstall:
	rm -f $(INSTALL_BIN)
	rm -f $(PREFIX)/bin/gl
	rm -rf $(INSTALL_FONTS)

clean:
	rm -f $(OBJ) glyph-* glyph
