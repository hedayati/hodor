SUBDIRS	= kern libhodor bench

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $(@)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $(@); \
	done

.PHONY: $(SUBDIRS) clean
