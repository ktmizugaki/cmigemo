SRCDIR  = ../src
OUTDIR  = build
SRCS    = $(shell cat SOURCES)
OBJS    = $(patsubst $(SRCDIR)/%.c,$(OUTDIR)/%.o,$(SRCS))
CFLAGS  = -O2 -Wall
LDFLAGS = 
BIN     = cmigemo
DICT    = 
ifeq ($(OS),Windows_NT)
  BIN  +=.exe
endif

CHARSET     = utf-8
INSTALL_DIR = ~/.migemo

.PHONY: all install bin dict clean distclean dep

all: bin dict

install: all
	mkdir -p $(INSTALL_DIR)
	@echo Installing
	cp -a $(BIN)  $(INSTALL_DIR)/
	sed -e "s#@MIGEMO_DIR@#$(INSTALL_DIR)#" bash-completion-cmigemo.sh > $(INSTALL_DIR)/bash-completion-cmigemo.sh
	@echo Installing dictionary
	mkdir -p $(INSTALL_DIR)/dict
	cp -a dict/* $(INSTALL_DIR)/dict/
	@echo Add "source $(INSTALL_DIR)/bash-completion-cmigemo.sh" to your .bashrc

bin: $(BIN)

dict: $(
	mkdir -p dict
	make -C ../dict $(CHARSET)
	cp -a ../dict/$(CHARSET).d/* dict/

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
