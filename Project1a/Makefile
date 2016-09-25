CURR_DIR=$(PWD)
INCDIRS=-I gen_case/include/
SRC=$(CURR_DIR)/src
BIN=$(CURR_DIR)/bin
EXEC=$(BIN)/varsort
SOURCES=$(SRC)/varsort.c
OBJDIR=$(CURR_DIR)/obj

AUTODEPS:= $(patsubst $(SRC)/%.c,$(OBJDIR)/%.d,$(SOURCES))
OBJS=$(patsubst $(SRC)/%.c,$(OBJDIR)/%.o,$(SOURCES))
GCC=gcc
FLAGS=-g -Wall -gdwarf-2 

$(shell mkdir -p $(BIN))
$(shell mkdir -p $(OBJDIR))

all:$(EXEC)

-include $(AUTODEPS)

$(EXEC): $(OBJS)
	@echo "$(notdir $^) --> $(notdir $@)"
	@$(GCC) $^ -o $@ $(FLAGS) $(INCDIRS)
	@$(MAKE) lint

$(OBJDIR)/%.o: $(SRC)/%.c
	@echo "$(notdir $^) --> $(notdir $@)"
	@$(GCC) -o $@ $(FLAGS) $(INCDIRS) -c $< 

$(OBJDIR)/%.d: $(SRC)/%.c
	@echo "$(notdir $^) --> $(notdir $@)"
	@set -e; rm -f $@; \
         $(GCC) -MM $(INCDIRS) $(FLAGS) $< > $@.$$$$; \
         sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
         rm -f $@.$$$$

lint:
	@echo "Testing linter"
	@cp $(CURR_DIR)/../lint/CPPLINT.cfg $(SRC)/
	@$(CURR_DIR)/../lint/cpplint.py --extensions c,h $(SOURCES)
	@rm $(SRC)/CPPLINT.cfg

clean:
	@rm -rf $(EXEC) $(OBJDIR)/*.d* $(OBJDIR)/*.d $(OBJDIR)/*.o 

.PHONY:test
