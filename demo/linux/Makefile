VERSION = 1.0.0 

######################################
# target
######################################
TARGET = lwcli_demo

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES += $(wildcard ./*.c) 
C_SOURCES += $(wildcard ../../lwcli/*.c) 

######################################
# include
######################################
# C includes
C_INCLUDES += -I./
C_INCLUDES += -I../../lwcli

######################################
# define
######################################
# C defines
C_DEFS = \
#-D

######################################
# library
######################################
# C librarys
LIBS = # -lxxx
LIB_PATH = # -L./xxx

######################################
# compile gcc flags
######################################
CFLAGS = -Wall -c 
CC = gcc  

#######################################
# build the application
#######################################
# list of objects
#OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) $(LIB_PATH) $(LIBS) -o $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(C_INCLUDES) $(C_DEFS) $(CFLAGS) $< -o $@  

$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
.PHONY: clean  
clean:  
	rm -rf $(OBJECTS) $(BUILD_DIR)/  