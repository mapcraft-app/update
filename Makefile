# sudo apt install -y mingw-w64
CXX				:=	clang++-18
CXXFLAGS	:=	-Wall -Werror -Wextra -pedantic -std=c++17 \
							-DTARGET=$(TARGET) -O2 -static

RM				:=	rm -f
SRCS			:=	srcs/main.cpp
LIBRARY		?=	-I ./srcs -I ./srcs/7zip
OBJS			:=	$(SRCS:.cpp=.o)

#CXXFLAGS	?=	-Wall -Werror -Wextra -pedantic -std=c++17 -DTARGET=$(TARGET) -O3 -static -static-libgcc -static-libstdc++ $(LIBRARY)

ifeq ($(TARGET),windows)
CXX 			:=	x86_64-w64-mingw32-g++
NAME			:= 	update-windows.exe
CXXFLAGS	+=	-static-libstdc++

else ifeq ($(TARGET),macos)
NAME			:= 	update-darwin
CXXFLAGS	+=	-I /usr/include/c++/10 \
							-I /usr/include/x86_64-linux-gnu/c++/10 \
							-I /usr/include/x86_64-linux-gnu \
							-target aarch64-apple-darwin --verbose
#CXXFLAGS	+=	-I /usr/lib/gcc/x86_64-w64-mingw32/10-posix/include/c++ \
							-I /usr/lib/gcc/x86_64-w64-mingw32/10-posix/include/c++/x86_64-w64-mingw32 \
							-I /usr/x86_64-w64-mingw32/include/ \
							-stdlib=libc++ -target aarch64-apple-darwin --verbose
else
#	CXX 		:=	g++
NAME			:= 	update-linux
CXXFLAGS	+=	-stdlib=libstdc++ -target x86_64-pc-linux-gnu
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
