BUILD_DIR = ./build
LOG_DIR = ./logs
RM = rm -rf

all: build
	cd $(BUILD_DIR); make

build:
	mkdir $(BUILD_DIR) $(LOG_DIR); cd $(BUILD_DIR); cmake -D CMAKE_CXX_COMPILER=/usr/bin/g++ ..

clean:
	-$(RM) $(BUILD_DIR)