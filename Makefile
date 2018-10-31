# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -lz

# Default target
all: sender receiver

sender: sender.o packet_implem.o create_socket.o real_address.o node.o time.o $(LDFLAGS)

receiver: receiver.o packet_implem.o real_address.o create_socket.o time.o node.o wait_for_client.o $(LDFLAGS)

clean:
	@rm -f sender sender.o read_write_loop.o create_socket.o real_address.o wait_for_client.o node.o packet_implem.o time.o receiver receiver.o

