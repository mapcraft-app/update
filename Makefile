# sudo apt install -y mingw-w64
CXXFLAGS	:=	-Wall -Werror -Wextra -pedantic -std=c++17 -DTARGET=$(TARGET) -O2
RM				:=	rm -f
SRCS			:=	srcs/main.cpp
LIBRARY		?=	-I ./srcs -I ./srcs/7zip
OBJS			:=	$(SRCS:.cpp=.o)

ifeq ($(TARGET),windows)
CXX 			:=	x86_64-w64-mingw32-g++
NAME			:= 	update-windows.exe
CXXFLAGS	+=	-static -static-libstdc++
else ifeq ($(TARGET),macos)
CXX				:=	clang++
NAME			:= 	update-darwin
CXXFLAGS	+=	-target x86_64-apple-darwin
else
CXX				:=	clang++-18
NAME			:= 	update-linux
CXXFLAGS	+=	-static -stdlib=libstdc++ -target x86_64-pc-linux-gnu
endif

CXXFLAGS	+= $(LIBRARY)

# Colors
RED				:=	\e[0;91m
GREEN			:=	\e[0;92m
BLUE			:=	\e[0;94m
MAGENTA		:=	\e[0;95m
RESET			:=	\e[0;0m
PREFIX		:=	$(MAGENTA)$(NAME)$(RESET) => 

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
