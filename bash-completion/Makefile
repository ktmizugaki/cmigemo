ifeq ($(OS),Windows_NT)
  BIN_EXT = .exe
else
  BIN_EXT = 
endif

SRCDIR  = ../src
OUTDIR  = build
SRCS    = $(shell cat SOURCES)
OBJS    = $(patsubst $(SRCDIR)/%.c,$(OUTDIR)/%.o,$(SRCS))
CFLAGS  = -O2 -Wall
LDFLAGS = 
BIN     = cmigemo$(BIN_EXT)
DICT    = dict/migemo-dict

CHARSET     = utf-8
INSTALL_DIR = ~/.migemo

.PHONY: all install bin dict clean distclean dep

all: bin dict

install: all
	mkdir -p $(INSTALL_DIR)
	cp -a $(BIN)  $(INSTALL_DIR)/
	sed -e "s#@MIGEMO_DIR@#$(INSTALL_DIR)#" bash-completion-cmigemo.sh > $(INSTALL_DIR)/bash-completion-cmigemo.sh
	mkdir -p $(INSTALL_DIR)/dict
	cp -a dict/* $(INSTALL_DIR)/dict/
	@echo "#############################################"
	@echo "## Installation Completed"
	@echo "## To enable filename completion with migemo,"
	@echo "## add the following line to your .bashrc"
	@echo "##"
	@echo "## source $(INSTALL_DIR)/bash-completion-cmigemo.sh"
	@echo "#############################################"


bin: $(BIN)

dict: $(DICT)

$(OUTDIR)/make.dep dep: SOURCES
	@rm -f $(OUTDIR)/make.dep
	@mkdir -p $(OUTDIR)
	@echo Generating dependencies
	@for x in $(SRCS); do \
		$(CC) -MM $(CFLAGS) $$(if [ $${x#*.} = rc ]; then echo -n -x c $$x -MT $(OUTDIR)/$${x%.*}.coff; else echo -n $$x -MT $(OUTDIR)/$${x%.*}.o; fi) >> $(OUTDIR)/make.dep; done

$(OUTDIR)/%.o: $(SRCDIR)/%.c $(OUTDIR)/make.dep
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJS)

$(DICT):
	mkdir -p dict
	make -C ../dict $(CHARSET)
	cp -a ../dict/$(CHARSET).d/* dict/

clean:
	rm -f $(OBJS)
	rm -f $(OUTDIR)/make.dep

distclean: clean
	rm -f $(BIN)
	rm -rf dict

ifneq ($(MAKECMDGOALS),clean)
    ifneq ($(MAKECMDGOALS),distclean)
        -include $(OUTDIR)/make.dep
    endif
endif
