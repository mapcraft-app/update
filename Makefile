# sudo apt install -y mingw-w64

ifeq ($(TARGET),windows)
	CXX 			:=	i686-w64-mingw32-g++
	NAME			:= 	mapcraft-update.exe
else ifeq ($(TARGET),macos)
	CXX 			:=	g++
	NAME			:= 	mapcraft-update.mac
else
	CXX 			:=	g++
	NAME			:= 	mapcraft-update
endif

RM					:=	rm -f
SRCS				:=	srcs/main.cpp
LIBRARY			?=	-I ./srcs -I ./srcs/7zip
OBJS				:=	$(SRCS:.cpp=.o)
CXXFLAGS		?=	-Wall -Werror -Wextra -pedantic -std=c++17 -DTARGET=$(TARGET) -O3 -static -static-libgcc -static-libstdc++ $(LIBRARY)

# Colors
RED			:= \e[0;91m
GREEN		:= \e[0;92m
BLUE		:= \e[0;94m
MAGENTA	:= \e[0;95m
RESET		:= \e[0;0m
PREFIX	:= $(MAGENTA)$(NAME)$(RESET) => 

$(NAME): $(OBJS)
	@echo "$(PREFIX)$(GREEN)Bundling $(RESET)$(NAME)$(GREEN) executable$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJS) $(LIBRARY) -o $@

%.o: %.cpp
	@echo "$(PREFIX)$(GREEN)Compiling file $(RESET)$< $(BLUE)to $(RESET)$@"
	@$(CXX) $(CXXFLAGS) $(LIBRARY) -c $< -o $@

all: $(NAME)

fclean:
	$(RM) $(OBJS)
	$(RM) $(NAME)

clean:
	$(RM) $(OBJS)

re: fclean
	$(MAKE) all
