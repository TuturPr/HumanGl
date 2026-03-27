NAME = humangl

INCLUDE = includes
LIBS = -Iinclude -ldl -lglfw -pthread -lm

CC = cc
CXX = c++

CFLAGS = -Wall -Wextra -Werror -I $(INCLUDE) -I ./src -I ./includes -I ./includes/glm -I ./includes/imgui-1.92.6 -I ./includes/imgui-1.92.6/backends -O3
CXXFLAGS = -Wall -Wextra -Werror -I $(INCLUDE) -I ./src -I ./includes -I ./includes/glm -I ./includes/imgui-1.92.6 -I ./includes/imgui-1.92.6/backends -O3 -std=c++17

GREEN = \033[1;38;2;0;255;0m
RED = \033[1;38;2;255;0;0m
DEF_COLOR = \033[0m

# Sources

include Sources.mk

SRC_DIR = ./src
GLAD_SRC = libs/glad.c

SRC_CPP = $(SRC_MAIN)
SRC_C = $(GLAD_SRC)
SRC = $(SRC_CPP) $(SRC_C)

TMP = .tmp
OBJS = $(patsubst %.cpp, $(TMP)/%.o, $(SRC_CPP)) $(patsubst %.c, $(TMP)/%.o, $(SRC_C))

# Fonctions

all: $(NAME)
	@printf "%b\n" "$(GREEN)Compilation successful !$(DEF_COLOR)"

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(NAME) || \
		(printf "%b\n" "$(RED)Error during linking!$(DEF_COLOR)" && exit 1)

# Generic compilation rules
$(TMP)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ || (printf "%b\n" "$(RED)Error compiling $<!$(DEF_COLOR)" && exit 1)

$(TMP)/./%.o: ./%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ || (printf "%b\n" "$(RED)Error compiling $<!$(DEF_COLOR)" && exit 1)

$(TMP)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@ || (printf "%b\n" "$(RED)Error compiling $<!$(DEF_COLOR)" && exit 1)

clean:
	@rm -rf $(TMP)
	@printf "%b\n" "$(GREEN)Clean successful !$(DEF_COLOR)"

fclean: clean
	@rm -f $(NAME)
	@rm -rf $(MLX)/build
	@printf "%b\n" "$(GREEN)Full clean successful !$(DEF_COLOR)"

re: fclean all

.PHONY: all clean fclean re
