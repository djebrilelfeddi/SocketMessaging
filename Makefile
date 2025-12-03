# --- CONFIGURATION ---
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -Iinclude
LDFLAGS  := -pthread

# Dossiers
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Noms des exécutables
SERVER_TARGET := $(BIN_DIR)/server
CLIENT_TARGET := $(BIN_DIR)/client

# --- GESTION AUTOMATIQUE DES FICHIERS ---

# 1. On trouve tous les fichiers sources sauf les mains
# On suppose la structure : src/Server, src/Client, src/Utils
SRCS_SERVER := $(shell find $(SRC_DIR)/Server -name '*.cpp' 2>/dev/null)
SRCS_CLIENT := $(shell find $(SRC_DIR)/Client -name '*.cpp' 2>/dev/null)
SRCS_UTILS  := $(shell find $(SRC_DIR)/Utils -name '*.cpp' 2>/dev/null)

# 2. Les fichiers Main (supposés être à la racine de src/ ou dans leurs dossiers respectifs)
# Ajustez les chemins ici si vos mains sont ailleurs
MAIN_SERVER_SRC := $(SRC_DIR)/main_server.cpp
MAIN_CLIENT_SRC := $(SRC_DIR)/main_client.cpp

# 3. Conversion des .cpp en .o
# Exemple : src/Server/machin.cpp -> obj/Server/machin.o
OBJS_SERVER := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_SERVER))
OBJS_CLIENT := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_CLIENT))
OBJS_UTILS  := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_UTILS))

OBJ_MAIN_SERVER := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(MAIN_SERVER_SRC))
OBJ_MAIN_CLIENT := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(MAIN_CLIENT_SRC))

# Dépendances (.d) pour recompiler si un header change
ALL_OBJS := $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_UTILS) $(OBJ_MAIN_SERVER) $(OBJ_MAIN_CLIENT)
DEPS := $(ALL_OBJS:.o=.d)

# --- RÈGLES ---

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Édition des liens SERVEUR (Main + Code Server + Utils)
$(SERVER_TARGET): $(OBJ_MAIN_SERVER) $(OBJS_SERVER) $(OBJS_UTILS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@

# Édition des liens CLIENT (Main + Code Client + Utils)
$(CLIENT_TARGET): $(OBJ_MAIN_CLIENT) $(OBJS_CLIENT) $(OBJS_UTILS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@

# Règle générique de compilation (.cpp -> .o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)