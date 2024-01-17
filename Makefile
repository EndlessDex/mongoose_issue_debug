.PHONY: mongoose all run debugbuild clean help
.DEFAULT_GOAL = help

BULID_DIR = build
CMAKE_OPT ?= --parallel

# Need to echo command AND run it in order to have make print the command but not the if statement
CMAKE_COMMAND = cmake src -B $(BULID_DIR) -DCMAKE_BUILD_TYPE=Release

all: clean mongoose run

mongoose:
	@if [ ! -d $(BULID_DIR) ]; then \
		echo $(CMAKE_COMMAND); \
		$(CMAKE_COMMAND); \
	fi

	cmake --build $(BULID_DIR) $(CMAKE_OPT)
	cmake --install $(BULID_DIR)

run: ## Run the built server with the build httpd directory
	sudo $(PWD)/dist/bin/mongoose -d $(PWD)/dist/bin/httpd

debugbuild: ## Add as prefix to an above command to show verbose, in-order compilation
	$(eval CMAKE_OPT = --verbose)
	@CMAKE_OPT=$(CMAKE_OPT)

clean: ## Delete all files created by build process
	rm -rf build
	rm -rf dist/*

help:
	@echo 'Error: Please select a valid target from list below. Example: "make 4100P"'
	@grep -E '^[a-z0-9A-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\t\033[36m%-10s\033[0m %s\n", $$1, $$2}'
