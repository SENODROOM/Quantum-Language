CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Iinclude
TARGET   = quantum
SRCS     = src/main.cpp src/Token.cpp src/Lexer.cpp src/Parser.cpp src/Value.cpp src/Interpreter.cpp
OBJS     = $(SRCS:.cpp=.o)

.PHONY: all clean install uninstall test

all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "  Building Quantum Language..."
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread
	@echo "  ✓ Built: ./quantum"

install: $(TARGET)
	@echo "  Installing to /usr/local/bin/quantum..."
	install -m 755 $(TARGET) /usr/local/bin/$(TARGET)
	@echo "  ✓ Installed! Run: quantum <file.sa>"

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "  ✓ Uninstalled"

test: $(TARGET)
	@echo "  Running tests..."
	./$(TARGET) examples/hello.sa
	./$(TARGET) examples/features.sa
	./$(TARGET) examples/cybersec.sa
	./$(TARGET) examples/advanced.sa
	@echo "  ✓ All tests passed!"

clean:
	rm -f $(TARGET) src/*.o
	@echo "  ✓ Cleaned"

debug: CXXFLAGS = -std=c++17 -g -Wall -Wextra -Iinclude
debug: $(TARGET)
