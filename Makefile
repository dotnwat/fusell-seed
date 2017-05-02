CXXFLAGS += -Wall -Werror -std=c++11
CPPFLAGS += $(shell pkg-config fuse --cflags)
LIBS += $(shell pkg-config fuse --libs)
LIBS += -lm

ifneq ($(shell uname -s),Darwin)
  LIBS += -lrt
endif

all: gassy

CPPFLAGS += -DFUSE_USE_VERSION=30

OBJS = gassy.o inode.o gassy_fs.o inode_index.o \
	   local_address_space.o

dep_files := $(foreach f, $(OBJS), $(dir f).depend/$(notdir $f).d)
dep_dirs := $(addsuffix .depend, $(sort $(dir $(OBJS))))

$(dep_dirs):
	@mkdir -p $@

missing_dep_dirs := $(filter-out $(wildcard $(dep_dirs)), $(dep_dirs))
dep_file = $(dir $@).depend/$(notdir $@).d
dep_args = -MF $(dep_file) -MQ $@ -MMD -MP

%.o: %.cc $(missing_dep_dirs)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $*.o -c $(dep_args) $<

gassy: $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

dep_files_present := $(wildcard $(dep_files))
ifneq ($(dep_files_present),)
include $(dep_files_present)
endif

clean:
	rm -rf $(dep_dirs) $(OBJS) gassy
