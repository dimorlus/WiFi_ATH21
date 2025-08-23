# Makefile для Linux
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -Isrc
LIBS = -lpaho-mqttpp3 -lpaho-mqtt3as -lstdc++fs -lpthread

# Директории
SRCDIR = src
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
TARGET = $(BUILDDIR)/mqtt_logger

# Основная цель
all: $(TARGET)

# Создание исполняемого файла
$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(OBJECTS) -o $@ $(LIBS)

# Компиляция объектных файлов
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Создание директории сборки
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Установка зависимостей на Ubuntu/Debian
install-deps:
	sudo apt update
	sudo apt install -y build-essential cmake git
	sudo apt install -y libpaho-mqtt-dev libpaho-mqttpp-dev

# Установка зависимостей на CentOS/RHEL/Fedora
install-deps-rpm:
	sudo dnf install -y gcc-c++ cmake git
	sudo dnf install -y paho-c-devel paho-cpp-devel

# Очистка
clean:
	rm -rf $(BUILDDIR)

# Установка
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	mkdir -p ~/.config/mqtt_logger
	cp config.ini ~/.config/mqtt_logger/ 2>/dev/null || true

# Удаление
uninstall:
	sudo rm -f /usr/local/bin/mqtt_logger

# Запуск
run: $(TARGET)
	$(TARGET)

# Отладка
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all clean install uninstall run debug install-deps install-deps-rpm