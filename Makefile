.PHONY: all install build serve clean help

VENV := .venv
PIP := $(VENV)/bin/pip

all: build

$(VENV)/bin/python:
	python3 -m venv $(VENV)
	$(PIP) install --upgrade pip
	$(PIP) install -r requirements.txt

install: $(VENV)/bin/python

build: clean install
	$(VENV)/bin/mkdocs build --strict

serve: install
	$(VENV)/bin/mkdocs serve

clean:
	rm -rf site/

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Available targets:"
	@echo "  all      Build the MkDocs site (default target)"
	@echo "  install  Create a virtual environment and install dependencies"
	@echo "  build    Build the MkDocs site"
	@echo "  serve    Serve the site locally with live reload"
	@echo "  clean    Remove the build output (site/)"
	@echo "  help     Show this help message"
