#############################
# RedHate (ultros)
#############################

TARGET      =  achat
OBJ         =  alsa-chat.o

LDLIBS		=  -lasound -lpthread  

CC          =  gcc
CXX         =  g++
LD          =  gcc
MV          =  mv
CP          =  cp
ECHO        =  echo
RM          =  rm
AR          =  ar
RANLIB      =  ranlib
STRIP       =  strip

INCLUDES	?= -I/usr/include
LIBS		?= -L/usr/lib
LDFLAGS		?= -B/usr/lib

CFLAGS   	?= -std=c99 -Wall -pedantic  -g -O2 $(INCLUDES) $(LIBS)
CXXFLAGS 	?= -std=c99 -Wall -pedantic  -g -O2 $(INCLUDES) $(LIBS)
WARNINGS	:=

## colors are fun
BLACK1		= \033[1;30m
BLACK2		= \033[0;30m
RED1		= \033[1;31m
RED2		= \033[0;31m
GREEN1		= \033[1;32m
GREEN2		= \033[0;32m
YELLOW1		= \033[1;33m
YELLOW		= \033[0;33m
BLUE1		= \033[1;34m
BLUE2		= \033[0;34m
PURPLE1		= \033[1;35m
PURPLE2		= \033[0;35m
CYAN1		= \033[1;36m
CYAN2		= \033[0;36m
WHITE1		= \033[1;37m
WHITE2		= \033[0;37m
NOCOLOR		= \033[0m

.PHONY: all run clean

all:  $(OBJ) $(TARGET)

run:   $(OBJ) $(TARGET)
	@./$(TARGET)

clean: 
	@printf "$(RED1)[CLEANING]$(NOCOLOR)\n" 
	@rm $(OBJ) $(TARGET)

%.o: %.cpp
	@printf "$(RED1)[CXX]$(NOCOLOR) $(notdir $(basename $<)).o\n" 
	@$(CXX) $(WARNINGS) -c $< $(CXXFLAGS) -o $(basename $<).o 

%.o: %.cxx
	@printf "$(RED1)[CXX]$(NOCOLOR) $(notdir $(basename $<)).o\n" 
	@$(CXX) $(WARNINGS) -c $< $(CFLAGS) -o $(basename $<).o 

%.o: %.c
	@printf "$(RED1)[CC]$(NOCOLOR) $(notdir $(basename $<)).o\n" 
	@$(CC) $(WARNINGS) -c $< $(CFLAGS) -o $(basename $<).o 

%.a:
	@printf "$(RED1)[CC]$(NOCOLOR) $(basename $(TARGET_LIB)).a\n" 
	@$(AR) -cru $(basename $(TARGET_LIB)).a $(OBJ)

$(TARGET):  $(OBJ)
	@printf "$(RED1)[CC]$(NOCOLOR) $(TARGET)\n"
	@$(CC) $(OBJ) $(LDLIBS) $(LDFLAGS) $(CXXFLAGS) -o $(TARGET)
	@$(STRIP) $(TARGET)


