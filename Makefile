CC       := g++
CFLAGS   := -Wall -std=c++11 -O3
LIBS     := -ltbb -lsqlite3
INCLUDES := -Iinc
BIN      := airis

SRCDIR   := src
OBJDIR   := obj

SRCS=$(shell find $(SRCDIR) -type f -name *.cpp)
OBJS=$(patsubst $(SRCDIR)/%, $(OBJDIR)/%, $(SRCS:.cpp=.o))

# make sure dir exists
DIR_GUARD=@mkdir -p $(@D)


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(DIR_GUARD)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

all: main

main: $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(CFLAGS) $(LIBS)

clean:
	rm airis .depend
	rm -rf $(OBJDIR)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend
