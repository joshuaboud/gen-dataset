TARGET = gen-dataset
LIBS = -lboost_random
CC = g++
CFLAGS = -std=c++11 -g -O2 -Wall -Isrc/incl

SOURCE_FILES := $(shell find src/impl -name *.cpp)
HEADER_FILES := $(shell find src/incl -name *.hpp)
OBJECT_FILES := $(patsubst src/impl/%.cpp, build/%.o, $(SOURCE_FILES))

ifeq ($(PREFIX),)
	PREFIX := /usr/local/bin
endif

.PHONY: default all clean install uninstall

default: $(TARGET)
all: default

$(TARGET): $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) -Wall $(LIBS) -o $@

$(OBJECT_FILES): build/%.o : src/impl/%.cpp $(HEADER_FILES)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(patsubst build/%.o, src/impl/%.cpp, $@) -o $@

clean:
	rm -rf build
	rm $(TARGET)

install: all
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)

uninstall:
	-rm -f $(DESTDIR)$(PREFIX)$(TARGET)
