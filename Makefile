BULID_DIR = build
CMAKE_OPT ?= --parallel

# Need to echo command AND run it in order to have make print the command but not the if statement
CMAKE_COMMAND = cmake src -B $(BULID_DIR) -DCMAKE_BUILD_TYPE=Release

all:
	@if [ ! -d $(BULID_DIR) ]; then \
		echo $(CMAKE_COMMAND); \
		$(CMAKE_COMMAND); \
	fi

	cmake --build $(BULID_DIR) $(CMAKE_OPT)
	cmake --install $(BULID_DIR)

run: 
	sudo /home/apoindexter/git/mongoose_memory_leak/dist/bin/mongoose -d /home/apoindexter/git/mongoose_memory_leak/dist/bin/httpd

debugbuild: ## Add as prefix to an above command to show verbose, in-order compilation
	$(eval CMAKE_OPT = --verbose)
	@CMAKE_OPT=$(CMAKE_OPT)

clean: ## Delete all files created by build process
	rm -rf build
	rm -rf dist/*