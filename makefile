CC=gcc
DEPS = headers.h
OBJ = built_ins.o basic_setup.o shell.o run_command.o pinfo.o parse.o io_redirection.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
		gcc -o $@ $^ $(CFLAGS)
