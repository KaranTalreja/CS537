PROJ_WDIR=../../Project1a/
CURR_DIR=$(PWD)
INCDIRS=-I $(CURR_DIR)/include/
SRC=$(CURR_DIR)/src
BIN=$(CURR_DIR)/bin
EXEC_GEN=$(BIN)/genvar
EXEC_DUMP=$(BIN)/dumpvar
OUTDIR=$(CURR_DIR)/out
SOURCES_GEN=$(SRC)/genvar.c
SOURCES_DUMP=$(SRC)/dumpvar.c
OBJDIR=$(CURR_DIR)/obj
NUM_RECORDS=100
MAX_DATA_KEY=32

AUTODEPS:=$(patsubst $(SRC)/%.c,$(OBJDIR)/%.d,$(SOURCES_GEN))
AUTODEPS:=$(AUTODEPS)+$(patsubst $(SRC)/%.c,$(OBJDIR)/%.d,$(SOURCES_DUMP))
OBJS_GEN=$(patsubst $(SRC)/%.c,$(OBJDIR)/%.o,$(SOURCES_GEN))
OBJS_DUMP=$(patsubst $(SRC)/%.c,$(OBJDIR)/%.o,$(SOURCES_DUMP))
GCC=gcc
FLAGS=-g -Wall -O2

$(shell mkdir -p $(BIN))
$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(OUTDIR))

all:$(EXEC_GEN) $(EXEC_DUMP)

-include $(AUTODEPS)

$(EXEC_DUMP): $(OBJS_DUMP)
	@echo "$(notdir $^) --> $(notdir $@)"
	@$(GCC) $^ -o $@ $(FLAGS) $(INCDIRS) 

$(EXEC_GEN): $(OBJS_GEN)
	@echo "$(notdir $^) --> $(notdir $@)"
	@$(GCC) $^ -o $@ $(FLAGS) $(INCDIRS) 

$(OBJDIR)/%.o: $(SRC)/%.c
	@echo "$(notdir $^) --> $(notdir $@)"
	@$(GCC) -o $@ $(FLAGS) $(INCDIRS) -c $< 

$(OBJDIR)/%.d: $(SRC)/%.c
	@echo "$(notdir $^) --> $(notdir $@)"
	@set -e; rm -f $@; \
         $(GCC) -MM $(INCDIRS) $(FLAGS) $< > $@.$$$$; \
         sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
         rm -f $@.$$$$

test:
	@echo "Generating file out/outfile.$(NUM_RECORDS).$(MAX_DATA_KEY)"
	@$(EXEC_GEN) -s 0 -n $(NUM_RECORDS) -m $(MAX_DATA_KEY) -o $(OUTDIR)/outfile.$(NUM_RECORDS).$(MAX_DATA_KEY)

clean:
	@rm -rf $(EXEC_GEN) $(EXEC_DUMP) $(OBJDIR)/*.d* $(OBJDIR)/*.d $(OBJDIR)/*.o 

.PHONY:test
